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

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

int crear_conexion_consola(t_config_consola* datos_conexion, t_log* logger);
void enviar_mensaje(char* mensaje, int socket_cliente);
void* serializar_paquete(t_paquete* paquete, int bytes);
void eliminar_paquete(t_paquete* paquete);
// void liberar_conexion(int socket_cliente);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
t_paquete* crear_paquete(void);
void crear_buffer(t_paquete* paquete);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);



#endif
