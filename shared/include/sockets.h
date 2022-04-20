#ifndef SOCKETS_H_
#define SOCKETS_H_

#include<sys/socket.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<unistd.h>
#include<string.h>

int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto);
int esperar_cliente(t_log* logger, const char* name, int socket_servidor);
//int crear_conexion(t_log* logger, const char* server_name, char* ip, char* puerto);
void liberar_conexion(int socket_cliente);


#endif