#include "../include/conexion.h"

uint32_t crear_conexion_consola(t_configuracion_consola* datos_conexion, t_log* logger)
{
	uint32_t socket_Kernel = crear_conexion_cliente(logger,"KERNEL",datos_conexion->ip,datos_conexion->puerto); //socket del servidor (kernel) al que se conecto
	 
	return socket_Kernel;
}

