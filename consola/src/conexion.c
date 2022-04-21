#include "../include/conexion.h"

int crear_conexion_consola(t_config_consola* datos_conexion, t_log* logger)
{
	int socket_Kernel = crear_conexion_cliente(logger,"KERNEL",datos_conexion->ip,datos_conexion->puerto);
	 
	return socket_Kernel;
}

/*
void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}*/