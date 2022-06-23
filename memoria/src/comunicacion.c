#include "../include/comunicacion.h"
uint32_t contador = 0;

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
    uint32_t *cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

    uint32_t id_proceso;
    uint32_t tamanio_proceso;
    op_code cop;
    uint32_t valor;
    uint32_t num_pagina;

    while (*cliente_socket != -1)
    { // mientras el cliente no se haya desconectado
        if (recv(*cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code))
        { // desconectamos al cliente xq no le esta mandando el cop bien
            loggear_info(logger, "DISCONNECT!\n", mutex_logger_memoria);
            return;
        }
        switch (cop)
        {
        case DEBUG:
            loggear_info(logger, "debug\n", mutex_logger_memoria);
            break;

        case ORDEN_ENVIO_TAMANIO:
            printf("el tamnanio de pagina es: %d\n", (configuracion_memoria->tam_pagina));
            send_valor_y_num_pagina (*cliente_socket, (configuracion_memoria->tam_pagina),(configuracion_memoria->entradas_por_tabla)); //envio el tamanio de pagina a la cpu
            loggear_info(logger, "Envie tamanio de pagina a CPU\n", mutex_logger_memoria);        
            break;


        case INICIALIZAR_ESTRUCTURAS:
            recv_inicializar_estructuras(*cliente_socket, &tamanio_proceso, &id_proceso);
            loggear_info(logger, "INICIALIZANDO ESTRUCTURAS\n", mutex_logger_memoria);
            t_estructura_proceso *estructura = malloc(sizeof(t_estructura_proceso));
            estructura->id_proceso = id_proceso;
            
            estructura->espacio_en_memoria = malloc(tamanio_proceso);

            // estructura de memoria = espacio + lista de tablas + archivo
            t_tabla_pagina1 *tabla_pagina1 = malloc(sizeof(t_tabla_pagina1) + sizeof(uint32_t) * configuracion_memoria->entradas_por_tabla);
            tabla_pagina1->id_tabla = contador;
            //inicializar en null las filas de la tabla
            for (int i = 0; i < configuracion_memoria->entradas_por_tabla; i++)
            {
                tabla_pagina1->primer_nivel[i] = NULL;
            }

            estructura->tabla_pagina1 = tabla_pagina1;
            estructura->lista_tablas_segundo_nivel = list_create();           
            char* proceso_string = malloc(strlen("/proceso_") +strlen(string_itoa(id_proceso)));
            string_append(&proceso_string, "/proceso_");
            string_append( &proceso_string , string_itoa(id_proceso));
            char* path_archivo = malloc(strlen( configuracion_memoria->path_swap + strlen(proceso_string)));
            string_append(&path_archivo, configuracion_memoria->path_swap);
            string_append(&path_archivo, proceso_string);
     
             
            estructura->nombre_archivo_swap = malloc(strlen(path_archivo)); // "/home/utnso/swap/proceso_xx" (el +1 es por el \0)
            
            
            string_append(&estructura->nombre_archivo_swap, path_archivo);
            printf("El nombre del archivo es: %s\n", estructura->nombre_archivo_swap);
            
            list_add_con_mutex_tablas(lista_estructuras, estructura, mutex_lista_estructuras);

            send_valor_tb(*cliente_socket, tabla_pagina1->id_tabla); //le mandamos el id de la tabla que corresponde al proceso (es lo mismo que el contador)

            pthread_mutex_lock(&mutex_valor_tp);
            contador++; // valor de tabla de paginas NO VA A QUEDAR ASI
            pthread_mutex_unlock(&mutex_valor_tp);

            free(cliente_socket);

            loggear_info(logger, "Se envio el valor de la tabla de paginas al kernel\n", mutex_logger_memoria);
            break;
        
        case OBTENER_MARCO:
            recv_valor_y_num_pagina(*cliente_socket, &num_pagina, &valor);
            //HAY QUE PONER EL VALOR EN EL NUMERO DE PAGINA QUE CORRESPONDE DE LA TABLA DE PAGINAS
            //TODO: ESTO
            //send_valor_tb(*cliente_socket, marco);
            break;

        case LIBERAR_ESTRUCTURAS:
            loggear_info(logger, "LIBERANDO ESTRUCTURAS", mutex_logger_memoria);

            // ACA VA EL CODIGO PARA LIBERAR LAS ESTRUCTURAS
            send_fin_proceso(*cliente_socket);
            free(cliente_socket);
            break;
        
        case SUSPENSION:
            loggear_info(logger, "SUSPENDIENDO PROCESO\n", mutex_logger_memoria);
            recv_suspension(*cliente_socket, &id_proceso);
            //FALTA VER QUE HACE
            loggear_info(logger, "PROCESO SUSPENDIDO\n", mutex_logger_memoria);
            printf("suspendi proceso %d\n", id_proceso);
            send_confirmacion_suspension(*cliente_socket);
            free(cliente_socket);
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
    uint32_t* cliente_socket = esperar_cliente(logger, server_name, server_socket); // espera a que se conecte un cliente

    if (*cliente_socket != -1)
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

void list_add_con_mutex_tablas(t_list* lista, t_estructura_proceso* tabla_pagina1 , pthread_mutex_t mutex){
    pthread_mutex_lock(&mutex);
    list_add(lista, tabla_pagina1);
    pthread_mutex_unlock(&mutex);
}