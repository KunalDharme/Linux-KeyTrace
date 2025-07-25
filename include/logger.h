// logger.h
#pragma once
int init_logger(const char *plain_path, const char *timestamped_path);
void log_key(const char *type, const char *data);
void close_logger();

