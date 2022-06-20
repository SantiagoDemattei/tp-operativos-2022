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

t_configuracion_memoria* leer_configuracion();

t_configuracion_memoria* configuracion_memoria;

pthread_mutex_t mutex_logger_memoria;
pthread_mutex_t mutex_valor_tp;
pthread_mutex_t mutex_lista_tablas;
pthread_mutex_t mutex_tamanio_pagina;



#endif