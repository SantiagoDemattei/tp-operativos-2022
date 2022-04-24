#include "include/main.h"

int main(void){

    int socket_kernel;   	
    t_log* logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_configuracion_kernel* configuracion_kernel = leer_configuracion();
    socket_kernel = crear_comunicacion(configuracion_kernel, logger);

    while(server_escuchar(logger, "KERNEL", socket_kernel)){
        /*
        HACER COSAS DEL KERNEL
        */
    }

    liberar_estructuras(configuracion_kernel);
    log_destroy(logger);

    return EXIT_SUCCESS;
}
