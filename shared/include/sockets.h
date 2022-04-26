#ifndef SOCKETS_H_
#define SOCKETS_H_

#include<sys/socket.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<unistd.h>
#include<string.h>
#include "commons/collections/dictionary.h"
#include <commons/config.h>

uint32_t iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto);
uint32_t esperar_cliente(t_log* logger, const char* name, uint32_t socket_servidor);
uint32_t crear_conexion_cliente(t_log* logger, const char* server_name, char* ip, char* puerto);
void liberar_conexion(uint32_t socket_cliente);



#endif