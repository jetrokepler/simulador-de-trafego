#ifndef LOGGER_H
#define LOGGER_H

/* =======================================================================
 * INTEGRANTE 3 - Logger
 * Arquivo: logger.h
 *
 * Log simples e thread-safe (mutex) para simulation.log. Registra
 * prioridade de ambulância e início/fim de threads.
 * ===================================================================== */

void logger_init(const char *filename);
void logger_close(void);
void log_event(const char *fmt, ...);

#endif /* LOGGER_H */
