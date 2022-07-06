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
    uint32_t cant_paginas;
    uint32_t cant_tablas_segundo_nivel;

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
            loggear_info(logger, "DEBUG\n", mutex_logger_memoria);
            break;

        case ORDEN_ENVIO_TAMANIO:
            send_tamanio_y_cant_entradas(*cliente_socket, (configuracion_memoria->tam_pagina), (configuracion_memoria->entradas_por_tabla)); // envio el tamanio de pagina y cantidad de entradas de la tabla a la cpu
            loggear_info(logger, "Envie tamanio de pagina a CPU\n", mutex_logger_memoria);
            break;

        case INICIALIZAR_ESTRUCTURAS:
            recv_inicializar_estructuras(*cliente_socket, &tamanio_proceso, &id_proceso);
            loggear_info(logger, "INICIALIZANDO ESTRUCTURAS\n", mutex_logger_memoria);
            t_estructura_proceso *estructura = malloc(sizeof(t_estructura_proceso)); // creamos la estructura correspondiete al proceso
            estructura->id_proceso = id_proceso;
            estructura->espacio_en_memoria = malloc(tamanio_proceso);

            // averiguo cuantas paginas tiene mi proceso: tamanio en bytes del proceso / tamanio en bytes de una pagina
            cant_paginas = ceil(tamanio_proceso / (configuracion_memoria->tam_pagina));
            cant_tablas_segundo_nivel = ceil(cant_paginas / (configuracion_memoria->entradas_por_tabla)); // para saber cuantas tablas de 2do nivel crear

            // estructura de memoria = espacio + lista de tablas + archivo (espacio en swap)
            t_tabla_pagina1 *tabla_pagina1 = malloc(sizeof(t_tabla_pagina1) + sizeof(uint32_t) * (configuracion_memoria->entradas_por_tabla));
            tabla_pagina1->id_tabla = contador;
            tabla_pagina1->primer_nivel = list_create(); // entradas de la tabla de primer nivel (guarda los numeros de las tablas de segundo nivel)
            estructura->tabla_pagina1 = tabla_pagina1;

            estructura->lista_tablas_segundo_nivel = list_create(); // lista que guarda las tablas de segundo nivel
            // creo cada una de las tablas de segundo nivel que necesita
            for (int i = 0; i < cant_tablas_segundo_nivel; i++)
            {
                t_tabla_pagina2 *tabla_segundo_nivel = malloc(sizeof(t_tabla_pagina2) + sizeof(uint32_t) * (configuracion_memoria->entradas_por_tabla));
                int *id = malloc(sizeof(int));
                *id = i;
                tabla_segundo_nivel->id_tabla = i;                  // identificador de la tabla de segundo nivel
                tabla_segundo_nivel->segundo_nivel = list_create(); // lista de las entradas (filas) de la tabla de segundo nivel, se guarda en cada entrada un struct
                for (int j = 0; j < (configuracion_memoria->entradas_por_tabla); j++) 
                { // llenamos cada entrada de la tabla de 2do nivel
                    t_estructura_2do_nivel *entrada_segundo_nivel = malloc(sizeof(t_estructura_2do_nivel));
                    entrada_segundo_nivel->marco = -2;
                    entrada_segundo_nivel->uso = false;
                    entrada_segundo_nivel->modificado = false;
                    entrada_segundo_nivel->presencia = false;
                    list_add(tabla_segundo_nivel->segundo_nivel, entrada_segundo_nivel); //agrega las filas a la tabla de segundo nivel
                }
                list_add(tabla_pagina1->primer_nivel, id); // llenamos la tabla de primer nivel con el id de la tabla de segundo nivel
                list_add(estructura->lista_tablas_segundo_nivel, tabla_segundo_nivel); //guarda la tabla de 2do nivel en la lista de tablas de segundo nivel de la estrcutura del proceso
            }

            char *proceso_string = malloc(sizeof(id_proceso)+ strlen("/.swap")); //para agregarlo en la url del archivo de swap
            proceso_string = string_from_format("/%d.swap", id_proceso);
            char *path_archivo = malloc(strlen(configuracion_memoria->path_swap + strlen(proceso_string)));
            string_append(&path_archivo, configuracion_memoria->path_swap);
            string_append(&path_archivo, proceso_string);

            estructura->nombre_archivo_swap = malloc(strlen(path_archivo)); // "/home/utnso/id.swap"
            string_append(&estructura->nombre_archivo_swap, path_archivo);//agrego el path completo a la estructura

            if(!crear_archivo_swap(estructura,  tamanio_proceso, logger, mutex_logger_memoria)){ 
                break;
            } 
            
            loggear_info(logger, string_from_format("El nombre del archivo es: %s\n", estructura->nombre_archivo_swap), mutex_logger_memoria);
            list_add_con_mutex_tablas(lista_estructuras, estructura, mutex_lista_estructuras); // agrega la estructura a la lista de estructuras global donde estan las de todos los procesos

            send_valor_tb(*cliente_socket, tabla_pagina1->id_tabla); // le mandamos el id de la tabla que corresponde al proceso (es lo mismo que el contador)

            pthread_mutex_lock(&mutex_valor_tp);
            contador++;//para el id de la tabla de 1er nivel
            pthread_mutex_unlock(&mutex_valor_tp);

            free(cliente_socket);

            loggear_info(logger, "Se envio el valor de la tabla de paginas al kernel\n", mutex_logger_memoria);
            break;

        case PRIMER_ACCESO:
            recv_entrada_tabla_1er_nivel(*cliente_socket, &id_tabla1, &entrada_primer_nivel);
            loggear_info(logger, "Se recibio la entrada de la tabla de paginas de primer nivel\n", mutex_logger_memoria);
            num_tabla_segundo_nivel = obtener_tabla_2do_nivel(id_tabla1, entrada_primer_nivel); 
            send_num_tabla_2do_nivel(*cliente_socket, num_tabla_segundo_nivel);
            break;

        case SEGUNDO_ACCESO:
            recv_entrada_tabla_2do_nivel(*cliente_socket, &num_segundo_nivel, &entrada_tabla_2do_nivel);
            loggear_info(logger, "Se recibio la entrada de la tabla de pagina de segundo nivel\n", mutex_logger_memoria);
            // busco el frame en la tabla de segundo nivel
            marco_presencia = obtener_frame(num_segundo_nivel, entrada_tabla_2do_nivel); 
            send_frame(*cliente_socket, marco_presencia);
            break;

        case EJECUTAR_WRITE:
            recv_ejecutar_write(*cliente_socket, &frame, &desplazamiento, &valor_a_escribir, &id_proceso);
            loggear_warning(logger, "Se recibio orden de escritura\n", mutex_logger_memoria);
            buscar_estructura_del_proceso(id_proceso); // setea el proceso actual buscandolo en la lista de estructuras global
            escribir_valor(frame, desplazamiento, valor_a_escribir);
            send_ok(*cliente_socket);
            break;

        case EJECUTAR_READ:
            recv_ejecutar_read(*cliente_socket, &frame, &desplazamiento, &id_proceso);
            buscar_estructura_del_proceso(id_proceso);
            loggear_warning(logger, "Se recibio orden de lectura\n", mutex_logger_memoria);
            valor_leido = leer_valor(frame, desplazamiento);
            send_ok_read(*cliente_socket, valor_leido);
            break;

        case EJECUTAR_COPY:
            recv_ejecutar_copy(*cliente_socket, &frame_origen, &desplazamiento_origen, &frame_destino, &desplazamiento_destino, &id_proceso);
            buscar_estructura_del_proceso(id_proceso); //setea la estrctura actual 
            loggear_warning(logger, "Se recibio orden de copia\n", mutex_logger_memoria);
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
            loggear_warning(logger,string_from_format("Se suspendio el proceso %d\n", id_proceso), mutex_logger_memoria);
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

t_tabla_pagina1 *buscar_tabla_pagina1(uint32_t id_tabla) //busca en la lista de estructuras de todos los procesos la tabla de paginas de primer nivel corresponiente al proceso 
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
            tabla_pagina1 = estructura->tabla_pagina1;
            return tabla_pagina1;
        }
    }
    return NULL;
}

uint32_t buscar_nro_tabla_segundo_nivel(t_tabla_pagina1 *tabla_pagina1, uint32_t entrada_tabla_1er_nivel)
{   
    uint32_t i;
    uint32_t *retorno = malloc(sizeof(uint32_t));
    retorno = list_get(tabla_pagina1->primer_nivel, entrada_tabla_1er_nivel); // recorre por indice la tabla de primer nivel hasta llegar a la entrada que necesita y devuelve el contenido (nro de la tabla de segundo nivel)
    i = *retorno;                                                             // guarda el contenido en i
    free(retorno);
    return i;                                                           
}

uint32_t obtener_tabla_2do_nivel(uint32_t id_tabla, uint32_t entrada_primer_nivel) 
{   
    pthread_mutex_lock(&mutex_lista_estructuras);
    t_tabla_pagina1 *tabla_pagina1 = buscar_tabla_pagina1(id_tabla); // busca en la lista de estructuras de todos los procesos la tabla de paginas de primer nivel corresponiente al proceso                                       
    uint32_t nro_tabla_segundo_nivel = buscar_nro_tabla_segundo_nivel(tabla_pagina1, entrada_primer_nivel); // busco el numero de tabla de segundo nivel en la tabla de primer nivel (lo voy a encontrar porque conozco la entrada/fila a la que necesito ir)
    pthread_mutex_unlock(&mutex_lista_estructuras);

    return nro_tabla_segundo_nivel;
}

t_list *buscar_tabla_segundo_nivel(uint32_t nro_tabla_2do_nivel) //busca la tabla de segundo nivel en la lista de todas las tablas de segundo nivel del proceso actual
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    t_list *lista_tablas_2do_nivel = estructura_proceso_actual->lista_tablas_segundo_nivel;
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
    for (int i = 0; i < list_size(lista_tablas_2do_nivel); i++)
    {
        t_tabla_pagina2 *tabla_pagina2 = list_get(lista_tablas_2do_nivel, i);
        if (tabla_pagina2->id_tabla == nro_tabla_2do_nivel)//es el numero que viene del primer acceso
        {
            return tabla_pagina2->segundo_nivel;
        }
    }
    return NULL;
}

t_marco_presencia *obtener_frame(uint32_t nro_tabla_2do_nivel, uint32_t entrada_tabla_2do_nivel) //devuelve el marco con el bit de presencia para poder reiniciar la instruccion
{
    t_estructura_2do_nivel *fila_2do_nivel;
    t_marco_presencia *marco_presencia = malloc(sizeof(t_marco_presencia));
    pthread_mutex_lock(&mutex_lista_estructuras);
    t_list *tabla_segundo_nivel = buscar_tabla_segundo_nivel(nro_tabla_2do_nivel);

    if (tabla_segundo_nivel != NULL) //la tabla de segundo nivel siempre esta llena con marcos que si estan libres estan en -2
    {
        fila_2do_nivel = list_get(tabla_segundo_nivel, entrada_tabla_2do_nivel); //obtiene la fila que quiere de la tabla de segundo nivel
    
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
            else //fallo de pagina (bit de presencia = 0)
            {
                // ir a buscar la pagina en swap
                // modificar el campo de frame (de -2 al que obtuve de SWAP) y el de presencia (de 0 a 1) en la fila de la tabla de 2do nivel
                // return el frame
                printf("Entre por el else y el bit de presencia es 0\n");
                marco_presencia->marco = 0; // ES PARA PROBAR AHORAA!!! aca va el frame resultado del algoritmo de reemplazo
                marco_presencia->presencia = 0; 
                /*
                devolver una estructura:
                    1) marco
                    2) bit de presencia original (0)
                */
            }
        
    }
    pthread_mutex_unlock(&mutex_lista_estructuras);
    return marco_presencia;
}

void escribir_valor(uint32_t frame, uint32_t desplazamiento, uint32_t valor_a_escribir)
{   
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    void *memoria_del_proceso = estructura_proceso_actual->espacio_en_memoria; // desde 0 bytes hasta el tamanio maximo de la memoria del proceso
    size_t frame_real = (configuracion_memoria->tam_pagina * frame);           // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que escribir
    // tamanio pagina = tamanio frame (en paginacion)
    printf("Escribiendo en frame %d\n", frame);
    printf("frame real: %d\n", frame_real);
    printf("desplazamiento: %d\n", desplazamiento);
    printf("valor a escribir: %d\n", valor_a_escribir); 
    memcpy(memoria_del_proceso + frame_real + desplazamiento, &valor_a_escribir, sizeof(valor_a_escribir)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
}

uint32_t leer_valor(uint32_t frame, uint32_t desplazamiento)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    uint32_t valor_leido;
    void *memoria_del_proceso = estructura_proceso_actual->espacio_en_memoria; // desde 0 bytes hasta el tamanio maximo de la memoria del proceso
    size_t frame_real = (configuracion_memoria->tam_pagina * frame);           // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    // tamanio pagina = tamanio frame (en paginacion)
    memcpy(&valor_leido, memoria_del_proceso + frame_real + desplazamiento, sizeof(valor_leido)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
    return valor_leido;
}

void copiar_valor(uint32_t frame_origen, uint32_t desplazamiento_origen, uint32_t frame_destino, uint32_t desplazamiento_destino)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    void *memoria_del_proceso = estructura_proceso_actual->espacio_en_memoria;       // desde 0 bytes hasta el tamanio maximo de la memoria del proceso
    size_t frame_real_origen = (configuracion_memoria->tam_pagina * frame_origen);   // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    size_t frame_real_destino = (configuracion_memoria->tam_pagina * frame_destino); // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    // tamanio pagina = tamanio frame (en paginacion)
    memcpy(memoria_del_proceso + frame_real_destino + desplazamiento_destino, memoria_del_proceso + frame_real_origen + desplazamiento_origen, sizeof(uint32_t)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
}

void buscar_estructura_del_proceso(uint32_t pid) //setea la estrctura actual con el proceso que corresponde 
{
    t_estructura_proceso *procesoAux;
    pthread_mutex_lock(&mutex_lista_estructuras);
    for(int i =0; i < list_size(lista_estructuras); i++)
    {   
        
        procesoAux = list_get(lista_estructuras, i);
        if(procesoAux->id_proceso == pid)
        {   
            pthread_mutex_lock(&mutex_estructura_proceso_actual);
            estructura_proceso_actual = procesoAux;
            pthread_mutex_unlock(&mutex_estructura_proceso_actual);
            break;
        }
        
    }
    pthread_mutex_unlock(&mutex_lista_estructuras);
}