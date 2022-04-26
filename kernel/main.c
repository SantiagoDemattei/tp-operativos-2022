#include "include/main.h"

uint32_t socket_kernel; 
t_log* logger;
t_configuracion_kernel* configuracion_kernel;

void sighandler(int x) {
    switch (x) {
        case SIGINT:
            liberar_conexion(socket_kernel);
            log_destroy(logger);
            liberar_estructuras(configuracion_kernel);
            exit(EXIT_SUCCESS);
    }
}

uint32_t main(void){

    signal(SIGINT , sighandler);
      	
    logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    configuracion_kernel = leer_configuracion();
    socket_kernel = crear_comunicacion(configuracion_kernel, logger);

    while(server_escuchar(logger, "KERNEL", socket_kernel)!=0);

    liberar_conexion(socket_kernel);
    liberar_estructuras(configuracion_kernel);
    log_destroy(logger);

    return EXIT_SUCCESS;
}
