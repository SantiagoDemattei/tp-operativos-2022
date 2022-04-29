#include "include/main.h"

uint32_t socket_cpu; 
t_log* logger_cpu;
t_configuracion_cpu* configuracion_cpu;

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

    signal(SIGINT , sighandler);
      	
    logger_cpu = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
    configuracion_cpu = leer_configuracion(logger_cpu);
    socket_cpu = crear_comunicacion(configuracion_cpu, logger_cpu);

    while(server_escuchar(logger_cpu, "CPU", socket_cpu)!=0);

    liberar_conexion(socket_cpu);
    liberar_estructuras_cpu(configuracion_cpu);
    log_destroy(logger_cpu);

    return EXIT_SUCCESS;
}
