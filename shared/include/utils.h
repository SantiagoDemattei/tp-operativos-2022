#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include "estructuras.h"

void loggear_lista_instrucciones(t_list* lista_instrucciones, t_log* logger);
void destruir_argumentos(t_argumento* argumento);
void destruir_instruccion(t_instruccion* instruccionS);

#endif