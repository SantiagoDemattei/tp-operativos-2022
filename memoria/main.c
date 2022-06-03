#include "include/main.h"

uint32_t socket_memoria; 
t_log* logger;
t_configuracion_memoria* configuracion_memoria;

void sighandler(int x) {
    switch (x) {
        case SIGINT:
            liberar_conexion(socket_memoria);
            log_destroy(logger);
            liberar_estructuras_memoria(configuracion_memoria);
            exit(EXIT_SUCCESS);
    }
}

uint32_t main(void){

    signal(SIGINT , sighandler);

    logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    configuracion_memoria = leer_configuracion();
    socket_memoria = crear_comunicacion_kernel(configuracion_memoria, logger); //inicia el servidor para que el kernel se conecte 
    inicializar_semaforos();

    while(server_escuchar(logger, "MEMORIA", socket_memoria)!=0); // servidor de kernel y cpu (ambos se conectan a la misma ip y puerto)

    liberar_conexion(socket_memoria);
    liberar_estructuras_memoria(configuracion_memoria);
    log_destroy(logger);

    return EXIT_SUCCESS;
}
