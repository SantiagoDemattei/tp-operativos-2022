#include "../include/comunicacion.h"

int crear_comunicacion(t_configuracion_kernel* configuracion_kernel, t_log* logger){
    
    int socket_kernel = iniciar_servidor(logger, "CONSOLA", configuracion_kernel->ip_memoria, configuracion_kernel->puerto_escucha);

    if(socket_kernel == -1){
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        return -1;
    }

    return socket_kernel;
}


static void procesar_conexion(void* void_args){
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args; // recibo a mi cliente y sus datos
    t_log* logger = args->log;
    int cliente_socket = args->fd;
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
                
                int tamanio;
                t_list* instrucciones ;
                if(recv_iniciar_consola(cliente_socket, &instrucciones, &tamanio)){
                    log_info(logger, "Se recibieron las instrucciones");
                    log_info(logger, "Tamanio de la consola: %d",tamanio);
                    log_info(logger, "Cantidad de instrucciones: %d", list_size(instrucciones));
                    loggear_lista_instrucciones(instrucciones, logger);
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

int server_escuchar(t_log* logger, char* server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket); // espera a que se conecte un cliente

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

