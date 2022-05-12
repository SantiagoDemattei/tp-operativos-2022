#include "include/main.h"

void sighandler(int x) {
    switch (x) {
        case SIGINT:
            liberar_conexion(socket_cpu);
            log_destroy(logger_cpu);
            liberar_estructuras_cpu(configuracion_cpu);
            exit(EXIT_SUCCESS);
    }
}

uint32_t main(void){

    signal(SIGINT, sighandler);
      	
    logger_cpu = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
    configuracion_cpu = leer_configuracion(logger_cpu);
    socket_cpu = crear_comunicacion(configuracion_cpu, logger_cpu); // servidor de kernel

    while(server_escuchar(logger_cpu, "CPU", socket_cpu)!=0);

    liberar_conexion(socket_cpu);
    liberar_estructuras_cpu(configuracion_cpu);
    log_destroy(logger_cpu);

    return EXIT_SUCCESS;
}
