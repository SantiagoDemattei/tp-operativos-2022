#include "../include/comunicacion.h"

uint32_t crear_comunicacion_kernel(t_configuracion_memoria* configuracion_memoria, t_log* logger){ //SERVIDOR DE KERNEL
    
    uint32_t socket_memoria = iniciar_servidor(logger, "KERNEL", configuracion_memoria->ip_memoria, configuracion_memoria->puerto_escucha);

    if(socket_memoria == -1){
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        return -1;
    }

    return socket_memoria;
}


uint32_t crear_comunicacion_cpu(t_configuracion_memoria* configuracion_memoria, t_log* logger){ //SERVIDOR DE CPU
    
 socket_cpu_dispatch = iniciar_servidor(logger, "CPU", configuracion_memoria->ip_memoria, configuracion_memoria->puerto_escucha);

    if(socket_cpu_dispatch == -1){
     log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        return -1;
    }

    return socket_cpu_dispatch;
}

static void procesar_conexion(void* void_args){  //entre el kernel (cliente) y la memoria (server)
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args; // recibo a mi cliente y sus datos
    t_log* logger = args->log;
    uint32_t cliente_socket = args->fd;
    char* server_name = args->server_name;
    free(args);
    uint32_t mensaje; 
    uint32_t valor_tb;
    op_code cop; 

     while (cliente_socket != -1){  //mientras el cliente no se haya desconectado
         if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) { //desconectamos al cliente xq no le esta mandando el cop bien 
            log_info(logger, "DISCONNECT!");
            return;
        }
        switch (cop) {
            case DEBUG: 
                log_info(logger, "debug");
                break;
                
            case INICIALIZAR_ESTRUCTURAS:
                    log_info(logger, "INICIALIZANDO ESTRUCTURAS"); //FALTA INICIALIZAR ESTRUCTURAS 

                    valor_tb = 2123; // valor de tabla de paginas NO VA A QUEDAR ASI
                    
                    send_valor_tb(cliente_socket, valor_tb); 
                    log_info(logger, "Se envio el valor de la tabla de paginas al kernel\n");
                 break;
            
            // Errores
            case -1:
                log_error(logger, "Cliente desconectado de %s...", server_name);
                return;
            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                return;
        }
    }

    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    return;
}

uint32_t server_escuchar(t_log* logger, char* server_name, uint32_t server_socket) {
    uint32_t cliente_socket = esperar_cliente(logger, server_name, server_socket); // espera a que se conecte un cliente

    if (cliente_socket != -1) { // si se conecto un cliente
        pthread_t hilo; // crea un hilo para procesar la conexion
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args)); // crea una estructura para pasarle los argumentos al hilo
        args->log = logger; // guarda el logger en la estructura
        args->fd = cliente_socket; // guarda el socket del cliente en la estructura
        args->server_name = server_name;  // guarda el nombre del servidor en la estructura
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args); // crea el hilo
        pthread_detach(hilo); // lo desconecta del hilo
        return 1; // devuelve 1 para indicar que se conecto un cliente
    }
    return 0;
}