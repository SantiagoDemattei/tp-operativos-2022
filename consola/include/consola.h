#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <string.h>
#include <commons/log.h>
#include "configuracion.h"
#include "conexion.h"
#include "../../shared/include/sockets.h"

typedef struct {
    char* instruccion;
} t_instruccion;

void iniciar_consola(int tamanio, char* path);
t_list* obtener_instrucciones(char* path);
t_instruccion* crear_instruccion(char* instruccion);
void destruir_instruccion(t_instruccion* instruccion);


#endif
