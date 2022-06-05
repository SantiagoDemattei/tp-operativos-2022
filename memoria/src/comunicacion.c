#include "../include/comunicacion.h"

uint32_t crear_comunicacion_kernel(t_configuracion_memoria *configuracion_memoria, t_log *logger)
{ // SERVIDOR DE KERNEL

    uint32_t socket_memoria = iniciar_servidor(logger, "KERNEL", configuracion_memoria->ip_memoria, configuracion_memoria->puerto_escucha);

    if (socket_memoria == -1)
    {
        loggear_error(logger, "No se pudo iniciar el servidor de comunicacion\n", mutex_logger_memoria);
        return -1;
    }

    return socket_memoria;
}

uint32_t crear_comunicacion_cpu(t_configuracion_memoria *configuracion_memoria, t_log *logger)
{ // SERVIDOR DE CPU

    socket_cpu_dispatch = iniciar_servidor(logger, "CPU", configuracion_memoria->ip_memoria, configuracion_memoria->puerto_escucha);

    if (socket_cpu_dispatch == -1)
    {
        loggear_error(logger, "No se pudo iniciar el servidor de comunicacion\n", mutex_logger_memoria);
        return -1;
    }

    return socket_cpu_dispatch;
}

static void procesar_conexion(void *void_args)
{                                                                           // entre el kernel (cliente) y la memoria (server)
    t_procesar_conexion_args *args = (t_procesar_conexion_args *)void_args; // recibo a mi cliente y sus datos
    t_log *logger = args->log;
    uint32_t cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);
    uint32_t mensaje;
    uint32_t valor_tb;
    op_code cop;

    while (cliente_socket != -1)
    { // mientras el cliente no se haya desconectado
        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code))
        { // desconectamos al cliente xq no le esta mandando el cop bien
            loggear_info(logger, "DISCONNECT!\n", mutex_logger_memoria);
            return;
        }
        switch (cop)
        {
        case DEBUG:
            loggear_info(logger, "debug\n", mutex_logger_memoria);
            break;

        case INICIALIZAR_ESTRUCTURAS:
            loggear_info(logger, "INICIALIZANDO ESTRUCTURAS\n", mutex_logger_memoria);

            pthread_mutex_lock(&mutex_valor_tp);
            valor_tb = 2123; // valor de tabla de paginas NO VA A QUEDAR ASI
            pthread_mutex_unlock(&mutex_valor_tp);

            pthread_mutex_lock(&mutex_valor_tp);
            send_valor_tb(cliente_socket, valor_tb);
            pthread_mutex_unlock(&mutex_valor_tp);
            

            loggear_info(logger, "Se envio el valor de la tabla de paginas al kernel\n", mutex_logger_memoria);
            break;

        case LIBERAR_ESTRUCTURAS:
            loggear_info(logger, "LIBERANDO ESTRUCTURAS", mutex_logger_memoria);

            // ACA VA EL CODIGO PARA LIBERAR LAS ESTRUCTURAS

            send_fin_proceso(cliente_socket);
            break;

        // Errores
        case -1:
            pthread_mutex_lock(&mutex_logger_memoria);
            log_error(logger, "Cliente desconectado de %s...", server_name);
            pthread_mutex_unlock(&mutex_logger_memoria);
            return;
        default:
            pthread_mutex_lock(&mutex_logger_memoria);
            log_error(logger, "Algo anduvo mal en el server de %s", server_name);
            pthread_mutex_unlock(&mutex_logger_memoria);
            return;
        }
    }

    pthread_mutex_lock(&mutex_logger_memoria);
    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    pthread_mutex_unlock(&mutex_logger_memoria);
    return;
}

uint32_t server_escuchar(t_log *logger, char *server_name, uint32_t server_socket)
{
    uint32_t cliente_socket = esperar_cliente(logger, server_name, server_socket); // espera a que se conecte un cliente

    if (cliente_socket != -1)
    {                                                                              // si se conecto un cliente
        pthread_t hilo;                                                            // crea un hilo para procesar la conexion
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args)); // crea una estructura para pasarle los argumentos al hilo
        args->log = logger;                                                        // guarda el logger en la estructura
        args->fd = cliente_socket;                                                 // guarda el socket del cliente en la estructura
        args->server_name = server_name;                                           // guarda el nombre del servidor en la estructura
        pthread_create(&hilo, NULL, (void *)procesar_conexion, (void *)args);      // crea el hilo
        pthread_detach(hilo);                                                      // lo desconecta del hilo
        return 1;                                                                  // devuelve 1 para indicar que se conecto un cliente
    }
    return 0;
}