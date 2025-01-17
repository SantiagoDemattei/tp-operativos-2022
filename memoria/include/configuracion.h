#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/estructuras.h"
#include "../../shared/include/utils.h"

t_configuracion_memoria* leer_configuracion(char* path_config);
t_estructura_swap* variable_global;
t_configuracion_memoria* configuracion_memoria;
t_log* logger;
pthread_mutex_t mutex_logger_memoria;
pthread_mutex_t mutex_valor_tp;
pthread_mutex_t mutex_lista_estructuras;
pthread_mutex_t mutex_tamanio_pagina;
pthread_mutex_t mutex_estructura_proceso_actual;
pthread_mutex_t mutex_marcos;
pthread_mutex_t mutex_espacio_memoria;
pthread_mutex_t mutex_variable_global;
sem_t sem_swap;
sem_t sem_fin_swap;
sem_t sem_creacion_archivo_swap;
void* espacio_memoria;
int cant_total_marcos;
t_list* marcos_totales; 
#endif