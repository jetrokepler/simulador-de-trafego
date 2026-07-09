#ifndef AMBULANCE_H
#define AMBULANCE_H

#include "map.h"

/* =======================================================================
 * INTEGRANTE 3 - Ambulância
 * Arquivo: ambulance.h
 * ===================================================================== */

/* Força o semáforo do cruzamento inter_id a abrir na direção `dir`
 * (usada pela ambulância antes de tentar entrar em um cruzamento).
 * Não bypassa a exclusão mútua de célula: apenas garante que o sinal
 * estará verde quando a ambulância chegar. */
void request_priority(int inter_id, Direction dir);

/* Devolve o cruzamento ao ciclo normal de alternância do semáforo. */
void release_priority(int inter_id);

#endif /* AMBULANCE_H */
