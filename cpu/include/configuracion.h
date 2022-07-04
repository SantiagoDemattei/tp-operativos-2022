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
uint32_t cant_entradas_por_tabla;
t_list* tlb; 

t_log* logger_cpu;
t_configuracion_cpu* configuracion_cpu;

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

typedef struct t_direccion_fisica{
    uint32_t numero_pagina;
    uint32_t entrada_tabla_1er_nivel;
    uint32_t entrada_tabla_2do_nivel;
    uint32_t desplazamiento;
}t_direccion_fisica;


#endif
