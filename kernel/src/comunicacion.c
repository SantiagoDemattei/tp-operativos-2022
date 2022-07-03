#include "../include/comunicacion.h"

uint32_t cantidad_procesos_en_memoria = 0; // para verificar el grado de multiprogramacion
uint32_t contador = 1;                     // variable global para asignar el ID a los procesos que llegan
uint32_t id_desalojado; 
double estimacion_desalojado=0;
double real_desalojado=0;

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
    t_procesar_conexion_args *args = (t_procesar_conexion_args *)void_args; // recibo a mi cliente y sus datos
    t_log *logger = args->log;
    uint32_t *cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

    op_code cop;
    uint32_t tamanio = 0;
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
uint32_t server_escuchar(t_log *logger, char *server_name, uint32_t server_socket) //crea un hilo para atender a cada cliente 
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

    pthread_mutex_lock(&mutex_variable_contador);
    pcb->id = contador; // para que no haya varios procesos con el mismo ID
    contador++;
    pthread_mutex_unlock(&mutex_variable_contador);

    pcb->instrucciones = instrucciones;
    pcb->program_counter = 0;
    pcb->tamanio = tamanio;
    pcb->tabla_pagina = 0;
    pcb->tiempo_bloqueo = 0;
    pcb->cliente_socket = cliente_socket;//socket de la consola que le manda el proceso
    pcb->estimacion_rafaga_anterior = atof(configuracion_kernel->estimacion_inicial);
    pcb->rafaga_real_anterior = 0; // como es la primera vez, no corrio antes -> da 0
    pcb->blocked_suspendido = false;
    return pcb;
}

void verificacion_multiprogramacion(t_pcb *pcb)
{
    op_code cop;
    queue_push_con_mutex(cola_new, pcb, mutex_cola_new); // agrega el pcb a la cola de NEW
    if (consulta_grado()) //si el grado de multiprogramacion lo permite 
    {
        t_pcb *tope_cola_new = queue_pop_con_mutex(cola_new, mutex_cola_new); // saca el pcb del tope de la cola de NEW

        pthread_mutex_lock(&mutex_cantidad_procesos);
        cantidad_procesos_en_memoria++; // aumenta el grado de multiprogramacion
        pthread_mutex_unlock(&mutex_cantidad_procesos);

        socket_memoria = crear_conexion_memoria(configuracion_kernel, logger); // se conecta con el server de memoria
        printf("el id es %d\n", tope_cola_new->id);
        send_inicializar_estructuras(socket_memoria, tope_cola_new->tamanio, tope_cola_new->id);                          // para que la memoria inicialice estructuras y obtenga el valor de la TP
        if (recv(socket_memoria, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            loggear_error(logger, "Error al recibir el op_code INICIALIZAR_ESTRUCTURAS de la memoria", mutex_logger_kernel);
            return;
        }
        recv_valor_tb(socket_memoria, &tope_cola_new->tabla_pagina); // recibe el id de la tabla de paginas y lo guarda en el pcb
        printf("el id de la tabla de pagina de %d es %d\n", tope_cola_new->id, tope_cola_new->tabla_pagina);
        liberar_conexion(socket_memoria);

        list_add_con_mutex(cola_ready, tope_cola_new, mutex_cola_ready); // agrega el pcb a la cola de ready

        sem_post(&sem_nuevo_ready); // nuevo pcb en ready para que sepa el planificador

        if (list_size_con_mutex(cola_ready, mutex_cola_ready) == 1)
        { // si el pcb recien agregado es el primero (o sea no habia nadie en ready antes), planifico (tanto en fifo como en sjf)
            sem_post(&sem_planificar); //habilito al planificador
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
    printf("Cantidad de procesos en memoria: %d\n", cantidad_procesos_en_memoria);
    if (cantidad_procesos_en_memoria < configuracion_kernel->grado_multiprogramacion)
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
            sem_wait(&sem_running);                                                          // espera que no haya nadie en running
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
            if (running == NULL) //va a ser null solo la primera vez o si estan todos bloqueados (si estan todos bloqueados no hay ninguno corriendo)
            {
                t_pcb *pcb_elegido = realizar_estimacion(); // si un proceso termina de ejecutar en running no hay nadie pero tenemos que estimar de los procesos que estan en ready quien ejecuta
                pthread_mutex_unlock(&mutex_estado_running);

                pthread_mutex_lock(&mutex_estado_running);
                running = pcb_elegido;     // modifico el running
                fecha_inicio = time(NULL); // guarda la hora actual de inicio de ejecucion
                pthread_mutex_unlock(&mutex_estado_running);

                send_pcb(socket_cpu_dispatch, pcb_elegido, ENVIAR_PCB); // mando el pcb a la CPU a traves del socket dispatch
                sem_post(&sem_recibir);                                 // activo al hilo recibir para que se ponga a la espera de un mensaje de la CPU
                printf("MANDE EL PCB A LA CPU, %d \n", pcb_elegido->id);
                destruir_pcb(pcb_elegido);

                pthread_mutex_lock(&mutex_estado_running);
            }
            else // si hay alguien corriendo (viene aca a partir del segundo proceso, nunca entra aca con el primero)
            {
                pthread_mutex_unlock(&mutex_estado_running);
                send_interrupcion_por_nuevo_ready(socket_cpu_interrupt); // aviso a la cpu que hay un nuevo proceso en ready para que interrumpa al proceso que esta ejecutando
                sem_post(&sem_recibir);                                  // activo al hilo recibir para que se ponga a la espera de un mensaje de la CPU
                sem_wait(&sem_desalojo);                                // espera que lo habilite el kernel cuando le llega el pcb del proceso desalojado 
                t_pcb *pcb_elegido = realizar_estimacion();  
               
                pthread_mutex_lock(&mutex_info_desalojado);                                                            
                if(pcb_elegido->id == id_desalojado){ // si el que elegi es el mismo que el que estaba antes en running (el que acabo de desalojar)
                    
                    pcb_elegido->estimacion_rafaga_anterior = estimacion_desalojado; //al pedo porque ya se lo asigne cuando lo desalojamos pero funciona
                    pcb_elegido->rafaga_real_anterior = real_desalojado;//lo que llego a ejecutar antes de que sea interrumpido
                }                                         
                pthread_mutex_unlock(&mutex_info_desalojado);                 
                
                send_pcb(socket_cpu_dispatch, pcb_elegido, ENVIAR_PCB); // mando el pcb a la CPU a traves del socket dispatch para que lo ejecute

                pthread_mutex_lock(&mutex_estado_running);
                running = pcb_elegido; // modifico el running
                fecha_inicio = time(NULL); //setea la fecha de inicio de ejecucion porque es un proceso nuevo o continua el mismo(fue desalojado y se volvio a elegir)
                pthread_mutex_unlock(&mutex_estado_running);

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

void controlador_tiempo_blocked_proceso(t_pcb *pcb)
{
    
    usleep((atoi(configuracion_kernel->tiempo_maximo_bloqueado)) * 1000); // espero el tiempo maximo de bloqueado definido en el archivo de config
    
    // si el proceso sigue en la cola de bloqueados, pongo pcb->suspendido=true;
    if (queue_find_con_mutex(cola_blocked, pcb, mutex_cola_blocked) != NULL) // si el proceso sigue en la cola de bloqueados, lo suspedno porque supero el tiempo maximo que podia estar bloqueado
    {
        printf("voy a suspender el proceso: %d\n", pcb->id);
        comunicacion_suspension_memoria(pcb); // cuando suspendo al proceso aviso a la memoria
        pthread_mutex_lock(&mutex_cantidad_procesos);
        cantidad_procesos_en_memoria--;
        pthread_mutex_unlock(&mutex_cantidad_procesos);
        sem_post(&sem_queue_suspended); //habilito al hilo de largo plazo que revise si hay alguien que pueda pasar a ready porque baje el grado de multiprogramacion
    }
    pthread_exit(NULL);
}

void recibir()//de cpu
{
    op_code cop;
    op_code cop2;
    t_pcb *pcb;
    t_pcb *pcb_exit;
    float alfa = atof(configuracion_kernel->alfa); 

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
            running = NULL;           // modifico el running del kernel porque no hay ningun proceso en ejecucion
            fecha_final = time(NULL); // cuando sale de running, guardo la hora actual (me importa para el sjf)
            pthread_mutex_unlock(&mutex_estado_running);
            sem_post(&sem_running); // habilito el semaforo de running porque no hay ningun proceso corriendo

            //cuando el proceso se bloquea tenemos que calcular la estimacion para dsp cuando vaya a ready podamos elegir quien tiene la menor 
            pthread_mutex_lock(&mutex_info_desalojado); 
            if(pcb->id == id_desalojado){   //si el proceso que se bloquea es el que habiamos desalojado, la rafaga anterior va a ser lo que ya habia ejecutado antes de desalojarse () + lo nuevo que ejecuto cuando lo volvimos a poner en running 
                pcb->rafaga_real_anterior = difftime(fecha_final, fecha_inicio) * 1000 + pcb->rafaga_real_anterior; 
            }
            else{
                pcb->rafaga_real_anterior = difftime(fecha_final, fecha_inicio) * 1000;  
            }
            pthread_mutex_unlock(&mutex_info_desalojado);
            
             // diferencia entre fecha final e inicial en milisegundos, nos dice cuanto ejecuto el proceso verdaderamente en milisegundos
             
            pcb->estimacion_rafaga_anterior = alfa * (pcb->rafaga_real_anterior) + (1 - alfa) * (pcb->estimacion_rafaga_anterior);
            queue_push_con_mutex(cola_blocked, pcb, mutex_cola_blocked); // encolamos a los procesos que se van bloqueando

            // creo hilo para controlar el tiempo maximo que puede pasar un proceso en la cola de blocked
            pthread_t controlador_tiempo_blocked; 
            
            pthread_create(&controlador_tiempo_blocked, NULL, (void *)controlador_tiempo_blocked_proceso, (void *)pcb);
            pthread_detach(controlador_tiempo_blocked);
            

            loggear_info(logger, "Nuevo proceso en cola blocked", mutex_logger_kernel);

            sem_post(&sem_planificar);    // como se libero la cpu por I/O, puedo poner a otro a planificar venite
            sem_post(&sem_nuevo_bloqued); // aviso que hay un nuevo pcb en la cola de blocked
            break;

        case ENVIAR_PCB: // kernel recibe el pcb para terminar el proceso (exit)
            recv_pcb(socket_cpu_dispatch, &pcb_exit);
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
            liberar_conexion(socket_memoria); // libero la conexion con la memoria

            pthread_mutex_lock(&mutex_cantidad_procesos);
            cantidad_procesos_en_memoria--; // baja el nivel de multiprogramacion
            pthread_mutex_unlock(&mutex_cantidad_procesos);
            sem_post(&sem_queue_suspended); // habilito al hilo de largo plazo que revise si hay alguien que pueda pasar a ready porque baje el grado de multiprogramacion
            send(*(pcb_exit->cliente_socket), &cop2, sizeof(op_code), 0) != sizeof(op_code); 
            free(pcb_exit->cliente_socket);
            destruir_pcb(pcb_exit);
            break;

        case INTERRUPCION:                       // desaloje a un proceso
            recv_pcb(socket_cpu_dispatch, &pcb); // recibe el PCB del proceso que se interrumpio

            pthread_mutex_lock(&mutex_estado_running);
            running = NULL;
            fecha_final = time(NULL); // cuando sale de running, guardo la hora actual
            pthread_mutex_unlock(&mutex_estado_running);

            pcb->rafaga_real_anterior = difftime(fecha_final, fecha_inicio) * 1000; // diferencia entre fecha final e inicial en milisegundos, nos dice cuanto tiempo ejecuto el proceso verdaderamente en milisegundos
            pcb->estimacion_rafaga_anterior = pcb->estimacion_rafaga_anterior - pcb->rafaga_real_anterior; // hago la estimacion antes de meterlo en ready (de lo que le falta ejecutar)
            
            pthread_mutex_lock(&mutex_info_desalojado);
            id_desalojado = pcb->id;
            estimacion_desalojado = pcb->estimacion_rafaga_anterior; //guardo la estimacion del proceso que se desalojo
            real_desalojado = pcb->rafaga_real_anterior;  //guardo la rafaga real del proceso que se desalojo
            pthread_mutex_unlock(&mutex_info_desalojado);

            list_add_con_mutex(cola_ready, pcb, mutex_cola_ready); // mete en ready al proceso desalojado
            
            sem_post(&sem_nuevo_ready); // aviso que hay un nuevo pcb en la cola de ready
            sem_post(&sem_desalojo);    // aviso que llego el proceso desalojado y hay q planificar
            break;
        }
    }
}

void revisar_entrada_a_ready() // reviso si hay algun proceso en new o en suspended ready para meterlo en ready, lo hace el hilo de largo plazo
{
    while (true)
    {
        sem_wait(&sem_queue_suspended); //espera que se haya bajado el grado de multiprogramacion (porque suspendi un proceso o porque alguno termino)
        if (consulta_grado() && (!queue_vacia_con_mutex(cola_new, mutex_cola_new) || !queue_vacia_con_mutex(cola_ready_suspendido, mutex_cola_ready_suspendido))) // chequeo si hay algun proceso en new o en suspended ready y que el grado de multiprogramacion me lo permita 
        {

            if (!queue_vacia_con_mutex(cola_ready_suspendido, mutex_cola_ready_suspendido)) // si hay alguien en ready suspendido -> lo priorizo y lo pongo en ready primero
            {
                t_pcb *pcb = queue_pop_con_mutex(cola_ready_suspendido, mutex_cola_ready_suspendido);//lo desuspende
                printf("voy a meter %d de ready suspendido a ready\n", pcb->id);
                pcb->blocked_suspendido = false;//pone el flag de suspendido a false
                list_add_con_mutex(cola_ready, pcb, mutex_cola_ready);//lo mete en la cola de ready normal (no la de ready suspendido)
            }
            else
            { // sino pongo en ready al que este en new
                t_pcb *pcb = queue_pop_con_mutex(cola_new, mutex_cola_new);
                list_add_con_mutex(cola_ready, pcb, mutex_cola_ready);
            }

            pthread_mutex_lock(&mutex_cantidad_procesos);
            cantidad_procesos_en_memoria++; // aumento el grado de multiprogramacion
            pthread_mutex_unlock(&mutex_cantidad_procesos);

            sem_post(&sem_nuevo_ready); // avisa que llego alguien a ready
            if (strcmp(configuracion_kernel->algoritmo_planificacion, "SRT") == 0)
            {
                sem_post(&sem_planificar); //si es SJF, como llego un proceso a ready tiene que planificar 
            }
            loggear_info(logger, "Nuevo proceso en cola ready", mutex_logger_kernel);
        }
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

        pcb = queue_peek_con_mutex(cola_blocked, mutex_cola_blocked); // agarra el pcb del proceso que se bloquea pero no lo saca de la cola block para poder suspenderlo si tuviera que hacerlo
        tiempo = pcb->tiempo_bloqueo;
        usleep(tiempo * 1000);                                 // bloquea al proceso
        queue_pop_con_mutex(cola_blocked, mutex_cola_blocked); // cuando termina el tiempo de bloqueo, saca el pcb de la cola de blocked

        if (pcb->blocked_suspendido)  //si el proceso estaba suspendido y se "desperto", lo pongo en ready suspendido 
        {
            queue_push_con_mutex(cola_ready_suspendido, pcb, mutex_cola_ready_suspendido);
            sem_post(&sem_queue_suspended); // habilito el semaforo del hilo de largo plazo para chequear si de los procesos en new y ready suspended hay alguien para meterlo en ready
        }
        else //si no esta suspendido 
        {
            list_add_con_mutex(cola_ready, pcb, mutex_cola_ready); // cuando se despierta lo pone en ready

            sem_post(&sem_nuevo_ready); // aviso que meti un nuevo pcb en la cola de ready y puede planificar
            if (strcmp(configuracion_kernel->algoritmo_planificacion, "SRT") == 0)
            {
                sem_post(&sem_planificar); // planifica solo si es SJF cuando llega uno a ready
            }
        }
    }
}

t_pcb *realizar_estimacion()
{ // devuelve el pcb que se va a ejecutar
    t_pcb *pcb_elegido;
    // recorrer lista de pcbs en ready y buscar el que tiene el valor de rafaga mas bajo
    pthread_mutex_lock(&mutex_cola_ready);
    pcb_elegido = list_get_minimum(cola_ready, (void *)comparar_estimaciones); // busco y devuelve el pcb con la estimacion mas baja
    
    bool(closure)(void *data)
    {
        t_pcb *pcb = (t_pcb *)data;
        return criterio_id_lista(pcb_elegido, pcb);//busca si ese pcb esta en esa cola de ready
    }
    
    list_remove_by_condition(cola_ready, closure); // recorre la lista de ready y cuando lo encuentra lo saca 
    pthread_mutex_unlock(&mutex_cola_ready);
    return pcb_elegido;
}

void *comparar_estimaciones(t_pcb *pcb1, t_pcb *pcb2)
{   
    return pcb1->estimacion_rafaga_anterior <= pcb2->estimacion_rafaga_anterior ? pcb1 : pcb2;
}

bool criterio_id_lista(t_pcb *pcb_buscado, t_pcb *pcb_de_la_cola)
{
    return pcb_de_la_cola->id == pcb_buscado->id;
}

void comunicacion_suspension_memoria(t_pcb *pcb)
{
    socket_memoria = crear_conexion_memoria(configuracion_kernel, logger); // creo la conexion con la memoria
    op_code cop3;

    send_suspension(socket_memoria, pcb->id); // aviso a la memoria que el proceso se va a suspender

    if (recv(socket_memoria, &cop3, sizeof(op_code), 0) != sizeof(op_code))
    { // se queda esperando la confirmacion de la memoria para suspender al proceso
        loggear_error(logger, "Error al recibir el op code CONFIRMACION SUSPENSION", mutex_logger_kernel);
        return;
    }

    loggear_info(logger, "Confirmacion de suspension recibida", mutex_logger_kernel);
    pcb->blocked_suspendido = true;   // suspendo al proceso
    liberar_conexion(socket_memoria); // cierro la conexion con la memoria
}
