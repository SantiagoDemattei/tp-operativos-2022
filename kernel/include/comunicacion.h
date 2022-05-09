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


uint32_t socket_kernel; 
t_log* logger;
t_configuracion_kernel* configuracion_kernel;
pthread_mutex_t mutex_cantidad_procesos;



uint32_t crear_comunicacion(t_configuracion_kernel* configuracion_kernel, t_log* logger);
uint32_t server_escuchar(t_log* logger, char* server_name, uint32_t server_socket);
t_pcb* crear_pcb(t_list* instrucciones, uint32_t socket_cpu, t_log* logger, uint32_t);
static void procesar_conexion(void* void_args);
void incrementar_cantidad_procesos();



#endif 
