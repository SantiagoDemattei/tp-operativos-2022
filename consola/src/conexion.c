#include "../include/conexion.h"

uint32_t crear_conexion_consola(t_configuracion_consola* datos_conexion, t_log* logger)
{
	uint32_t socket_Kernel = crear_conexion_cliente(logger,"KERNEL",datos_conexion->ip,datos_conexion->puerto);
	 
	return socket_Kernel;
}


/*
void liberar_conexion(uint32_t socket_cliente)
{
	close(socket_cliente);
}*/
