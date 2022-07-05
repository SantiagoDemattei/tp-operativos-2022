#ifndef TLB_MANAGER_H
#define TLB_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <signal.h>
#include "configuracion.h"
#include "../../shared/include/estructuras.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"

typedef enum ALGORITMO_REEMPLAZO {
    FIFO,
    LRU,
} ALGORITMO_REEMPLAZO; //enum para los distintos tipos de algoritmos de reemplazo de la tlb

t_tlb* elegir_victima_lru();
void borrar_entrada(t_tlb* victima);
void agregar(t_tlb* elemento_a_agregar);
uint32_t buscar(uint32_t numero_pagina);
ALGORITMO_REEMPLAZO algoritmo_remplazo (char* algoritmo);
#endif