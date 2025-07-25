// device_finder.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include "../include/device_finder.h"

const char* find_keyboard_device() {
    static char resolved_path[PATH_MAX];
    const char *dirpath = "/dev/input/by-path";

    DIR *dir = opendir(dirpath);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            // Look for "kbd" in the symbolic link name
            if (strstr(entry->d_name, "kbd")) {
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", dirpath, entry->d_name);

                // Resolve the symbolic link to its actual device path
                if (realpath(full_path, resolved_path)) {
                    closedir(dir);
                    return resolved_path;
                }
            }
        }
        closedir(dir);
    }

    // Fallback: parse /proc/bus/input/devices
    FILE *fp = fopen("/proc/bus/input/devices", "r");
    if (!fp) return NULL;

    char line[512];
    char event_name[32]; // Temporary buffer for "eventXX" string
    // This loop looks for the Handlers= line that contains "kbd" (keyboard)
    // and also "event" (event handler) to find the correct device.
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "Handlers=") && strstr(line, "kbd")) {
            char *event_ptr = strstr(line, "event");
            if (event_ptr) {
                // Extract the "eventXX" part (e.g., "event0", "event2")
                sscanf(event_ptr, "%s", event_name);
                snprintf(resolved_path, sizeof(resolved_path), "/dev/input/%s", event_name);
                fclose(fp);
                return resolved_path;
            }
        }
    }

    fclose(fp);
    return NULL;
}

const char* find_mouse_device() {
    static char resolved_path[PATH_MAX];

    // NEW: Prioritize /dev/input/by-id for "event-mouse" or "mouse" linking to eventX
    const char *by_id_dirpath = "/dev/input/by-id";
    DIR *by_id_dir = opendir(by_id_dirpath);
    if (by_id_dir) {
        struct dirent *entry;
        while ((entry = readdir(by_id_dir)) != NULL) {
            // Look for "event-mouse" or "mouse" in the symbolic link name
            // and ensure it resolves to an "event" device, not "js" (joystick)
            if ((strstr(entry->d_name, "event-mouse") || strstr(entry->d_name, "mouse")) &&
                strstr(entry->d_name, "js") == NULL) { // Exclude joystick entries
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", by_id_dirpath, entry->d_name);

                // Resolve the symbolic link to its actual device path
                if (realpath(full_path, resolved_path)) {
                    // Double-check that it's an event device, not a joystick
                    if (strstr(resolved_path, "/event") != NULL) {
                        closedir(by_id_dir);
                        return resolved_path;
                    }
                }
            }
        }
        closedir(by_id_dir);
    }


    // Existing: Fallback to /dev/input/by-path
    const char *by_path_dirpath = "/dev/input/by-path";
    DIR *by_path_dir = opendir(by_path_dirpath);
    if (by_path_dir) {
        struct dirent *entry;
        while ((entry = readdir(by_path_dir)) != NULL) {
            // Look for "mouse" in the symbolic link name and exclude "js"
            if (strstr(entry->d_name, "mouse") && strstr(entry->d_name, "js") == NULL) {
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", by_path_dirpath, entry->d_name);

                // Resolve the symbolic link to its actual device path
                if (realpath(full_path, resolved_path)) {
                    // Double-check that it's an event device, not a joystick
                    if (strstr(resolved_path, "/event") != NULL) {
                        closedir(by_path_dir);
                        return resolved_path;
                    }
                }
            }
        }
        closedir(by_path_dir);
    }

    // Existing: Fallback to /proc/bus/input/devices
    FILE *fp = fopen("/proc/bus/input/devices", "r");
    if (!fp) return NULL;

    char line[512];
    char device_event_name[32];

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "Handlers=") != NULL) {
            // Look for "mouse" and "event" in the Handlers line, and exclude "js"
            if (strstr(line, "mouse") != NULL && strstr(line, "event") != NULL && strstr(line, "js") == NULL) {
                char *event_ptr = strstr(line, "event");
                if (event_ptr) {
                    sscanf(event_ptr, "%s", device_event_name);
                    snprintf(resolved_path, sizeof(resolved_path), "/dev/input/%s", device_event_name);
                    fclose(fp);
                    return resolved_path;
                }
            }
        }
    }

    fclose(fp);
    return NULL;
}

