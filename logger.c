#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include "logger.h"
#include "clock.h"

/* =======================================================================
 * INTEGRANTE 3 - Logger
 * Arquivo: logger.c
 * ===================================================================== */

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *log_file = NULL;

void logger_init(const char *filename) {
    log_file = fopen(filename, "w");
    if (log_file == NULL) {
        /* Se não conseguir abrir o arquivo, cai para stderr em vez de
         * derrubar a simulação. */
        log_file = stderr;
    }
}

void logger_close(void) {
    if (log_file != NULL && log_file != stderr) {
        fclose(log_file);
    }
    log_file = NULL;
}

void log_event(const char *fmt, ...) {
    if (log_file == NULL) return;

    pthread_mutex_lock(&log_mutex);
    fprintf(log_file, "[tick %ld] ", g_clock.tick);

    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);
    pthread_mutex_unlock(&log_mutex);
}
