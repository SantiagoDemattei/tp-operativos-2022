#include "../include/comunicacion.h"

int crear_comunicacion(t_config_kernel* configuracion_kernel, t_log* logger){
    
    int socket_kernel = iniciar_servidor(logger, "CONSOLA", configuracion_kernel->ip_memoria, configuracion_kernel->puerto_escucha);

    int socket_cliente= esperar_cliente(logger, "CONSOLA", socket_kernel);
    return socket_kernel;
}

