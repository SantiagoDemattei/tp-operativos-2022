#include "../include/comunicacion.h"

uint32_t crear_comunicacion_dispatch(t_configuracion_cpu *t_configuracion_cpu, t_log *logger)
{ // funcion de servidor

    uint32_t socket_cpu_dispatch = iniciar_servidor(logger, "KERNEL DISPATCH", t_configuracion_cpu->ip_cpu, t_configuracion_cpu->puerto_escucha_dispatch);

    if (socket_cpu_dispatch == -1)
    {
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        return -1;
    }
    return socket_cpu_dispatch;
}

uint32_t crear_comunicacion_interrupt(t_configuracion_cpu *t_configuracion_cpu, t_log *logger)
{ // funcion de servidor

    uint32_t socket_cpu_interrupt = iniciar_servidor(logger, "KERNEL INTERRUPT", t_configuracion_cpu->ip_cpu, t_configuracion_cpu->puerto_escucha_interrupt);

    if (socket_cpu_interrupt == -1)
    {
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        return -1;
    }
    return socket_cpu_interrupt;
}

uint32_t crear_conexion_memoria(t_configuracion_cpu *datos_conexion, t_log *logger) // funcion de cliente de memoria
{
    uint32_t socket_memoria = crear_conexion_cliente(logger, "MEMORIA", datos_conexion->ip_memoria, datos_conexion->puerto_memoria);

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

    while (cliente_socket != -1)
    {
        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            log_info(logger, "DISCONNECT!");
            return;
        }

        switch (cop)
        {
        case DEBUG:
            log_info(logger, "debug");
            break;

        case ENVIAR_PCB:

            if (recv_pcb(cliente_socket, &running))
            {
                log_info(logger, "Se recibio el PCB");
                printf("copie el pcb en running\n");
                ciclo_instruccion(running, cliente_socket, logger);
            }

            break;

        case INT_NUEVO_READY:

            log_info(logger, "Desalojando proceso");
            // ACA VA EL CODIGO PARA INTERRUMPIR EL PROCESO QUE ESTA EJECUTANDO ACTUALMENTE
            running = NULL;
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

void ciclo_instruccion(t_pcb *running, uint32_t cliente_socket, t_log *logger)
{
    t_list *lista_instrucciones = running->instrucciones;
    uint32_t cantidad_instrucciones = list_size(lista_instrucciones);
    INSTRUCCIONES_EJECUCION instruccion_actual_enum;
    t_instruccion *instruccion_actual;
    float retardo;
    float segundos;
    int i;
    uint32_t cantidad_noops = 0;
    t_argumento *tiempo_bloqueo;
    t_argumento *argumentos;
    uint32_t j;
    while ((running->program_counter < cantidad_instrucciones) && (running != NULL))
    {
        instruccion_actual = list_get(running->instrucciones, running->program_counter);
        instruccion_actual_enum = enumerar_instruccion(instruccion_actual);
        argumentos = instruccion_actual->argumentos;

        printf("Antes del switch con la instruccion: %d\n", instruccion_actual_enum);
        switch (instruccion_actual_enum)
        {
        case NO_OP:
            retardo = configuracion_cpu->retardo_noop;
            segundos = retardo / 1000;
            sleep(segundos);
            running->program_counter++;
            break;

        case I_O:
            tiempo_bloqueo = list_get(instruccion_actual->argumentos, 0);
            running->program_counter++;
            send_pcb_con_tiempo_bloqueado(cliente_socket, running, tiempo_bloqueo->argumento);
            //running = NULL;
            break;

        case READ:
            running->program_counter++;
            break;

        case WRITE:
            running->program_counter++;
            break;

        case COPY:
            running->program_counter++;
            break;

        case EXIT:
            running->program_counter++;
            break;
        }

        // chequearInterrupciones();
    }
}

INSTRUCCIONES_EJECUCION enumerar_instruccion(t_instruccion *instruccion)
{
    char *identificador = instruccion->identificador;
    if (!strcmp(identificador, "NO_OP"))
    {
        return NO_OP;
    }
    else if (!strcmp(identificador, "I/O"))
    {
        return I_O;
    }
    else if (!strcmp(identificador, "READ"))
    {
        return READ;
    }
    else if (!strcmp(identificador, "WRITE"))
    {
        return WRITE;
    }
    else if (!strcmp(identificador, "COPY"))
    {
        return COPY;
    }
    else if (!strcmp(identificador, "EXIT"))
    {
        return EXIT;
    }
    return ERROR;
}