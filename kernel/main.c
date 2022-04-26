#include "include/main.h"

int socket_kernel; 
t_log* logger;

void sighandler(int x) {
    switch (x) {
        case SIGINT:
            liberar_conexion(socket_kernel);
            log_destroy(logger);
            exit(EXIT_SUCCESS);
    }
}

int main(void){

    signal(SIGINT , sighandler);
      	
    logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_configuracion_kernel* configuracion_kernel = leer_configuracion();
    socket_kernel = crear_comunicacion(configuracion_kernel, logger);

    while(server_escuchar(logger, "KERNEL", socket_kernel)!=0);

    liberar_conexion(socket_kernel);
    liberar_estructuras(configuracion_kernel);
    log_destroy(logger);

    return EXIT_SUCCESS;
}
