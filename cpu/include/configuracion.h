#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/estructuras.h"
#include "../../shared/include/utils.h"
#include "math.h"

t_pcb* running; //global de la cpu
uint32_t tamanio_pagina;
t_list* tlb;

pthread_mutex_t mutex_logger_cpu;
pthread_mutex_t mutex_running_cpu;
pthread_mutex_t mutex_interrupcion;
pthread_mutex_t mutex_tlb;

bool interrupciones;

t_configuracion_cpu* leer_configuracion(t_log* logger);
void liberar_estructura_datos(t_configuracion_cpu* datos);
char* eliminar_caracter_retorno(char* cadena);
void inicializar_semaforos();

typedef enum
{
    NO_OP,
    I_O,
    READ,
    WRITE,
    COPY,
    EXIT,
    ERROR
} INSTRUCCIONES_EJECUCION; //enum para los distintos tipos de instrucciones 




#endif
