#ifndef CONEXION_H_
#define CONEXION_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include "configuracion.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/utils.h"

uint32_t crear_conexion_consola(t_configuracion_consola* datos_conexion, t_log* logger);

// void liberar_conexion(uint32_t socket_cliente);

#endif
