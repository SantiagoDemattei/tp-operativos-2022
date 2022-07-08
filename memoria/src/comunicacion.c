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
    uint32_t nro_pagina;

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
            // asigno el espacio de la memoria que va a ser del proceso
            estructura->marco_comienzo = buscar_marcos_para_asignar(); //primer frame de la memoria que le puedo asignar (donde arranca el proceso)
            if (estructura->marco_comienzo == -1) 
            {
                send_valor_tb(*cliente_socket, -1); // send para kernel de que no se puede crear las estructuras para el proceso dado que no hay marcos libres
                loggear_error(logger, "No hay marcos libres\n", mutex_logger_memoria);
                break;
            }
            estructura->puntero_clock=0; // puntero de clock para el proceso
            estructura->marco_fin = (estructura->marco_comienzo) + (configuracion_memoria->marcos_por_proceso) - 1;
            llenar_marcos_para_el_proceso(estructura->marco_comienzo, estructura->marco_fin, 1); //cambia a 1 los marcos ocupados (de la memoria) que le asigno al proceso

            estructura->vector_marcos = list_create(); // marcos propios del proceso
            llenar_marcos_para_el_proceso_local(estructura->vector_marcos, configuracion_memoria->marcos_por_proceso, 0);  // lleno la lista de marcos propios del proceso (estado en 0 porque estan todos libres y num de pagona en -1 porque no tienen paginas los marcos)

            // averiguo cuantas paginas tiene mi proceso: tamanio en bytes del proceso / tamanio en bytes de una pagina
            cant_paginas = ceil(tamanio_proceso / (configuracion_memoria->tam_pagina));//la funcion ceil redondea para arriba
            cant_tablas_segundo_nivel = ceil(cant_paginas / (configuracion_memoria->entradas_por_tabla)); // para saber cuantas tablas de 2do nivel crear

            t_tabla_pagina1 *tabla_pagina1 = malloc(sizeof(t_tabla_pagina1) + sizeof(uint32_t) * (configuracion_memoria->entradas_por_tabla));
            tabla_pagina1->id_tabla = contador;
            tabla_pagina1->primer_nivel = list_create(); // filas de la tabla de primer nivel (guarda los numeros de las tablas de segundo nivel)
            estructura->tabla_pagina1 = tabla_pagina1;

            estructura->lista_tablas_segundo_nivel = list_create(); // lista que guarda las tablas de segundo nivel (cada nodo es una tabla)
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
                    list_add(tabla_segundo_nivel->segundo_nivel, entrada_segundo_nivel); // agrega las filas a la tabla de segundo nivel
                }
                list_add(tabla_pagina1->primer_nivel, id);                             // llenamos la tabla de primer nivel con el id de la tabla de segundo nivel
                list_add(estructura->lista_tablas_segundo_nivel, tabla_segundo_nivel); // guarda la tabla de 2do nivel en la lista de tablas de segundo nivel de la estrcutura del proceso
            }

            char *proceso_string = malloc(sizeof(id_proceso) + strlen("/.swap")); // para agregarlo en la url del archivo de swap
            proceso_string = string_from_format("/%d.swap", id_proceso);
            char *path_archivo = malloc(strlen(configuracion_memoria->path_swap + strlen(proceso_string)));
            string_append(&path_archivo, configuracion_memoria->path_swap);
            string_append(&path_archivo, proceso_string);

            estructura->nombre_archivo_swap = malloc(strlen(path_archivo)); // "/home/utnso/id.swap"
            string_append(&estructura->nombre_archivo_swap, path_archivo);  // agrego el path completo a la estructura

            if (!crear_archivo_swap(estructura, tamanio_proceso, logger, mutex_logger_memoria))
            {
                loggear_error(logger, "No se pudo crear el archivo de swap\n", mutex_logger_memoria);
                break;
            }

            loggear_info(logger, string_from_format("El nombre del archivo es: %s\n", estructura->nombre_archivo_swap), mutex_logger_memoria);
            list_add_con_mutex_tablas(lista_estructuras, estructura, mutex_lista_estructuras); // agrega la estructura a la lista de estructuras global donde estan las de todos los procesos

            send_valor_tb(*cliente_socket, tabla_pagina1->id_tabla); // le mandamos el id de la tabla que corresponde al proceso (es lo mismo que el contador)

            pthread_mutex_lock(&mutex_valor_tp);
            contador++; // para el id de la tabla de 1er nivel
            pthread_mutex_unlock(&mutex_valor_tp);

            free(cliente_socket);

            loggear_info(logger, "Se envio el valor de la tabla de paginas al kernel\n", mutex_logger_memoria);
            break;

        case PRIMER_ACCESO:
            recv_entrada_tabla_1er_nivel(*cliente_socket, &id_tabla1, &entrada_primer_nivel);
            loggear_info(logger, "Se recibio la entrada de la tabla de paginas de primer nivel\n", mutex_logger_memoria);
            num_tabla_segundo_nivel = obtener_tabla_2do_nivel(id_tabla1, entrada_primer_nivel);//busco la tabla de segundo nivel
            send_num_tabla_2do_nivel(*cliente_socket, num_tabla_segundo_nivel);
            break;

        case SEGUNDO_ACCESO:
            recv_entrada_tabla_2do_nivel(*cliente_socket, &num_segundo_nivel, &entrada_tabla_2do_nivel, &nro_pagina);
            loggear_info(logger, "Se recibio la entrada de la tabla de pagina de segundo nivel\n", mutex_logger_memoria);
            // busco el frame en la tabla de segundo nivel
            marco_presencia = obtener_frame(num_segundo_nivel, entrada_tabla_2do_nivel, nro_pagina);
            printf("\nle mando a la cpu el marco: %d\n\n", marco_presencia->marco);
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
            buscar_estructura_del_proceso(id_proceso); // setea la estrctura actual
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
            loggear_warning(logger, string_from_format("Se suspendio el proceso %d\n", id_proceso), mutex_logger_memoria);
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

t_tabla_pagina1 *buscar_tabla_pagina1(uint32_t id_tabla) // busca en la lista de estructuras de todos los procesos la tabla de paginas de primer nivel corresponiente al proceso
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
    uint32_t *retorno = list_get(tabla_pagina1->primer_nivel, entrada_tabla_1er_nivel); // recorre por indice la tabla de primer nivel hasta llegar a la entrada que necesita y devuelve el contenido (nro de la tabla de segundo nivel)
    return *retorno;
}

uint32_t obtener_tabla_2do_nivel(uint32_t id_tabla, uint32_t entrada_primer_nivel)
{
    pthread_mutex_lock(&mutex_lista_estructuras);
    t_tabla_pagina1 *tabla_pagina1 = buscar_tabla_pagina1(id_tabla);                                        // busca en la lista de estructuras de todos los procesos la tabla de paginas de primer nivel corresponiente al proceso
    uint32_t nro_tabla_segundo_nivel = buscar_nro_tabla_segundo_nivel(tabla_pagina1, entrada_primer_nivel); // busco el numero de tabla de segundo nivel en la tabla de primer nivel (lo voy a encontrar porque conozco la entrada/fila a la que necesito ir)
    pthread_mutex_unlock(&mutex_lista_estructuras);
    return nro_tabla_segundo_nivel;
}

t_list *buscar_tabla_segundo_nivel(uint32_t nro_tabla_2do_nivel) // busca la tabla de segundo nivel en la lista de todas las tablas de segundo nivel del proceso actual
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    t_list *lista_tablas_2do_nivel = estructura_proceso_actual->lista_tablas_segundo_nivel;
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
    for (int i = 0; i < list_size(lista_tablas_2do_nivel); i++)
    {
        t_tabla_pagina2 *tabla_pagina2 = list_get(lista_tablas_2do_nivel, i);
        if (tabla_pagina2->id_tabla == nro_tabla_2do_nivel) // es el numero que viene del primer acceso
        {
            return tabla_pagina2->segundo_nivel;
        }
    }
    return NULL;
}

t_marco_presencia *obtener_frame(uint32_t nro_tabla_2do_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t nro_pagina) // devuelve el marco con el bit de presencia para poder reiniciar la instruccion si fuera necesario
{
    t_estructura_2do_nivel *fila_2do_nivel;
    t_marco_presencia *marco_presencia = malloc(sizeof(t_marco_presencia));
    pthread_mutex_lock(&mutex_lista_estructuras);;
    t_list *tabla_segundo_nivel = buscar_tabla_segundo_nivel(nro_tabla_2do_nivel);
    fila_2do_nivel = list_get(tabla_segundo_nivel, entrada_tabla_2do_nivel); // obtiene la fila que quiere de la tabla de segundo nivel
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
    else // fallo de pagina (bit de presencia = 0) -> hay que cargar la pagina en memoria (porque esta en swap)
    {
        // ir a buscar la pagina en swap
        void *contenido_pagina = buscar_contenido_pagina_en_swap(estructura_proceso_actual->archivo_swap, nro_pagina, configuracion_memoria->tam_pagina); // busca el contenido de la pagina en el archivo de swap (porque no esta en la memoria)
        uint32_t marco_asignado = buscar_marco_libre(nro_pagina, contenido_pagina);                                                                       // obtengo un frame libre y le cargo el contenido

        fila_2do_nivel->marco = marco_asignado;                                                                                                           // cambio el marco de la fila de la tabla de paginas al marco en el que cargue la pagina
        fila_2do_nivel->presencia = 1;                                                                                                                    // cambio el bit de presencia a 1 en la fila de la tabla de paginas
        fila_2do_nivel->uso = 1;
        fila_2do_nivel->modificado = 0;


        // ESTO ES LO QUE LE DEVUELVO A LA CPU
        marco_presencia->marco = fila_2do_nivel->marco;
        marco_presencia->presencia = 0; // no significa q en la tabla de paginas sea 0
        /*
        devolver una estructura:
            1) marco
            2) bit de presencia original (0)
        */
    }

    pthread_mutex_unlock(&mutex_lista_estructuras);
    return marco_presencia;
}

uint32_t buscar_marco_libre(uint32_t nro_pagina, void *contenido_pagina)
{ // busca un marco libre en la tabla de paginas y lo carga con el contenido de la pagina){
    uint32_t marco_asignado;
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    t_vector_marcos *elemento;
    marco_asignado = buscar_marcos_para_asignar_local(estructura_proceso_actual->vector_marcos); // busco un marco libre en el vector de marcos del proceso actual+
    if (marco_asignado != -1)
    {                                                                                // Hay un marco libre y lo devuelvo
        elemento = list_get(estructura_proceso_actual->vector_marcos, marco_asignado); // busco el elemento a editar
        elemento->estado = 1;                                                          // pongo el marco ocupado en el vector del proceso
        elemento->nro_pagina = nro_pagina;                                             // pongo el numero de pagina en el vector del proceso
        escribir_contenido_pagina_en_marco(estructura_proceso_actual->marco_comienzo, contenido_pagina, marco_asignado, configuracion_memoria->tam_pagina);
        pthread_mutex_unlock(&mutex_estructura_proceso_actual);
        return marco_asignado; // retorno el marco
    }
    // ELSE
    ALGORITMO_MEMORIA algoritmo_memoria = algoritmo_reemplazo();

    t_list *paginas_cargadas = buscar_paginas_con_presencia_en_1(estructura_proceso_actual->lista_tablas_segundo_nivel); // busco las paginas con presencia en 1 en la tabla de paginas del proceso actual
    printf("\nPaginas cargadas: %d\n", list_size(paginas_cargadas));
    switch (algoritmo_memoria)
    {
    case CLOCK:
        printf("entre al algoritmo de reemplazo CLOCK\n");
        printf("marco asignado: %d\n", marco_asignado);
        for (int i = estructura_proceso_actual->puntero_clock; i < list_size(paginas_cargadas); i++)
        {                                                                 // recorro todas las paginas con presencia en 1
            t_estructura_2do_nivel *fila = list_get(paginas_cargadas, i); // obtengo una fila
          //TODO: QUE PASA SI LA PAGINA SON IGUALES
            if (fila->uso == 0)                                           // si la fila que obtuve tiene U = 0
            {
                if (fila->modificado == 0)
                {                                                                                                                                                       // como M = 0, puedo descartar la pagina que estaba en el marco
                    marco_asignado = fila->marco;                                                                                                                       // el marco que le asigno a la nueva pagina, es el mismo que tenia la pagina que descarte
                    escribir_contenido_pagina_en_marco(estructura_proceso_actual->marco_comienzo, contenido_pagina, marco_asignado, configuracion_memoria->tam_pagina); // escribo el contenido de la nueva pagina en el marco asignado
                    loggear_info(logger, string_from_format("Reemplazando pagina %d con bit de uso en 0 y modificado en 0",elemento->nro_pagina),mutex_logger_memoria);
                    elemento = list_get(estructura_proceso_actual->vector_marcos, marco_asignado);
                    fila->presencia = 0;                                                                                                                                // como descarte una pagina, edito su entrada de la tabla de paginas poniendole el bit de presencia en 0 (ya que no esta mas presente en memoria)
                    elemento->nro_pagina = nro_pagina;                                                                                                                  // le actualizo la pagina
                    elemento->estado = 1;                                                                                                                               // el estado permanece en 1 xq saque una pagina e inmediatamente meti otra
                    estructura_proceso_actual->puntero_clock = i + 1;
                    return marco_asignado;
                }
                else
                {   
                    elemento = list_get(estructura_proceso_actual->vector_marcos, fila->marco);
                    void *contenido_pagina_que_esta_cargada = buscar_contenido_pagina_en_memoria(estructura_proceso_actual->marco_comienzo, fila->marco, configuracion_memoria->tam_pagina); // busco el contenido de la pagina actual (la que esta modificada) en la memoria
                    escribir_contenido_pagina_en_swap(estructura_proceso_actual->archivo_swap, contenido_pagina_que_esta_cargada, elemento->nro_pagina, configuracion_memoria->tam_pagina);  // escribo el contenido de la pagina actual en el archivo swap
                    free(contenido_pagina_que_esta_cargada);
                    escribir_contenido_pagina_en_marco(estructura_proceso_actual->marco_comienzo, contenido_pagina, fila->marco, configuracion_memoria->tam_pagina);  // escribo el contenido de la pagina que tengo que cargar en el marco
                    loggear_info(logger, string_from_format("Reemplazando pagina %d con bit de uso en 0 y modificado en 1",elemento->nro_pagina),mutex_logger_memoria);
                    fila->presencia = 0;
                    elemento->nro_pagina = nro_pagina;
                    elemento->estado = 1;
                    estructura_proceso_actual->puntero_clock = i + 1;
                    marco_asignado = fila->marco;
                    return marco_asignado;
                }
            }
            else
            { // si la fila que obtuve tiene U = 1
                fila->uso = false;
                loggear_info(logger, string_from_format("Pagina %d con bit de uso en 1, ahora 0 ",elemento->nro_pagina),mutex_logger_memoria);
                if (list_size(paginas_cargadas) - 1 == i)
                { // si ya revise hasta el tope, pongo desde el principio de la lista
                    i = 0; 
                }
            }
        }

        break;

    case CLOCK_M:
        break;

    default:
        loggear_error(logger, "Error en algoritmo de reemplazo", mutex_logger_memoria);
        break;
    }
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
    return marco_asignado;
}

t_list *buscar_paginas_con_presencia_en_1(t_list *lista_tablas_segundo_nivel)
{ // recorro cada fila de cada tabla de paginas de segundo nivel y devuelvo una lista con las paginas con presencia en 1
    t_list *paginas_cargadas = list_create();
    printf("List size: %d\n", list_size(lista_tablas_segundo_nivel));
    for (int i = 0; i < list_size(lista_tablas_segundo_nivel); i++)
    {
        t_list *tabla_segundo_nivel = list_get(lista_tablas_segundo_nivel, i);
        printf("list size tabla segundo nivel: %d\n", list_size(tabla_segundo_nivel));//ACA TIRA CUALQUIER COSA
        for (int j = 0; j < list_size(tabla_segundo_nivel); j++)
        { // recorro las filas de cada tabla
            t_estructura_2do_nivel *fila = list_get(tabla_segundo_nivel, j);
            printf("el marco es: %d\n", fila->marco);
            if (fila->presencia == 1)
            {
                list_add(paginas_cargadas, fila);
            }
        }
    }

    bool comparar_paginas_cargadas(void *ex1, void *ex2)
    {
        t_estructura_2do_nivel *pagina_1 = ex1;
        t_estructura_2do_nivel *pagina_2 = ex2;
        return pagina_1->marco < pagina_2->marco;
    }

    // ordenar lista de paginas_cargadas por numero de marco ascendente (de menor a mayor)
    list_sort(paginas_cargadas, &comparar_paginas_cargadas); // VER SI COMPILA
    return paginas_cargadas;
}

uint32_t buscar_marcos_para_asignar()
{
    pthread_mutex_lock(&mutex_marcos);
    for (int i = 0; i < list_size(marcos_totales); i++)
    {
        uint32_t *elemento = list_get(marcos_totales, i); // busca desde el primero y se fija si es 0. Si es 0, quiere decir q hay marcos libres.
        if (*elemento == 0)
        {
            pthread_mutex_unlock(&mutex_marcos);
            return i;//se guarda el indice de la lista donde esta ese marco libre
        }
    }
    pthread_mutex_unlock(&mutex_marcos);
    return -1;
}

uint32_t buscar_marcos_para_asignar_local(t_list *lista)
{   
    for (int i = 0; i < list_size(lista); i++)
    {
        t_vector_marcos *elemento = list_get(lista, i);
        if (elemento->estado == 0)
        {
            return i; // si hay alguno libre devuelve el numero de marco
        }
    }
    return -1; // si estan todos llenos
}

void escribir_valor(uint32_t frame, uint32_t desplazamiento, uint32_t valor_a_escribir)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    pthread_mutex_lock(&mutex_espacio_memoria);
    size_t frame_real = (configuracion_memoria->tam_pagina * frame); // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que escribir
    // tamanio pagina = tamanio frame (en paginacion)
    uint32_t inicio = estructura_proceso_actual->marco_comienzo;
    size_t comienzo_real = configuracion_memoria->tam_pagina * inicio; // donde verdaderamente comienza el proceso
    printf("Escribiendo en frame %d\n", frame);
    printf("frame real: %d\n", frame_real);
    printf("desplazamiento: %d\n", desplazamiento);
    printf("valor a escribir: %d\n", valor_a_escribir);
    memcpy(espacio_memoria + comienzo_real + frame_real + desplazamiento, &valor_a_escribir, sizeof(valor_a_escribir)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_unlock(&mutex_espacio_memoria);
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
}

uint32_t leer_valor(uint32_t frame, uint32_t desplazamiento)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    pthread_mutex_lock(&mutex_espacio_memoria);
    uint32_t valor_leido;
    uint32_t inicio = estructura_proceso_actual->marco_comienzo; // marco donde comienza el proceso
    size_t comienzo_real = configuracion_memoria->tam_pagina * inicio;
    size_t frame_real = (configuracion_memoria->tam_pagina * frame); // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    // tamanio pagina = tamanio frame (en paginacion)
    memcpy(&valor_leido, espacio_memoria + comienzo_real + frame_real + desplazamiento, sizeof(valor_leido)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_unlock(&mutex_espacio_memoria);
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
    return valor_leido;
}

void copiar_valor(uint32_t frame_origen, uint32_t desplazamiento_origen, uint32_t frame_destino, uint32_t desplazamiento_destino)
{
    pthread_mutex_lock(&mutex_estructura_proceso_actual);
    pthread_mutex_lock(&mutex_espacio_memoria);
    uint32_t inicio = estructura_proceso_actual->marco_comienzo;
    size_t comienzo_real = configuracion_memoria->tam_pagina * inicio;               // donde verdaderamente comienza el proceso
    size_t frame_real_origen = (configuracion_memoria->tam_pagina * frame_origen);   // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    size_t frame_real_destino = (configuracion_memoria->tam_pagina * frame_destino); // lo que hago aca es calcular en donde esta ubicado el frame donde tengo que leer
    // tamanio pagina = tamanio frame (en paginacion)
    memcpy(espacio_memoria + comienzo_real + frame_real_destino + desplazamiento_destino, espacio_memoria + comienzo_real + frame_real_origen + desplazamiento_origen, sizeof(uint32_t)); // comienzo de la memoria + todos los bytes que hay hasta el frame elegido + el desplazamiento sobre el frame
    pthread_mutex_unlock(&mutex_espacio_memoria);
    pthread_mutex_unlock(&mutex_estructura_proceso_actual);
}

void buscar_estructura_del_proceso(uint32_t pid) // setea la estrctura actual con el proceso que corresponde
{
    t_estructura_proceso *proceso_aux;
    pthread_mutex_lock(&mutex_lista_estructuras);
    for (int i = 0; i < list_size(lista_estructuras); i++)
    {
        proceso_aux = list_get(lista_estructuras, i);
        if (proceso_aux->id_proceso == pid)
        {
            pthread_mutex_lock(&mutex_estructura_proceso_actual);
            estructura_proceso_actual = proceso_aux;
            pthread_mutex_unlock(&mutex_estructura_proceso_actual);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_estructuras);
}

void llenar_marcos_para_el_proceso(uint32_t inicio, uint32_t fin, uint32_t contenido)
{
    pthread_mutex_lock(&mutex_marcos);
    for (int i = inicio; i < fin; i++)
    {
        // modifico 0 por 1 los marcos de memoria que ahora le pertenecen al proceso
        uint32_t *elemento = list_get(marcos_totales, i);
        *elemento = contenido;
    }
    pthread_mutex_unlock(&mutex_marcos);
}

void llenar_marcos_para_el_proceso_local(t_list *lista_marcos_del_proceso, uint32_t cant_marcos , uint32_t estado)
{
    for (int i = 0; i < cant_marcos; i++)
    {
        t_vector_marcos *elemento = malloc(sizeof(t_vector_marcos));
        elemento->estado = estado; //estado 0 = libre, estado 1 = ocupado
        elemento->nro_pagina = -1; //en -1 los marcos que no tienen nada 
        list_add(lista_marcos_del_proceso, elemento);
    }
}

ALGORITMO_MEMORIA algoritmo_reemplazo()
{

    if (strcmp(configuracion_memoria->algoritmo_reemplazo, "CLOCK") == 0)
    {
        return CLOCK;
    }
    else if (strcmp(configuracion_memoria->algoritmo_reemplazo, "CLOCK-M"))
    {
        return CLOCK_M;
    }
    return -1;
}
