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
    uint32_t valor_a_escribir;
    uint32_t num_pagina;
    uint32_t entrada_primer_nivel;
    uint32_t id_tabla1;
    uint32_t num_tabla_segundo_nivel;
    uint32_t num_segundo_nivel;
    uint32_t entrada_tabla_2do_nivel;
    t_marco_presencia *marco_presencia = malloc(sizeof(t_marco_presencia));
    uint32_t frame;
    uint32_t desplazamiento;
    uint32_t valor_leido;
    uint32_t frame_origen;
    uint32_t frame_destino;
    uint32_t desplazamiento_origen;
    uint32_t desplazamiento_destino;

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
            printf("el tamanio de pagina es: %d\n", (configuracion_memoria->tam_pagina));
            send_tamanio_y_cant_entradas(*cliente_socket, (configuracion_memoria->tam_pagina), (configuracion_memoria->entradas_por_tabla)); // envio el tamanio de pagina a la cpu
            loggear_info(logger, "Envie tamanio de pagina a CPU\n", mutex_logger_memoria);
            break;

        case INICIALIZAR_ESTRUCTURAS:
            recv_inicializar_estructuras(*cliente_socket, &tamanio_proceso, &id_proceso);
            loggear_info(logger, "INICIALIZANDO ESTRUCTURAS\n", mutex_logger_memoria);
            t_estructura_proceso *estructura = malloc(sizeof(t_estructura_proceso));
            estructura->id_proceso = id_proceso;
            estructura->espacio_en_memoria = malloc(tamanio_proceso);

            // estructura de memoria = espacio + lista de tablas + archivo
            t_tabla_pagina1 *tabla_pagina1 = malloc(sizeof(t_tabla_pagina1) + sizeof(uint32_t) * (configuracion_memoria->entradas_por_tabla));
            tabla_pagina1->id_tabla = contador;
            tabla_pagina1->primer_nivel = list_create();
            estructura->tabla_pagina1 = tabla_pagina1;

            estructura->lista_tablas_segundo_nivel = list_create(); // lista de tablas de segundo nivel
            // creo cada una de las tablas de segundo nivel
            for (int i = 0; i < configuracion_memoria->entradas_por_tabla; i++)
            {
                t_tabla_pagina2 *tabla_segundo_nivel = malloc(sizeof(t_tabla_pagina2) + sizeof(uint32_t) * (configuracion_memoria->entradas_por_tabla));
                tabla_segundo_nivel->id_tabla = i;
                tabla_segundo_nivel->segundo_nivel = list_create();
                list_add(estructura->lista_tablas_segundo_nivel, tabla_segundo_nivel);
            }
            printf("id_proceso: %d\n", id_proceso);
            char *proceso_string = malloc(strlen("/proceso_") + strlen(string_itoa(id_proceso)));
            proceso_string = string_from_format("/proceso_%d", id_proceso);
            char *path_archivo = malloc(strlen(configuracion_memoria->path_swap + strlen(proceso_string)));
            string_append(&path_archivo, configuracion_memoria->path_swap);
            string_append(&path_archivo, proceso_string);

            estructura->nombre_archivo_swap = malloc(strlen(path_archivo)); // "/home/utnso/swap/proceso_xx" (el +1 es por el \0)
            string_append(&estructura->nombre_archivo_swap, path_archivo);
            
            loggear_info(logger, string_from_format("El nombre del archivo es: %s\n", estructura->nombre_archivo_swap), mutex_logger_memoria);
            list_add_con_mutex_tablas(lista_estructuras, estructura, mutex_lista_estructuras);

            send_valor_tb(*cliente_socket, tabla_pagina1->id_tabla); // le mandamos el id de la tabla que corresponde al proceso (es lo mismo que el contador)

            pthread_mutex_lock(&mutex_valor_tp);
            contador++;
            pthread_mutex_unlock(&mutex_valor_tp);

            free(cliente_socket);

            loggear_info(logger, "Se envio el valor de la tabla de paginas al kernel\n", mutex_logger_memoria);
            break;

        case PRIMER_ACCESO:
            recv_entrada_tabla_1er_nivel(*cliente_socket, &id_tabla1, &entrada_primer_nivel);
            loggear_info(logger, "Se recibio la entrada de la tabla de paginas de primer nivel\n", mutex_logger_memoria);
            num_tabla_segundo_nivel = obtener_tabla_2do_nivel(id_tabla1, entrada_primer_nivel);
            send_num_tabla_2do_nivel(*cliente_socket, num_tabla_segundo_nivel);

        case SEGUNDO_ACCESO:
            recv_entrada_tabla_2do_nivel(*cliente_socket, &num_segundo_nivel, &entrada_tabla_2do_nivel);
            loggear_info(logger, "Se recibio la entrada de la tabla de pagina de segundo nivel\n", mutex_logger_memoria);
            // busco el frame en la tabla de segundo nivel
            marco_presencia = obtener_frame(num_segundo_nivel, entrada_tabla_2do_nivel);
            send_frame(*cliente_socket, marco_presencia);
            break;

        case EJECUTAR_WRITE:
            recv_ejecutar_write(*cliente_socket, &frame, &desplazamiento, &valor_a_escribir);
            loggear_info(logger, "Se recibio orden de escritura\n", mutex_logger_memoria);
            escribir_valor(frame, desplazamiento, valor_a_escribir);
            send_ok(*cliente_socket);
            break;
            
        case EJECUTAR_READ:
            recv_ejecutar_read(*cliente_socket, &frame, &desplazamiento);
            loggear_info(logger, "Se recibio orden de lectura\n", mutex_logger_memoria);
            valor_leido = leer_valor(frame, desplazamiento);
            send_ok_read(*cliente_socket, valor_leido);
            break;

        case EJECUTAR_COPY:
            recv_ejecutar_copy(*cliente_socket, &frame_origen, &desplazamiento_origen, &frame_destino, &desplazamiento_destino);
            loggear_info(logger, "Se recibio orden de copia\n", mutex_logger_memoria);
            copiar_valor(frame_origen, desplazamiento_origen, frame_destino, desplazamiento_destino);
            send_ok(*cliente_socket);
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
            // FALTA VER QUE HACE
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
    uint32_t *cliente_socket = esperar_cliente(logger, server_name, server_socket); // espera a que se conecte un cliente

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

void list_add_con_mutex_tablas(t_list *lista, t_estructura_proceso *tabla_pagina1, pthread_mutex_t mutex)
{
    pthread_mutex_lock(&mutex);
    list_add(lista, tabla_pagina1);
    pthread_mutex_unlock(&mutex);
}

t_tabla_pagina1 *buscar_tabla_pagina1(uint32_t id_tabla)
{
    t_estructura_proceso *estructura = NULL;
    t_tabla_pagina1 *tabla_pagina1 = NULL;
    int i;
    for (i = 0; i < list_size(lista_estructuras); i++)
    {
        estructura = list_get(lista_estructuras, i);
        if (estructura->tabla_pagina1->id_tabla == id_tabla)
        {
            pthread_mutex_lock(&mutex_estructura_proceso_actual);
            estructura_proceso_actual = estructura;
            pthread_mutex_unlock(&mutex_estructura_proceso_actual);
            return tabla_pagina1;
        }
    }
    return NULL;
}

uint32_t buscar_nro_tabla_segundo_nivel(t_tabla_pagina1 *tabla_pagina1, uint32_t entrada_tabla_1er_nivel)
{
    t_list *tabla_primer_nivel = tabla_pagina1->primer_nivel;
    uint32_t *retorno = list_get(tabla_primer_nivel, entrada_tabla_1er_nivel);
    return *retorno;
}

uint32_t obtener_tabla_2do_nivel(uint32_t id_tabla, uint32_t entrada_primer_nivel)
{
    pthread_mutex_lock(&mutex_lista_estructuras);
    t_tabla_pagina1 *tabla_pagina1 = buscar_tabla_pagina1(id_tabla);                                        // obtengo de la lista de estructuras, la tabla de primer nivel del proceso
    uint32_t nro_tabla_segundo_nivel = buscar_nro_tabla_segundo_nivel(tabla_pagina1, entrada_primer_nivel); // busco el numero de tabla de segundo nivel en la tabla de primer nivel
    pthread_mutex_unlock(&mutex_lista_estructuras);

    return nro_tabla_segundo_nivel;
}

t_list *buscar_tabla_segundo_nivel(uint32_t nro_tabla_2do_nivel)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    t_list *lista_tablas_2do_nivel = estructura_proceso_actual->lista_tablas_segundo_nivel;
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
    for (int i = 0; i < list_size(lista_tablas_2do_nivel); i++)
    {
        t_tabla_pagina2 *tabla_pagina2 = list_get(lista_tablas_2do_nivel, i);
        if (tabla_pagina2->id_tabla == nro_tabla_2do_nivel)
        {
            return tabla_pagina2->segundo_nivel;
        }
    }
    return NULL;
}

t_marco_presencia* obtener_frame(uint32_t nro_tabla_2do_nivel, uint32_t entrada_tabla_2do_nivel)
{
    t_estructura_2do_nivel *fila_2do_nivel;
    t_marco_presencia *marco_presencia = malloc(sizeof(t_marco_presencia));
    pthread_mutex_lock(&mutex_lista_estructuras);
    t_list *tabla_segundo_nivel = buscar_tabla_segundo_nivel(nro_tabla_2do_nivel);

    if (tabla_segundo_nivel != NULL)
    {
        fila_2do_nivel = list_get(tabla_segundo_nivel, entrada_tabla_2do_nivel);
        if (fila_2do_nivel == NULL)
        {
            // buscar en swap la info del frame
            // list_add de la estructura de 2do nivel en la tabla de segundo nivel (reemplazar si la tabla esta llena con los algoritmos CLOCK y CLOCK-M)
            // return el frame
        }
        else
        {
            // verificar el bit de presencia
            // si esta en 1, retorno el frame
            // si esta en 0, tengo que cargarlo en memoria trayendomelo de SWAP
            if (fila_2do_nivel->presencia == 1)
            {   
                marco_presencia->marco = fila_2do_nivel->marco;
                marco_presencia->presencia = 1; 
                /*
                devolver una estructura: 
                    1) marco
                    2) bit de presencia original (1)
                */
            }
            else
            {
                // cargarlo de swap
                // modificar el campo de frame (de -1 al que obtuve de SWAP) y el de presencia (de 0 a 1) en la fila de la tabla de 2do nivel
                // return el frame
                printf("Entre por el else y el bit de presencia es 0\n");
                marco_presencia->marco = 0; // ES PARA PROBAR AHORAA!!!
                marco_presencia->presencia = 0;
                /*
                devolver una estructura: 
                    1) marco
                    2) bit de presencia original (0)
                */
            }
        }
    }
    pthread_mutex_unlock(&mutex_lista_estructuras);
    return marco_presencia;
}

void escribir_valor(uint32_t frame, uint32_t desplazamiento, uint32_t valor_a_escribir)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    char *memoria_del_proceso = estructura_proceso_actual->espacio_en_memoria; // desde 0 bytes hasta el tamanio maximo de la memoria del proceso
    size_t frame_real = (configuracion_memoria->tam_memoria * frame);          // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que escribir
    // tamanio pagina = tamanio frame (en paginacion)
    memcpy(memoria_del_proceso + frame_real + desplazamiento, &valor_a_escribir, sizeof(valor_a_escribir)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
}

uint32_t leer_valor(uint32_t frame, uint32_t desplazamiento)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    uint32_t valor_leido;
    char *memoria_del_proceso = estructura_proceso_actual->espacio_en_memoria; // desde 0 bytes hasta el tamanio maximo de la memoria del proceso
    size_t frame_real = (configuracion_memoria->tam_memoria * frame);          // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    // tamanio pagina = tamanio frame (en paginacion)
    memcpy(&valor_leido, memoria_del_proceso + frame_real + desplazamiento, sizeof(valor_leido)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
    return valor_leido;
}

void copiar_valor(uint32_t frame_origen, uint32_t desplazamiento_origen, uint32_t frame_destino, uint32_t desplazamiento_destino)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    char *memoria_del_proceso = estructura_proceso_actual->espacio_en_memoria; // desde 0 bytes hasta el tamanio maximo de la memoria del proceso
    size_t frame_real_origen = (configuracion_memoria->tam_memoria * frame_origen);          // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    size_t frame_real_destino = (configuracion_memoria->tam_memoria * frame_destino);          // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    // tamanio pagina = tamanio frame (en paginacion)
    memcpy(memoria_del_proceso + frame_real_destino + desplazamiento_destino, memoria_del_proceso + frame_real_origen + desplazamiento_origen, sizeof(uint32_t)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
}