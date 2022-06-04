#include "../include/comunicacion.h"

uint32_t cantidad_procesos_en_memoria = 0; // variable global para asignar el ID a los procesos que llegan y para verificar el grado de multiprogramacion

uint32_t crear_comunicacion(t_configuracion_kernel *configuracion_kernel, t_log *logger) // inicio el servidor
{

    socket_kernel = iniciar_servidor(logger, "CONSOLA", configuracion_kernel->ip_kernel, configuracion_kernel->puerto_escucha); // levanto el servidor para el cliente: consola en este caso

    if (socket_kernel == -1)
    {
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        return -1;
    }

    return socket_kernel;
}

uint32_t crear_conexion_cpu_dispatch(t_configuracion_kernel *datos_conexion, t_log *logger) //"kernel" cliente de cpu (dispatch)
{
    socket_cpu_dispatch = crear_conexion_cliente(logger, "CPU DISPATCH", datos_conexion->ip_cpu, datos_conexion->puerto_cpu_dispatch);

    return socket_cpu_dispatch;
}

uint32_t crear_conexion_cpu_interrupt(t_configuracion_kernel *datos_conexion, t_log *logger) //"kernel" cliente de cpu
{
    socket_cpu_interrupt = crear_conexion_cliente(logger, "CPU INTERRUPT", datos_conexion->ip_cpu, datos_conexion->puerto_cpu_interrupt);

    return socket_cpu_interrupt;
}

uint32_t crear_conexion_memoria(t_configuracion_kernel *datos_conexion, t_log *logger) //"kernel" cliente de memoria
{
    socket_memoria = crear_conexion_cliente(logger, "MEMORIA", datos_conexion->ip_memoria, datos_conexion->puerto_memoria);

    return socket_memoria;
}

static void procesar_conexion(void *void_args)
{
    //tener solo un hilo por todas las conexiones, solo uno escucha al cliente. es un gran productor consumidor
    t_procesar_conexion_args *args = (t_procesar_conexion_args *)void_args; // recibo a mi cliente y sus datos
    t_log *logger = args->log;
    uint32_t cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

    op_code cop;
    uint32_t tamanio = 0;
    uint32_t tiempo_bloqueo;
    t_list *instrucciones = NULL;
    t_pcb *pcb;

    while (cliente_socket != -1) // mientras el cliente no se haya desconectado
    {
        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            pthread_mutex_lock(&mutex_logger_kernel);
            log_info(logger, "DISCONNECT!"); // desconectamos al cliente  xq no le esta mandando el cop bien
            pthread_mutex_unlock(&mutex_logger_kernel);
            return;
        }
        printf("Cliente %d conectado\n", cliente_socket);
        switch (cop)
        {
        case DEBUG: // para probar
            pthread_mutex_lock(&mutex_logger_kernel);
            log_info(logger, "debug");
            pthread_mutex_unlock(&mutex_logger_kernel);
            break;

        case INICIAR_PROCESO:

            if (recv_iniciar_consola(cliente_socket, &instrucciones, &tamanio)) // recibe las instrucciones y el tamanio del proceso
            {
                pthread_mutex_lock(&mutex_logger_kernel);
                log_info(logger, "Se recibieron las instrucciones");
                log_info(logger, "Tamanio de la consola: %d", tamanio);
                log_info(logger, "Cantidad de instrucciones: %d", list_size(instrucciones));
                pthread_mutex_unlock(&mutex_logger_kernel);

                socket_cpu_dispatch = crear_conexion_cpu_dispatch(configuracion_kernel, logger); //"kernel" cliente de cpu (dispatch) para pasarle el pcb
                socket_cpu_interrupt = crear_conexion_cpu_interrupt(configuracion_kernel, logger);
                socket_memoria = crear_conexion_memoria(configuracion_kernel, logger); // kernel cliente de memoria
                t_pcb *pcb = crear_pcb(instrucciones, logger, tamanio);

                verificacion_multiprogramacion(pcb); // planificador de largo plazo

                atencion_cpu(socket_cpu_dispatch, cliente_socket, logger);
                // liberar memoria
                free(pcb);
                list_destroy_and_destroy_elements(instrucciones, (void *)destruir_instruccion);
            }
            else
            {
                pthread_mutex_lock(&mutex_logger_kernel);
                log_error(logger, "No se recibieron las instrucciones");
                pthread_mutex_unlock(&mutex_logger_kernel);
            }
            break;

        // Errores
        case -1:
            pthread_mutex_lock(&mutex_logger_kernel);
            log_error(logger, "Cliente desconectado de %s...", server_name);
            pthread_mutex_unlock(&mutex_logger_kernel);
            return;
        default:
            pthread_mutex_lock(&mutex_logger_kernel);
            log_error(logger, "Algo anduvo mal en el server de %s", server_name);
            pthread_mutex_unlock(&mutex_logger_kernel);
            return;
        }
    }
    pthread_mutex_lock(&mutex_logger_kernel);
    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    pthread_mutex_unlock(&mutex_logger_kernel);
    return;
}

uint32_t server_escuchar(t_log *logger, char *server_name, uint32_t server_socket)
{
    uint32_t cliente_socket = esperar_cliente(logger, server_name, server_socket); // cuando se conecta un cliente nos devuelve la "linea" donde estan conectados
    pthread_t hilo;                                                                // crea un hilo para procesar la conexion

    if (cliente_socket != -1)
    {                                                                              // si se conecto un cliente
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args)); // crea una estructura para pasarle los argumentos al hilo
        args->log = logger;                                                        // guarda el logger en la estructura
        args->fd = cliente_socket;                                                 // guarda el socket del cliente en la estructura
        args->server_name = server_name;                                           // guarda el nombre del servidor en la estructura
        pthread_create(&hilo, NULL, (void *)procesar_conexion, (void *)args);      // crea el hilo
        pthread_detach(hilo);                                                      // lo desconecta del hilo                                        // libera el socket del cliente
        return 1;                                                                  // devuelve 1 para indicar que se conecto un cliente
    }
    // liberar_conexion(server_socket);
    return 0; // devuelve 0 si no se conecto nadie
}

t_pcb *crear_pcb(t_list *instrucciones, t_log *logger, uint32_t tamanio)
{
    t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->id = cantidad_procesos_en_memoria;
    pcb->instrucciones = instrucciones;
    pcb->program_counter = 0;
    pcb->tamanio = tamanio;
    pcb->tabla_pagina = 0;
    pcb->estimacion_rafaga = 0;
    return pcb;
}

void verificacion_multiprogramacion(t_pcb *pcb)
{
    op_code cop;
    pthread_mutex_lock(&mutex_cola_new);
    queue_push(cola_new, pcb); // agrego un proceso a la cola de nuevos
    pthread_mutex_unlock(&mutex_cola_new);
    if (cantidad_procesos_en_memoria <= configuracion_kernel->grado_multiprogramacion) // si el grado de multi lo permite
    {
        pthread_mutex_lock(&mutex_cola_new);
        t_pcb *consola_tope_lista = queue_pop(cola_new); // saco un proceso de la cola de nuevos
        pthread_mutex_unlock(&mutex_cola_new);
        pthread_mutex_lock(&mutex_cola_ready);
        queue_push(cola_ready, consola_tope_lista); // agregar el pcb a la cola de ready
        pthread_mutex_unlock(&mutex_cola_ready);
        pthread_mutex_lock(&mutex_cantidad_procesos);
        cantidad_procesos_en_memoria++; // aumenta el grado de multiprogramacion
        pthread_mutex_unlock(&mutex_cantidad_procesos);
        send_inicializar_estructuras(socket_memoria); // para que la memoria inicialice estructuras y obtenga el valor de la TP
        if (recv(socket_memoria, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            return;
        }
        recv_valor_tb(socket_memoria, &consola_tope_lista->tabla_pagina); // recibe el valor de la tp y lo guarda en el pcb

        if (strcmp(configuracion_kernel->algoritmo_planificacion, "SRT") == 0) // Sjf con desalojo
        {

            pthread_mutex_lock(&mutex_estado_running);
            if (running == NULL && queue_size(cola_ready) == 1) // el proceso es el unico en la cola de ready y nadie esta en running
            {
                pthread_mutex_unlock(&mutex_estado_running);
                pthread_mutex_lock(&mutex_estado_running);
                running = consola_tope_lista; // pongo en running al proceso que esta en ready
                pthread_mutex_unlock(&mutex_estado_running);
                send_pcb(socket_cpu_dispatch, running); // cuando pongo a ejecutar un proceso tengo que mandar a la CPU a través de la conexión de dispatch el PCB
                pthread_mutex_lock(&mutex_cola_ready);
                queue_pop(cola_ready); // saco al proceso de ready porq lo pongo a ejecutar
                pthread_mutex_unlock(&mutex_cola_ready);
            }
            else // HAY Q HACER OTRO IF.
            {
                // hay alguien ejecutando y llega alguien a ready
                pthread_mutex_unlock(&mutex_estado_running);
                t_pcb *pcb_desalojado;
                send_interrupcion_por_nuevo_ready(socket_cpu_interrupt);
                recv_pcb(socket_cpu_dispatch, pcb_desalojado);
                pthread_mutex_lock(&mutex_estado_running);
                running = NULL;
                pthread_mutex_unlock(&mutex_estado_running);
                pthread_mutex_lock(&mutex_cola_ready);
                queue_push(cola_ready, pcb_desalojado);
                pthread_mutex_unlock(&mutex_cola_ready);
                planificar();
            }
        }
        else // Fifo, el primero que entra
        {
            pthread_mutex_lock(&mutex_estado_running);
            pthread_mutex_lock(&mutex_cola_ready);
            if (running == NULL && queue_size(cola_ready) == 1)
            {
                pthread_mutex_unlock(&mutex_cola_ready);
                pthread_mutex_unlock(&mutex_estado_running);
                pthread_mutex_lock(&mutex_estado_running);
                running = consola_tope_lista;
                pthread_mutex_unlock(&mutex_estado_running);
                send_pcb(socket_cpu_dispatch, running); // si no hay nadie ejecutando y estas solo en ready
                pthread_mutex_lock(&mutex_cola_ready);
                queue_pop(cola_ready); // saco al proceso de ready porq lo pongo a ejecutar
                pthread_mutex_unlock(&mutex_cola_ready);
                pthread_mutex_lock(&mutex_estado_running);
                pthread_mutex_lock(&mutex_cola_ready);
            }
            pthread_mutex_unlock(&mutex_cola_ready);
            pthread_mutex_unlock(&mutex_estado_running);
        }
    }
    return;
}

void atencion_cpu(uint32_t socket_cpu_dispatch, uint32_t cliente_socket, t_log *logger)
{
    op_code cop2;
    op_code cop;
    t_pcb *pcb;
    t_pcb *pcbExit;
    uint32_t tiempo;
    printf("NOC NOC\n");
    while (true) // va a salir cuando no hayan mas procesos en la cola
    {

        if (recv(socket_cpu_dispatch, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            pthread_mutex_lock(&mutex_logger_kernel);
            log_info(logger, "DISCoONNECT!"); // desconectamos al cpu  xq no le esta mandando el cop bien
            pthread_mutex_unlock(&mutex_logger_kernel);
            return;
        }
        printf("Recibi el cop %d\n", cop);
        switch (cop)
        {
        case BLOQUEO_IO:

        //crear funcion loggear con mutex que haga el lock y el unlock y pasar por parametro el mensaje (monitor)
        //
            recv_pcb_con_tiempo_bloqueado(socket_cpu_dispatch, &pcb, &tiempo);

            pthread_mutex_lock(&mutex_logger_kernel);
            log_info(logger, "Se bloqueo el proceso %d por IO en el hilo %d", pcb->id, pthread_self());
            log_info(logger, "Tiempo bloqueado recibido: %d\n", tiempo);
            pthread_mutex_unlock(&mutex_logger_kernel);

            pthread_mutex_lock(&mutex_estado_running);
            running = NULL;
            pthread_mutex_unlock(&mutex_estado_running);

            pthread_mutex_lock(&mutex_cola_ready);
            if (queue_size(cola_ready) > 0)
            {
                pthread_mutex_unlock(&mutex_cola_ready);
                sem_post(&sem_planificar);
                pthread_mutex_lock(&mutex_cola_ready);
            }
            pthread_mutex_unlock(&mutex_cola_ready);

            pthread_mutex_lock(&mutex_lista_blocked);
            list_add(bloqueados, pcb); // lista para los procesos bloqueados y poder sacar cualquier proceso de la lista
            pthread_mutex_unlock(&mutex_lista_blocked);

            usleep(tiempo * 1000); // usleep tabaja en microsegundos(milisegundos*1000) -> pongo a dormir el hilo xq esta haciendo E/S
                                   // planificar();

            pthread_mutex_lock(&mutex_lista_blocked);
            int indice = buscar_indice_pcb_en_lista_bloqueados(bloqueados, pcb);
            list_remove(bloqueados, indice); // saca el pcb de la lista de bloqueados
            pthread_mutex_lock(&mutex_logger_kernel);
            log_info(logger, "Proceso %d desbloqueado en el hilo %d", pcb->id, pthread_self());
            pthread_mutex_unlock(&mutex_logger_kernel);
            pthread_mutex_unlock(&mutex_lista_blocked);

            pthread_mutex_lock(&mutex_cola_ready);
            queue_push(cola_ready, pcb); // Cuando se desbloquea, vuelve a ready

            sem_post(&sem_planificar);
            pthread_mutex_unlock(&mutex_cola_ready);
            break;

        case ENVIAR_PCB:
            recv_pcb(socket_cpu_dispatch, &pcbExit);
            pthread_mutex_lock(&mutex_logger_kernel);
            log_info(logger, "Orden EXIT recibida, id: %d\n", pcbExit->id);
            pthread_mutex_unlock(&mutex_logger_kernel);
            pthread_mutex_lock(&mutex_cola_exit);
            queue_push(cola_exit, pcbExit);
            pthread_mutex_unlock(&mutex_cola_exit);
            send_fin_proceso(socket_memoria); // le aviso a la memoria que el proceso termino para que libere las estructuras

            if (recv(socket_memoria, &cop2, sizeof(op_code), 0) != sizeof(op_code))
            { // recibo el op code LIBERAR ESTRUCTURAS(es el unico cop q puede recibir de memoria, por eso no hago un switch)
                log_error(logger, "Error al recibir el op code LIBERAR ESTRUCTURAS");
                return;
            }
            printf("El cliente es %d  \n", cliente_socket);
            if (send(cliente_socket, &cop2, sizeof(op_code), 0) != sizeof(op_code)) // avisa a la consola que el proceso termino
            {
                pthread_mutex_lock(&mutex_logger_kernel);
                log_error(logger, "Error al avisarle a la consola que finalizo");
                
                pthread_mutex_unlock(&mutex_logger_kernel);
                return;
            }
            else
            {
                pthread_mutex_lock(&mutex_logger_kernel);
                // loggear el id del hilo
                log_info(logger, "hilo: %d\n", pthread_self());
                log_info(logger, "le mande a la consola que termino!!!!!!!\n");
                pthread_mutex_unlock(&mutex_logger_kernel);
                
            }

            pthread_mutex_lock(&mutex_cantidad_procesos);
            cantidad_procesos_en_memoria--; // baja el nivel de multiprogramacion
            pthread_mutex_unlock(&mutex_cantidad_procesos);

            pthread_mutex_lock(&mutex_estado_running);
            running = NULL; // estado de running en null porque el proceso termino de ejecutar
            pthread_mutex_unlock(&mutex_estado_running);
   
            pthread_mutex_lock(&mutex_cola_ready);
            if (queue_size(cola_ready) > 0)
            {
                pthread_mutex_unlock(&mutex_cola_ready);
                pthread_mutex_lock(&mutex_logger_kernel);
                pthread_mutex_lock(&mutex_cola_ready);
                log_info(logger, "En la cola quedan %d procesos \n", queue_size(cola_ready));
                pthread_mutex_unlock(&mutex_cola_ready);
                pthread_mutex_unlock(&mutex_logger_kernel);
                sem_post(&sem_planificar); // post = signal
                pthread_mutex_lock(&mutex_logger_kernel);
                log_info(logger,"Estoy falleciendo y soy el hilo: %d\n", pthread_self());
                pthread_mutex_unlock(&mutex_logger_kernel);
                // cuando un proceso termina de ejecutar(exit) planificamos
            }
            else
            {
                pthread_mutex_unlock(&mutex_logger_kernel);
                printf("No quedan mas procesos en la cola \n");

                pthread_mutex_lock(&mutex_logger_kernel);
                log_info(logger,"Estoy falleciendo y soy el hiloo: %d\n", pthread_self());
                pthread_mutex_unlock(&mutex_logger_kernel);
                // sem_post(&finalizar_planificar);
                return;
            }
            
            break;
        }
        
    }
}

void planificar()
{
    ALGORITMO algoritmo = algortimo_de_planificacion(configuracion_kernel->algoritmo_planificacion);
    t_pcb *pcb_tope_lista;
    switch (algoritmo) // segun el algortimo planifica
    {
    case FIFO:
        while (true)
        {
            sem_wait(&sem_planificar);
            pcb_tope_lista = malloc(sizeof(t_pcb));

            pthread_mutex_lock(&mutex_estado_running);
            if (running == NULL) // no hay alguien ejecutando
            {
                pthread_mutex_unlock(&mutex_estado_running);

                pthread_mutex_lock(&mutex_cola_ready);
                pcb_tope_lista = queue_pop(cola_ready); // si no hay nadie ejecutando, saco al primero de la cola de ready
                pthread_mutex_unlock(&mutex_cola_ready);

                pthread_mutex_lock(&mutex_estado_running);
                running = pcb_tope_lista; // pone al proceso a ejecutar
                pthread_mutex_unlock(&mutex_estado_running);

                send_pcb(socket_cpu_dispatch, pcb_tope_lista); // se lo mando a la cpu para que lo ponga a ejecutar. 

                pthread_mutex_lock(&mutex_logger_kernel);
                log_info(logger, "Proceso %d en ejecucion, (lo puso a ejecutar el hilo: %d)", pcb_tope_lista->id, pthread_self());
                pthread_mutex_unlock(&mutex_logger_kernel);

                pthread_mutex_lock(&mutex_estado_running);
            }
            pthread_mutex_unlock(&mutex_estado_running);


            // list_destroy_and_destroy_elements(pcb_tope_lista->instrucciones, (void *)destruir_instruccion);
            //  free(pcb_tope_lista); // HAY QUE VER QUE NO TIRE SEG FAULT
        }
        break;

    case SJF: // con desalojo
        // sem_wait(&planificar) || sem_wait(&nuevoAReady);
        //guardar en timer.start en blblio timer cuando se y cuando vuelve comparar el tiempo
        
        break;
    }
    return;
}

ALGORITMO algortimo_de_planificacion(char *algortimo_de_planificacion)
{
    if (strcmp(algortimo_de_planificacion, "FIFO") == 0)
    {
        return FIFO;
    }
    else if (strcmp(algortimo_de_planificacion, "SJF") == 0)
    {
        return SJF;
    }
    return ERROR;
}