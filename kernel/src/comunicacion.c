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
    // tener solo un hilo por todas las conexiones, solo uno escucha al cliente. es un gran productor consumidor
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
            loggear_info(logger, "DISCONNECT", mutex_logger_kernel);
            return;
        }

        switch (cop)
        {
        case DEBUG: // para probar
            loggear_info(logger, "DEBUG", mutex_logger_kernel);
            break;

        case INICIAR_PROCESO:
            if (recv_iniciar_consola(cliente_socket, &instrucciones, &tamanio))
            {
                loggear_info(logger, "Se recibieron las instrucciones", mutex_logger_kernel);
                loggear_lista_instrucciones(instrucciones, logger);

                t_pcb *pcb = crear_pcb(instrucciones, logger, tamanio);
                verificacion_multiprogramacion(pcb);
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
        procesar_conexion(args);                                                   
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
    pcb->tiempo_bloqueo = 0;
    return pcb;
}

void verificacion_multiprogramacion(t_pcb *pcb)
{
    op_code cop;
    queue_push_con_mutex(cola_new, pcb, mutex_cola_new);
    if (consulta_grado())
    {
        t_pcb *tope_cola_new = queue_pop_con_mutex(cola_new, mutex_cola_new); // ver si no genera segfault

        pthread_mutex_lock(&mutex_cantidad_procesos);
        cantidad_procesos_en_memoria++; // aumenta el grado de multiprogramacion
        pthread_mutex_unlock(&mutex_cantidad_procesos);

        socket_memoria = crear_conexion_memoria(configuracion_kernel, logger);
        send_inicializar_estructuras(socket_memoria); // para que la memoria inicialice estructuras y obtenga el valor de la TP
        if (recv(socket_memoria, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            loggear_error(logger, "Error al recibir el op_code INICIALIZAR_ESTRUCTURAS de la memoria", mutex_logger_kernel);
            return;
        }
        recv_valor_tb(socket_memoria, &tope_cola_new->tabla_pagina); // recibe el valor de la tp y lo guarda en el pcb

        liberar_conexion(socket_memoria);

        queue_push_con_mutex(cola_ready, tope_cola_new, mutex_cola_ready); // agrega el pcb a la cola de ready
        sem_post(&sem_nuevo_ready);

        if (queue_size_con_mutex(cola_ready, mutex_cola_ready) == 1)
        { // si el pcb recien agregado es el primero (o sea no habia nadie en ready antes), planifico.
            sem_post(&sem_nuevo_ready);
            sem_post(&sem_planificar);
        } else {
            sem_post(&sem_nuevo_ready);
        }
    }

    // Asi como está, si no lo permite el grado de multiprogramacion, lo deja en new.
    // TODO: Hacer que cuando se recibe una orden de EXIT, al liberar espacio dentro del grado de multiprogramacion,
    // verificar si hay procesos en la cola new para meter dentro de ready y planificar.

    return;
}

bool consulta_grado()
{
    pthread_mutex_lock(&mutex_cantidad_procesos);
    if (cantidad_procesos_en_memoria <= configuracion_kernel->grado_multiprogramacion)
    {
        pthread_mutex_unlock(&mutex_cantidad_procesos);
        return true;
    }
    pthread_mutex_unlock(&mutex_cantidad_procesos);
    return false;
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
            sem_wait(&sem_nuevo_ready);
            sem_wait(&sem_planificar);
            pcb_tope_lista = queue_pop_con_mutex(cola_ready, mutex_cola_ready);
            pthread_mutex_lock(&mutex_estado_running);
            if (running == NULL)
            {
                pthread_mutex_unlock(&mutex_estado_running);

                pthread_mutex_lock(&mutex_estado_running);
                running = pcb_tope_lista; // modifico el running de kernel
                pthread_mutex_unlock(&mutex_estado_running);

                socket_cpu_dispatch = crear_conexion_cpu_dispatch(configuracion_kernel, logger);

                send_pcb(socket_cpu_dispatch, pcb_tope_lista); // mando el pcb a la CPU
                sem_post(&sem_recibir); // activo al hilo recibir para que se ponga a la espera de un mensaje de la CPU
                printf("MANDE EL PCB A LA CPU \n");
                close(socket_cpu_dispatch);

                pthread_mutex_lock(&mutex_estado_running);
            }
            pthread_mutex_unlock(&mutex_estado_running);
        }
        break;

    case SJF: // con desalojo
        // sem_wait(&planificar) || sem_wait(&nuevoAReady);
        // guardar en timer.start en blblio timer cuando se y cuando vuelve comparar el tiempo

        break;
    }
    return;
}

void recibir()
{
    op_code cop;
    op_code cop2;
    t_pcb *pcb;
    t_pcb *pcbExit;
    uint32_t tiempo;
    while (true)
    {   
        sem_wait(&sem_recibir);
        if (recv(socket_cpu_dispatch, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            loggear_info(logger, "DISCONNECT", mutex_logger_kernel);
            return;
        }

        switch(cop)
        {
        case BLOQUEO_IO:
            recv_pcb_con_tiempo_bloqueado(socket_cpu_dispatch, &pcb, &tiempo);

            pcb->tiempo_bloqueo = tiempo;

            pthread_mutex_lock(&mutex_estado_running);
            running = NULL; // modifico el running de kernel
            pthread_mutex_unlock(&mutex_estado_running);

            queue_push_con_mutex(cola_blocked, pcb, mutex_cola_blocked);

            loggear_info(logger, "Nuevo proceso en cola blocked", mutex_logger_kernel);

            sem_post(&sem_planificar); // como se libero la cpu por I/O, puedo poner a otro a planificar
            sem_post(&sem_nuevo_bloqued); // aviso que hay un nuevo pcb en la cola de blocked
            break;
        
        case ENVIAR_PCB:
            recv_pcb(socket_cpu_dispatch, &pcbExit);
            loggear_info(logger, "Orden EXIT recibida", mutex_logger_kernel);
            
            pthread_mutex_lock(&mutex_estado_running);
            running = NULL; // modifico el running de kernel
            pthread_mutex_unlock(&mutex_estado_running);

            sem_post(&sem_planificar); // como se libero la cpu por EXIT, puedo poner a otro a planificar

            socket_memoria = crear_conexion_memoria(configuracion_kernel, logger);
            send_fin_proceso(socket_memoria); // le aviso a la memoria que el proceso termino para que libere las estructuras

            if (recv(socket_memoria, &cop2, sizeof(op_code), 0) != sizeof(op_code))
            { 
                loggear_error(logger, "Error al recibir el op code LIBERAR ESTRUCTURAS", mutex_logger_kernel);
                return;
            }
            liberar_conexion(socket_memoria);

            pthread_mutex_lock(&mutex_cantidad_procesos);
            cantidad_procesos_en_memoria--; // baja el nivel de multiprogramacion
            pthread_mutex_unlock(&mutex_cantidad_procesos);

            revisar_new(); // como se libero espacio dentro del grado de multiprogramacion, revisa si hay algun proceso en new para meterlo en ready

            /* TODO: VER COMO CONTESTARLE A LA CONSOLA ESPECIFICA QUE TERMINO (Analizar: https://stackoverflow.com/questions/57730441/sockets-programming-sending-and-receiving-different-data-to-different-clients-i)
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
                pthread_
            }
            */

            break;
        }

        
    liberar_conexion(socket_cpu_dispatch);
    }
}

void revisar_new(){
    if(consulta_grado() && !queue_vacia_con_mutex(cola_new, mutex_cola_new)){
        t_pcb* pcb = queue_pop_con_mutex(cola_new, mutex_cola_new);
        queue_push_con_mutex(cola_ready, pcb, mutex_cola_ready);
        sem_post(&sem_nuevo_ready);
    }
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

void bloquear(){
    t_pcb* pcb;
    uint32_t tiempo;
    while(true){
        sem_wait(&sem_nuevo_bloqued); // espero a que haya alguien en la cola de blocked

        pcb = queue_pop_con_mutex(cola_blocked, mutex_cola_blocked);
        tiempo = pcb->tiempo_bloqueo;
        usleep(tiempo * 1000);

        queue_push_con_mutex(cola_ready, pcb, mutex_cola_ready);
        sem_post(&sem_nuevo_ready); // aviso que meti un nuevo pcb en la cola de ready
    }
}