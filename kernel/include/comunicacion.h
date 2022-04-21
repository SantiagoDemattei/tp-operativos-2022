#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include "configuracion.h"

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"

typedef struct {
    t_log* log;
    int fd;
    char* server_name;
} t_procesar_conexion_args;

int crear_comunicacion(t_config_kernel* configuracion_kernel, t_log* logger);
int server_escuchar(t_log* logger, char* server_name, int server_socket);
static void procesar_conexion(void* void_args);

#endif 