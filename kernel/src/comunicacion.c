#include "../include/comunicacion.h"

uint32_t cantidad_procesos_en_memoria = 0;  //variable global para asignar el ID a los procesos que llegan y para verificar el grado de multiprogramacion

uint32_t crear_comunicacion(t_configuracion_kernel *configuracion_kernel, t_log *logger) //inicio el servidor 
{ 

    socket_kernel = iniciar_servidor(logger, "CONSOLA", configuracion_kernel->ip_kernel, configuracion_kernel->puerto_escucha); //levanto el servidor para el cliente: consola en este caso

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
    uint32_t cliente_socket = args->fd; 
    char *server_name = args->server_name;
    free(args);

    op_code cop;
    uint32_t tamanio = 0;
    uint32_t tiempo_bloqueo; 
    t_list *instrucciones = NULL;
    t_pcb *pcb; 

    while (cliente_socket != -1) //mientras el cliente no se haya desconectado
    {
        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code))    
        {
            log_info(logger, "DISCONNECT!"); //desconectamos al cliente  xq no le esta mandando el cop bien 
            return;
        }

        switch (cop) 
        {
        case DEBUG: //para probar 

            log_info(logger, "debug");
            break;

        case INICIAR_PROCESO: 

            if (recv_iniciar_consola(cliente_socket, &instrucciones, &tamanio)) //recibe las instrucciones y el tamanio del proceso
            {
                log_info(logger, "Se recibieron las instrucciones");
                log_info(logger, "Tamanio de la consola: %d", tamanio);
                log_info(logger, "Cantidad de instrucciones: %d", list_size(instrucciones));

                socket_cpu_dispatch = crear_conexion_cpu_dispatch(configuracion_kernel, logger);  //"kernel" cliente de cpu (dispatch) para pasarle el pcb 
                socket_cpu_interrupt = crear_conexion_cpu_interrupt(configuracion_kernel, logger); 
                socket_memoria = crear_conexion_memoria(configuracion_kernel, logger); //kernel cliente de memoria
                t_pcb *pcb = crear_pcb(instrucciones, logger, tamanio); 

                verificacion_multiprogramacion(pcb, cop); // planificador de largo plazo

                atencion_cpu(socket_cpu_dispatch, logger); 
                // liberar memoria
                free(pcb);
                list_destroy_and_destroy_elements(instrucciones, (void *)destruir_instruccion);
            }
            else
            {
                log_error(logger, "No se recibieron las instrucciones");
            }
            break;
        
        case BLOQUEO_IO:
            recv_pcb_con_tiempo_bloqueado(socket_cpu_dispatch, &pcb, &tiempo_bloqueo);
            break;

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

uint32_t server_escuchar(t_log *logger, char *server_name, uint32_t server_socket)
{
    uint32_t cliente_socket = esperar_cliente(logger, server_name, server_socket); // cuando se conecta un cliente nos devuelve la "linea" donde estan conectados

    if (cliente_socket != -1)
    {                                                                              // si se conecto un cliente
        pthread_t hilo;                                                            // crea un hilo para procesar la conexion
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args)); // crea una estructura para pasarle los argumentos al hilo
        args->log = logger;                                                        // guarda el logger en la estructura
        args->fd = cliente_socket;                                                 // guarda el socket del cliente en la estructura
        args->server_name = server_name;                                           // guarda el nombre del servidor en la estructura
        pthread_create(&hilo, NULL, (void *)procesar_conexion, (void *)args);      // crea el hilo
        pthread_detach(hilo);                                                      // lo desconecta del hilo
        return 1;                                                                   // devuelve 1 para indicar que se conecto un cliente
    }
    return 0; //devuelve 0 si no se conecto nadie
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

void verificacion_multiprogramacion(t_pcb *pcb, op_code cop)
{
    queue_push(cola_new, pcb); //agrego un proceso a la cola de nuevos
    if (cantidad_procesos_en_memoria <= configuracion_kernel->grado_multiprogramacion) //si el grado de multi lo permite
    {
        t_pcb *consola_tope_lista = queue_pop(cola_new); 
        queue_push(cola_ready, consola_tope_lista);  // agregar el pcb a la cola de ready
        pthread_mutex_lock(&mutex_cantidad_procesos);
        cantidad_procesos_en_memoria++;
        pthread_mutex_unlock(&mutex_cantidad_procesos);
        send_inicializar_estructuras(socket_memoria); 
        if (recv(socket_memoria, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            return; 
        }
        recv_valor_tb(socket_memoria, &consola_tope_lista->tabla_pagina); //recibe el valor de la tp y lo guarda en el pcb

        if (strcmp(configuracion_kernel->algoritmo_planificacion, "SRT") != 0) //Sjf con desalojo
        {

            pthread_mutex_lock(&mutex_estado_running);
            if (running == NULL && queue_size(cola_ready) == 1) //el proceso es el unico en la cola de ready y nadie esta en running
            {
                pthread_mutex_unlock(&mutex_estado_running); 
                pthread_mutex_lock(&mutex_estado_running);
                running = consola_tope_lista; //pongo en running al proceso que esta en ready 
                pthread_mutex_unlock(&mutex_estado_running);
                send_pcb(socket_cpu_dispatch, running); // cuando pongo a ejecutar un proceso tengo que mandar a la CPU a través de la conexión de dispatch el PCB
            }
            else
            {
                pthread_mutex_unlock(&mutex_estado_running);
                t_pcb *pcb_desalojado;
                send_interrupcion_por_nuevo_ready(socket_cpu_interrupt);
                recv_pcb(socket_cpu_dispatch, pcb_desalojado);
                pthread_mutex_lock(&mutex_estado_running);
                running = NULL;
                pthread_mutex_unlock(&mutex_estado_running);
                queue_push(cola_ready, pcb_desalojado);
                // planificar
            }
        }
        else
        {
            pthread_mutex_lock(&mutex_estado_running);
            if (running == NULL && queue_size(cola_ready) == 1)
            {
                pthread_mutex_unlock(&mutex_estado_running);
                pthread_mutex_lock(&mutex_estado_running);
                running = consola_tope_lista;
                pthread_mutex_unlock(&mutex_estado_running);
                send_pcb(socket_cpu_dispatch, running); // si no hay nadie ejecutando y estas solo en ready
                pthread_mutex_lock(&mutex_estado_running);
            }
            pthread_mutex_unlock(&mutex_estado_running);
        }
    }
    return;
}

void atencion_cpu(uint32_t socket_cpu_dispatch, t_log *logger)
{
    op_code cop;
    t_pcb *pcb;
    uint32_t tiempo;
    if (recv(socket_cpu_dispatch, &cop, sizeof(op_code), 0) != sizeof(op_code))
    {
        log_info(logger, "DISCONNECT!");
        return;
    }
    switch (cop)
    {
    case BLOQUEO_IO:
        recv_pcb_con_tiempo_bloqueado(socket_cpu_dispatch, pcb, &tiempo);
        printf("Tiempo bloqueado recibido: %d\n", tiempo);
        pthread_mutex_lock(&mutex_estado_running);
        running = NULL;
        pthread_mutex_unlock(&mutex_estado_running);
        
        queue_push(cola_blocked, pcb);

        break;
    }
}
