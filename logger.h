#ifndef LOGGER_H
#define LOGGER_H


void logger_init(const char *filename);
void logger_close(void);
void log_event(const char *fmt, ...);

#endif
