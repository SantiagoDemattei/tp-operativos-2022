#include "../include/conexion.h"

int crear_conexion(t_datos_conexion* datos_conexion)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(datos_conexion->ip,  datos_conexion->puerto, &hints, &server_info);
    printf("estoy conectando a la ip %s\n", datos_conexion->ip); 
    printf("estoy conectando a el puerto %s\n", datos_conexion->puerto);
	// Ahora vamos a crear el socket.
	int socket_kernel = socket(server_info -> ai_family, server_info -> ai_socktype, server_info -> ai_protocol);
	// Ahora que tenemos el socket, vamos a conectarlo
    if(connect(socket_kernel, server_info -> ai_addr, server_info -> ai_addrlen) == -1){ // aca ya deberia estar corriendo el kernel esperando conexion
    	printf("error, no se ha encontrado conexion\n"); 
    }
    else{ printf("Conectado a Kernel");}

	freeaddrinfo(server_info);

	return socket_kernel;
}