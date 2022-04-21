#include "include/main.h"

int main(void){

    int socket_kernel;   	
    t_log* logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    t_config_kernel* configuracion_kernel = leer_configuracion();
    socket_kernel = crear_comunicacion(configuracion_kernel, logger);


    liberar_estructuras(configuracion_kernel);
    log_destroy(logger);

    return EXIT_SUCCESS;
}
