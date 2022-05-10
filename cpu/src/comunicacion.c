#include "../include/comunicacion.h"

uint32_t crear_comunicacion(t_configuracion_cpu* t_configuracion_cpu, t_log* logger){ //funcion de servidor 
    
    uint32_t socket_cpu = iniciar_servidor(logger, "CPU", t_configuracion_cpu->ip_memoria, t_configuracion_cpu->puerto_escucha_dispatch);

    if(socket_cpu == -1){
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        return -1;
    }
    return socket_cpu;
}

uint32_t crear_conexion_memoria(t_configuracion_cpu* datos_conexion, t_log* logger) //funcion de cliente de memoria
{
	uint32_t socket_memoria = crear_conexion_cliente(logger, "MEMORIA", datos_conexion->ip_memoria , datos_conexion->puerto_memoria);
	 
	return socket_memoria;
}

static void procesar_conexion(void* void_args){
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args; // recibo a mi cliente y sus datos
    t_log* logger = args->log;
    uint32_t cliente_socket = args->fd;
    char* server_name = args->server_name;
    t_pcb* pcb = malloc(sizeof(t_pcb));
    free(args);

    op_code cop;

     while (cliente_socket != -1){
         if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }

        switch (cop) {
            case DEBUG:
                log_info(logger, "debug");
                break;
            
            case ENVIAR_PCB:
                log_info(logger, "Se recibio el pcb");
                

                if(recv_pcb(cliente_socket, &pcb)){
                    
                    loggear_lista_instrucciones(pcb->instrucciones,logger);

                    /*
                    int socket_memoria = crear_conexion_memoria(configuracion_cpu, logger);

                    if(socket_memoria == -1){
                        log_error(logger, "No se pudo conectar con la memoria");
                        return;
                    }
                    send_debug(socket_memoria);
                    */
                }
                    

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