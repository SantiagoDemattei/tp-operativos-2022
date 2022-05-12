#include "../include/comunicacion.h"

uint32_t cantidad_procesos = 0;

uint32_t crear_comunicacion(t_configuracion_kernel* configuracion_kernel, t_log* logger){ //funcion de servidor 
    
    uint32_t socket_kernel = iniciar_servidor(logger, "CONSOLA", configuracion_kernel->ip_memoria, configuracion_kernel->puerto_escucha);

    if(socket_kernel == -1){
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        return -1;
    }

    return socket_kernel;
}

uint32_t crear_conexion_cpu(t_configuracion_kernel* datos_conexion, t_log* logger) //"kernel" cliente de cpu
{
	uint32_t socket_cpu = crear_conexion_cliente(logger, "CPU", datos_conexion->ip_cpu, datos_conexion->puerto_cpu_dispatch);
    
	return socket_cpu;
}

uint32_t crear_conexion_memoria(t_configuracion_kernel* datos_conexion, t_log* logger) //"kernel" cliente de memoria
{
	uint32_t socket_memoria = crear_conexion_cliente(logger, "MEMORIA", datos_conexion->ip_memoria, datos_conexion->puerto_memoria);
    
	return socket_memoria;
}


static void procesar_conexion(void* void_args){
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args; // recibo a mi cliente y sus datos
    t_log* logger = args->log;
    uint32_t cliente_socket = args->fd;
    char* server_name = args->server_name;
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

            case INICIAR_PROCESO:
            {
                
                uint32_t tamanio = 0;
                
                t_list* instrucciones = NULL;
                if(recv_iniciar_consola(cliente_socket, &instrucciones, &tamanio)){
                    log_info(logger, "Se recibieron las instrucciones");
                    log_info(logger, "Tamanio de la consola: %d",tamanio);
                    log_info(logger, "Cantidad de instrucciones: %d", list_size(instrucciones));
                    // loggear_lista_instrucciones(instrucciones, logger);

                    // envio instrucciones a cpu
                    int socket_cpu = crear_conexion_cpu(configuracion_kernel, logger);
                    int socket_memoria = crear_conexion_memoria(configuracion_kernel, logger);
                    t_pcb* pcb = crear_pcb(instrucciones, socket_cpu, logger, tamanio);
                    send_pcb(socket_cpu, pcb);
                    free(pcb);

                    send_debug(socket_memoria);
                    
                    // liberar memoria
                    list_destroy_and_destroy_elements(instrucciones, (void*) destruir_instruccion);
                }
                else{
                    log_error(logger, "No se recibieron las instrucciones");
                }
                break;
            }

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

 t_pcb* crear_pcb(t_list* instrucciones, uint32_t socket_cpu, t_log* logger, uint32_t tamanio){
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pthread_mutex_lock(&mutex_cantidad_procesos);
    pcb->id = cantidad_procesos;
    cantidad_procesos++;
    pthread_mutex_unlock(&mutex_cantidad_procesos);
    pcb->instrucciones = instrucciones;
    pcb->program_counter = 0 ;
    pcb->tamanio = tamanio;
    pcb->tabla_pagina = 0;
    pcb->estimacion_rafaga = 0;
    return pcb;
 }

