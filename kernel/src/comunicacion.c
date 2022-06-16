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
    uint32_t *cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

    op_code cop;
    uint32_t tamanio = 0;
    uint32_t tiempo_bloqueo;
    t_list *instrucciones = NULL;
    t_pcb *pcb;

    while (*cliente_socket != -1) // mientras el cliente no se haya desconectado. (Es el socket de la consola)
    {
        if (recv(*cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code))
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
            if (recv_iniciar_consola(*cliente_socket, &instrucciones, &tamanio))
            {
                loggear_info(logger, "Se recibieron las instrucciones", mutex_logger_kernel);
                loggear_lista_instrucciones(instrucciones, logger);

                t_pcb *pcb = crear_pcb(instrucciones, logger, tamanio, cliente_socket);

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
    uint32_t *cliente_socket = esperar_cliente(logger, server_name, server_socket); // cuando se conecta un cliente nos devuelve la "linea" donde estan conectados
    pthread_t thread;                                                               // crea un hilo para procesar la conexion
    if (*cliente_socket != -1)
    {                                                                              // si se conecto un cliente
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args)); // crea una estructura para pasarle los argumentos al hilo
        args->log = logger;                                                        // guarda el logger en la estructura
        args->fd = cliente_socket;                                                 // guarda el socket del cliente en la estructura
        args->server_name = server_name;                                           // guarda el nombre del servidor en la estructura
        pthread_create(&thread, NULL, (void *)procesar_conexion, args);
        pthread_detach(thread);
        return 1; // devuelve 1 para indicar que se conecto un cliente
    }
    liberar_conexion(server_socket);
    return 0; // devuelve 0 si no se conecto nadie
}

t_pcb *crear_pcb(t_list *instrucciones, t_log *logger, uint32_t tamanio, uint32_t *cliente_socket)
{
    t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->id = cantidad_procesos_en_memoria;
    pcb->instrucciones = instrucciones;
    pcb->program_counter = 0;
    pcb->tamanio = tamanio;
    pcb->tabla_pagina = 0;
    pcb->tiempo_bloqueo = 0;
    pcb->cliente_socket = cliente_socket;
    pcb->estimacion_rafaga_anterior = atof(configuracion_kernel->estimacion_inicial);
    pcb->rafaga_real_anterior = 0; // como es la primera vez, no corrio antes -> da 0
    pcb->suspendido = false;
    return pcb;
}

void verificacion_multiprogramacion(t_pcb *pcb)
{
    op_code cop;
    queue_push_con_mutex(cola_new, pcb, mutex_cola_new);
    if (consulta_grado())
    {
        t_pcb *tope_cola_new = queue_pop_con_mutex(cola_new, mutex_cola_new); 

        pthread_mutex_lock(&mutex_cantidad_procesos);
        cantidad_procesos_en_memoria++; // aumenta el grado de multiprogramacion
        pthread_mutex_unlock(&mutex_cantidad_procesos);

        socket_memoria = crear_conexion_memoria(configuracion_kernel, logger); //se conecta con el server de memoria
        send_inicializar_estructuras(socket_memoria); // para que la memoria inicialice estructuras y obtenga el valor de la TP
        if (recv(socket_memoria, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            loggear_error(logger, "Error al recibir el op_code INICIALIZAR_ESTRUCTURAS de la memoria", mutex_logger_kernel);
            return;
        }
        recv_valor_tb(socket_memoria, &tope_cola_new->tabla_pagina); // recibe el valor de la tp y lo guarda en el pcb

        liberar_conexion(socket_memoria);

        list_add_con_mutex(cola_ready, tope_cola_new, mutex_cola_ready); // agrega el pcb a la cola de ready

        sem_post(&sem_nuevo_ready);

        if (list_size_con_mutex(cola_ready, mutex_cola_ready) == 1)
        { // si el pcb recien agregado es el primero (o sea no habia nadie en ready antes), planifico.
            sem_post(&sem_planificar);
            return;
        }
        // HAY QUE PLANIFICAR SI SOS SJF
        if (strcmp(configuracion_kernel->algoritmo_planificacion, "SRT") == 0)
        {
            sem_post(&sem_planificar);
        }
    }
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
    socket_cpu_dispatch = crear_conexion_cpu_dispatch(configuracion_kernel, logger);   // linea donde estan conectados la cpu (dispatch) y el kernel
    socket_cpu_interrupt = crear_conexion_cpu_interrupt(configuracion_kernel, logger); // linea donde estan conectados la cpu (interrupt) y el kernel
    t_pcb *pcb_tope_lista;
    switch (algoritmo) // segun el algortimo planifica
    {
    case FIFO:
        while (true) // para que lo ejecute todo el tiempo
        {

            sem_wait(&sem_nuevo_ready);                                                      // espera a que llegue alguien a ready
            sem_wait(&sem_planificar);                                                       // espera que lo habiliten a planificar
            sem_wait(&sem_running);                                                          // avisa q no hay nadie en running
            pcb_tope_lista = list_get_and_remove_con_mutex(cola_ready, 0, mutex_cola_ready); // saco proceso de la lista de ready
            pthread_mutex_lock(&mutex_estado_running);
            if (running == NULL) // no hay nadie ejecutando lo pongo a ejecutar
            {
                pthread_mutex_unlock(&mutex_estado_running);

                pthread_mutex_lock(&mutex_estado_running);
                running = pcb_tope_lista; // modifico el running de kernel
                pthread_mutex_unlock(&mutex_estado_running);

                send_pcb(socket_cpu_dispatch, pcb_tope_lista, ENVIAR_PCB); // mando el pcb a la CPU a traves del socket dispatch
                sem_post(&sem_recibir);                                    // activo al hilo recibir para que se ponga a la espera de un mensaje de la CPU
                printf("MANDE EL PCB A LA CPU, %d \n", pcb_tope_lista->id);
                destruir_pcb(pcb_tope_lista);

                pthread_mutex_lock(&mutex_estado_running);
            }
            pthread_mutex_unlock(&mutex_estado_running);
        }
        break;

    case SJF:
        while (true)
        {
            sem_wait(&sem_nuevo_ready); // espera a que llegue alguien a ready
            sem_wait(&sem_planificar);  // espera que lo habiliten a planificar

            pthread_mutex_lock(&mutex_estado_running);
            if (running == NULL)
            {
                t_pcb *pcb_elegido = realizar_estimacion(); // si un proceso termina de ejecutar en running no hay nadie pero tenemos que estimar de los procesos que estan en ready quien ejecuta
                pthread_mutex_unlock(&mutex_estado_running);

                pthread_mutex_lock(&mutex_estado_running);

                running = pcb_elegido;  // modifico el running
                fecha_inicio = time(0); // guarda la hora actual de inicio de ejecucion
                pthread_mutex_unlock(&mutex_estado_running);

                send_pcb(socket_cpu_dispatch, pcb_elegido, ENVIAR_PCB); // mando el pcb a la CPU a traves del socket dispatch
                sem_post(&sem_recibir);                                 // activo al hilo recibir para que se ponga a la espera de un mensaje de la CPU
                printf("MANDE EL PCB A LA CPU, %d \n", pcb_elegido->id);
                destruir_pcb(pcb_elegido);

                pthread_mutex_lock(&mutex_estado_running);
            }
            else // si hay alguien corriendo
            {
                pthread_mutex_unlock(&mutex_estado_running);
                send_interrupcion_por_nuevo_ready(socket_cpu_interrupt); // aviso a la cpu que hay un nuevo proceso en ready para que desaloje al proceso que esta ejecutando
                sem_post(&sem_recibir);                                  // activo al hilo recibir para que se ponga a la espera de un mensaje de la CPU
                sem_wait(&sem_desalojo);                                 // espera que lo habilite el kernel cuando le llega el pcb del proceso desalojado
                t_pcb *pcb_elegido = realizar_estimacion();              // de los procesos que estan en ready vemos quien ejecuta segun la estimacion
                send_pcb(socket_cpu_dispatch, pcb_elegido, ENVIAR_PCB);  // mando el pcb a la CPU a traves del socket dispatch para que lo ejecute
                printf("MANDE EL PCB A LA CPU, %d \n", pcb_elegido->id);
                sem_post(&sem_recibir);
                pthread_mutex_lock(&mutex_estado_running);
            }
            pthread_mutex_unlock(&mutex_estado_running);
        }
        break;
    }
    return;
}

void recibir()
{
    op_code cop;
    op_code cop2;
    op_code cop3;
    t_pcb *pcb;
    t_pcb *pcbExit;
    uint32_t tiempo;
    while (true)
    {
        sem_wait(&sem_recibir);                                                     // espera que lo habiliten -> cuando el kernel le manda a la cpu el PCB para que lo ponga a ejecutar
        if (recv(socket_cpu_dispatch, &cop, sizeof(op_code), 0) != sizeof(op_code)) // espera recibir algun mensaje de la CPU porque alguna instruccion involucra que el kernel haga algo
        {
            loggear_info(logger, "DISCONNECT", mutex_logger_kernel);
            return;
        }

        switch (cop)
        {
        case BLOQUEO_IO:
            recv_pcb(socket_cpu_dispatch, &pcb); // recibe el PCB del proceso que se bloquea

            pthread_mutex_lock(&mutex_estado_running);
            running = NULL;        // modifico el running de
            fecha_final = time(0); // cuando sale de running, guardo la hora actual
            pthread_mutex_unlock(&mutex_estado_running);
            sem_post(&sem_running);

            pcb->rafaga_real_anterior = difftime(fecha_final, fecha_inicio) * 1000; // diferencia entre fecha final e inicial en milisegundos, nos dice cuanto ejecuto el proceso verdaderamente en milisegundos

            queue_push_con_mutex(cola_blocked, pcb, mutex_cola_blocked); // encolamos a los procesos que se van bloqueando

            // creo hilo para controlar el tiempo maximo que puede pasar un proceso en la cola de blocked
            pthread_t controlador_tiempo_blocked;
            pthread_create(&controlador_tiempo_blocked, NULL, (void *)controlador_tiempo_blocked_proceso, (void *)pcb);
            pthread_detach(controlador_tiempo_blocked);

            loggear_info(logger, "Nuevo proceso en cola blocked", mutex_logger_kernel);

            sem_post(&sem_planificar);    // como se libero la cpu por I/O, puedo poner a otro a planificar venite
            sem_post(&sem_nuevo_bloqued); // aviso que hay un nuevo pcb en la cola de blocked
            break;

        case ENVIAR_PCB: // kernel recibe el pcb para terminar el proceso
            recv_pcb(socket_cpu_dispatch, &pcbExit);
            loggear_info(logger, "Orden EXIT recibida", mutex_logger_kernel);

            pthread_mutex_lock(&mutex_estado_running);
            running = NULL; // modifico el running de kernel
            pthread_mutex_unlock(&mutex_estado_running);

            sem_post(&sem_running);
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
            revisar_new();
            send(*(pcbExit->cliente_socket), &cop2, sizeof(op_code), 0) != sizeof(op_code); // como se libero espacio dentro del grado de multiprogramacion, revisa si hay algun proceso en new para meterlo en ready
            free(pcbExit->cliente_socket);
            destruir_pcb(pcbExit);
            break;

        case INTERRUPCION:                       // desaloje a un proceso
            recv_pcb(socket_cpu_dispatch, &pcb); // recibe el PCB del proceso que se bloquea

            pthread_mutex_lock(&mutex_estado_running);
            running = NULL;
            fecha_final = time(0); // cuando sale de running, guardo la hora actual
            pthread_mutex_unlock(&mutex_estado_running);

            pcb->rafaga_real_anterior = difftime(fecha_final, fecha_inicio) * 1000; // diferencia entre fecha final e inicial en milisegundos, nos dice cuanto ejecuto el proceso verdaderamente en milisegundos
            list_add_con_mutex(cola_ready, pcb, mutex_cola_ready);                  // mete en ready al proceso desalojado

            sem_post(&sem_nuevo_ready); // aviso que hay un nuevo pcb en la cola de ready
            sem_post(&sem_desalojo);    // aviso que llego el proceso desalojado y hay q planificar
            break;
        
        }
    }
}

void revisar_entrada_a_ready() // reviso si hay algun proceso en new o en suspended ready para meterlo en ready
{ 
    if (consulta_grado() && (!queue_vacia_con_mutex(cola_new, mutex_cola_new) || !queue_vacia_con_mutex(cola_ready_suspendido, mutex_cola_ready_suspendido)))//chequeo si hay algun proceso en new o en suspended ready
    {
        if(!queue_vacia_con_mutex(cola_ready_suspendido, mutex_cola_ready_suspendido)) //si hay alguien en ready suspendido -> lo priorizo y lo pongo en ready primero
        {
            t_pcb *pcb = queue_pop_con_mutex(cola_ready_suspendido, mutex_cola_ready_suspendido);
            list_add_con_mutex(cola_ready, pcb, mutex_cola_ready);
        } else{ //sino pongo en ready al que este en new
            t_pcb *pcb = queue_pop_con_mutex(cola_new, mutex_cola_new);
            list_add_con_mutex(cola_ready, pcb, mutex_cola_ready);
        }
        sem_post(&sem_nuevo_ready); // avisa que llego alguien a ready
        if(strcmp(configuracion_kernel->algoritmo_planificacion, "SRT") == 0)
        {
            sem_post(&sem_planificar); // aviso que hay un nuevo pcb en la cola de ready
        }
        loggear_info(logger, "Nuevo proceso en cola ready", mutex_logger_kernel);
    }
}

ALGORITMO algortimo_de_planificacion(char *algortimo_de_planificacion) // para devolver el enum que corresponde
{
    if (strcmp(algortimo_de_planificacion, "FIFO") == 0)
    {
        return FIFO;
    }
    else if (strcmp(algortimo_de_planificacion, "SRT") == 0)
    {
        return SJF;
    }
    return ERROR;
}

void bloquear()
{
    t_pcb *pcb;
    uint32_t tiempo;
    while (true)
    {
        sem_wait(&sem_nuevo_bloqued); // espero a que haya alguien en la cola de blocked

        pcb = queue_pop_con_mutex(cola_blocked, mutex_cola_blocked); // saca a un proceso de la cola de blocked
        tiempo = pcb->tiempo_bloqueo;
        usleep(tiempo * 1000); // bloquea al proceso

        if (pcb->suspendido)
        {
            pcb->blocked_suspendido = false;
            list_add_con_mutex(cola_ready_suspendido, pcb, mutex_cola_ready_suspendido);
        }
        else
        {
            list_add_con_mutex(cola_ready, pcb, mutex_cola_ready); // cuando se despierta lo pone en ready

            sem_post(&sem_nuevo_ready); // aviso que meti un nuevo pcb en la cola de ready y puede planificar
            if (strcmp(configuracion_kernel->algoritmo_planificacion, "SRT") == 0)
            {
                sem_post(&sem_planificar);
            }
        }
    }
}

t_pcb *realizar_estimacion()
{ // devuelve el pcb que se va a ejecutar
    t_pcb *pcb_elegido;
    float alfa = atof(configuracion_kernel->alfa); // de char a float

    // recorrer lista de pcbs en ready y buscar el que tiene el valor de rafaga mas bajo
    for (int i = 0; i < list_size(cola_ready); i++)
    {
        t_pcb *aux = list_get(cola_ready, i);
        if (aux->rafaga_real_anterior == 0)
        {
            aux->estimacion_rafaga_anterior = aux->estimacion_rafaga_anterior;
        }
        else
        {
            aux->estimacion_rafaga_anterior = alfa * (aux->rafaga_real_anterior) + (1 - alfa) * (aux->estimacion_rafaga_anterior); // en estimacion de la rafafa anterior guardamos la ultima estimacion // estimacion rafaga siguiente = alfa * (duracion rafaga anterior real) + (1-alfa) * (estimacion de la rafaga anterior)
        }
        printf("el pcb %d tiene estimacion de rafaga: %f\n", aux->id, aux->estimacion_rafaga_anterior);
    }

    list_sort(cola_ready, (void *)comparar_estimaciones); // ordeno la lista de ready por estimacion de rafaga - la cola ready ya viene ordenada por FIFO
    pcb_elegido = list_remove(cola_ready, 0);

    return pcb_elegido;
}

bool comparar_estimaciones(t_pcb *pcb1, t_pcb *pcb2)
{
    return pcb1->estimacion_rafaga_anterior < pcb2->estimacion_rafaga_anterior;
}

void controlador_tiempo_blocked_proceso(t_pcb *pcb)
{
    usleep((configuracion_kernel->tiempo_maximo_bloqueado) * 1000); // espero el tiempo maximo de bloqueado definido en el archivo de config
    // si el proceso sigue en la cola de bloqueados, pongo pcb->suspendido=true;
    if (queue_find_con_mutex(cola_blocked, pcb, mutex_cola_blocked) != NULL)
    {
        comunicacion_suspension_memoria(pcb);
        
        pthread_mutex_lock(&mutex_cantidad_procesos);
        cantidad_procesos_en_memoria--;
        pthread_mutex_unlock(&mutex_cantidad_procesos);

        revisar_entrada_a_ready();
    }
}

void comunicacion_suspension_memoria(t_pcb *pcb){
    socket_memoria = crear_conexion_memoria(configuracion_kernel, logger);

    send_suspension(socket_memoria, pcb->id);
    
    if(recv(socket_memoria, &cop3, sizeof(op_code)) != sizeof(op_code)){
        loggear_error(logger, "Error al recibir el op code CONFIRMACION SUSPENSION", mutex_logger_kernel);
        return;
    }

    loggear_info(logger, "Confirmacion de suspension recibida", mutex_logger_kernel);
    pcb->blocked_suspendido = true;
    liberar_conexion(socket_memoria);
}
