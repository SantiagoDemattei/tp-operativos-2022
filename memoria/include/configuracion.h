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

t_configuracion_memoria* leer_configuracion();

pthread_mutex_t mutex_logger_memoria;
pthread_mutex_t mutex_valor_tp;

#endif