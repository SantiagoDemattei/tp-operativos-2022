#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <string.h>
#include <commons/log.h>
#include "configuracion.h"
#include "conexion.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"


int iniciar_consola(int tamanio, char* path, t_log* logger);	
t_list* obtener_instrucciones(char* path, t_log* logger);
t_instruccion* crear_instruccion(char* instruccion, t_log* logger);
void destruir_instruccion(t_instruccion* instruccion);
void destruir_argumentos(t_argumento* argumento);
void loggear_lista_instrucciones(t_list* lista_instrucciones, t_log* logger);


#endif
