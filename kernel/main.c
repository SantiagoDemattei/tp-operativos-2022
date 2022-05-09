#include "include/main.h"

void sighandler(int x) {
    switch (x) {
        case SIGINT:
            liberar_conexion(socket_kernel);
            log_destroy(logger);
            liberar_estructuras_kernel(configuracion_kernel);
            exit(EXIT_SUCCESS);
    }
}

uint32_t main(void){

    signal(SIGINT , sighandler);
      	
    logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    configuracion_kernel = leer_configuracion();
    socket_kernel = crear_comunicacion(configuracion_kernel, logger);
    pthread_mutex_init(&mutex_cantidad_procesos, NULL);

    while(server_escuchar(logger, "CONSOLA", socket_kernel)!=0);

    liberar_conexion(socket_kernel);
    liberar_estructuras_kernel(configuracion_kernel);
    log_destroy(logger);

    return EXIT_SUCCESS;
}
