#include "../include/comunicacion.h"

uint32_t crear_comunicacion_dispatch(t_configuracion_cpu *t_configuracion_cpu, t_log *logger)
{ // funcion de servidor

    uint32_t socket_cpu_dispatch = iniciar_servidor(logger, "KERNEL DISPATCH", t_configuracion_cpu->ip_cpu, t_configuracion_cpu->puerto_escucha_dispatch);

    if (socket_cpu_dispatch == -1)
    {
        pthread_mutex_lock(&mutex_logger_cpu);
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        pthread_mutex_unlock(&mutex_logger_cpu);
        return -1;
    }
    return socket_cpu_dispatch;
}

uint32_t crear_comunicacion_interrupt(t_configuracion_cpu *t_configuracion_cpu, t_log *logger)
{ // funcion de servidor

    uint32_t socket_cpu_interrupt = iniciar_servidor(logger, "KERNEL INTERRUPT", t_configuracion_cpu->ip_cpu, t_configuracion_cpu->puerto_escucha_interrupt);

    if (socket_cpu_interrupt == -1)
    {
        pthread_mutex_lock(&mutex_logger_cpu);
        log_error(logger, "No se pudo iniciar el servidor de comunicacion");
        pthread_mutex_unlock(&mutex_logger_cpu);
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
    uint32_t *cliente_socket = args->fd; // el cliente socket puede ser interrupt o dispatch
    char *server_name = args->server_name;
    free(args);

    op_code cop;

    while (*cliente_socket != -1)
    {
        if (recv(*cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code))
        {
            loggear_info(logger, "DISCONNECT", mutex_logger_cpu);
            return;
        }

        switch (cop)
        {
        case DEBUG:
            pthread_mutex_lock(&mutex_logger_cpu);
            log_info(logger, "debug");
            pthread_mutex_unlock(&mutex_logger_cpu);
            break;

        case ENVIAR_PCB: // recibir PCB del kernel para poner a ejecutar

            if (recv_pcb(*cliente_socket, &running)) // en running guardo el pcb que va a ejecutar
            {
                loggear_info(logger, "Se recibio el pcb para ejecutar\n", mutex_logger_cpu);
                pthread_mutex_lock(&mutex_interrupcion);
                interrupciones = false; // interrupciones desactivadas para chequearlas cuando termine de ejecutar una instruccion
                pthread_mutex_unlock(&mutex_interrupcion);
                ciclo_instruccion(cliente_socket, logger); // cuando la cpu recibe el pcb simula un ciclo de instruccion
            }
            break;

        case INT_NUEVO_READY: // solo se usa para interrumpir (si llega aca vino por el socket_interrupt)
            loggear_info(logger, "Interrumpiendo proceso", mutex_logger_cpu);

            pthread_mutex_lock(&mutex_interrupcion);
            interrupciones = true; // interrupciones activadas para chequearlas cuando termine de ejecutar una instruccion
            pthread_mutex_unlock(&mutex_interrupcion);

            break;

        // Errores
        case -1:
            pthread_mutex_lock(&mutex_logger_cpu);
            log_error(logger, "Cliente desconectado de %s...", server_name);
            pthread_mutex_unlock(&mutex_logger_cpu);
            return;
        default:
            pthread_mutex_lock(&mutex_logger_cpu);
            log_error(logger, "Algo anduvo mal en el server de %s", server_name);
            pthread_mutex_unlock(&mutex_logger_cpu);
            return;
        }
    }
    pthread_mutex_lock(&mutex_logger_cpu);
    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    pthread_mutex_unlock(&mutex_logger_cpu);
    return;
}

uint32_t server_escuchar(t_log *logger, char *server_name, uint32_t server_socket) // hilos al pedo
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

void ciclo_instruccion(uint32_t *cliente_socket, t_log *logger)
{
    t_list *lista_instrucciones = running->instrucciones; // lista de instrucciones del proceso que esta en running
    uint32_t cantidad_instrucciones = list_size(lista_instrucciones);
    INSTRUCCIONES_EJECUCION instruccion_actual_enum;
    t_instruccion *instruccion_actual;
    float retardo;
    t_argumento *tiempo_bloqueo1;
    t_argumento *argumentos;
    t_argumento *direccion_logica;
    uint32_t marco;
    t_argumento *valor;
    pthread_mutex_lock(&mutex_running_cpu);
    while ((running != NULL) && (running->program_counter < cantidad_instrucciones)) // recorro tomando como punto de partida la instrucción que indique el Program Counter del PCB recibido -> FETCH
    {
        t_direccion_fisica *direccion_fisica;
        pthread_mutex_unlock(&mutex_running_cpu);
        instruccion_actual = list_get(running->instrucciones, running->program_counter); // tomo la instruccion actual
        instruccion_actual_enum = enumerar_instruccion(instruccion_actual);
        argumentos = instruccion_actual->argumentos;

        pthread_mutex_lock(&mutex_logger_cpu);
        log_info(logger, "Antes del switch con la instruccion: %s\n", instruccion_actual->identificador);
        pthread_mutex_unlock(&mutex_logger_cpu);

        switch (instruccion_actual_enum)
        {
        case NO_OP: // DECODE + EXECUTE
            retardo = configuracion_cpu->retardo_noop;
            usleep(retardo * 1000); // espera un tiempo determinado
            pthread_mutex_lock(&mutex_running_cpu);
            running->program_counter++; // avanza a la prox instruccion
            pthread_mutex_unlock(&mutex_running_cpu);
            break;

        case I_O: // DECODE + EXECUTE
            tiempo_bloqueo1 = list_get(argumentos, 0);
            pthread_mutex_lock(&mutex_running_cpu);
            running->tiempo_bloqueo = tiempo_bloqueo1->argumento; // en el pcb me guardo el tiempo de bloqueo
            running->program_counter++;                           // avanzo el program counter
            send_pcb(*cliente_socket, running, BLOQUEO_IO);       // mando el pcb para que lo reciba el kernel y bloquee al pcb
            destruir_pcb(running);
            running = NULL; // proceso bloqueado por I/O -> en running no hay nadie
            pthread_mutex_unlock(&mutex_running_cpu);
            break;

        case READ:
            pthread_mutex_lock(&mutex_running_cpu);

            running->program_counter++;
            pthread_mutex_unlock(&mutex_running_cpu);
            break;

        case WRITE:
            direccion_logica = list_get(argumentos, 0);
            direccion_fisica = calcular_mmu(direccion_logica);
            printf("el numero de pagina es: %d\n", direccion_fisica->numero_pagina);
            printf("la entrada de primer nivel es: %d\n", direccion_fisica->entrada_tabla_1er_nivel);
            printf("la entrada de segundo nivel es: %d\n", direccion_fisica->entrada_tabla_2do_nivel);
            printf("El desplazamiento es: %d\n", direccion_fisica->desplazamiento);

            valor = list_get(argumentos, 1);
            marco = buscar(direccion_fisica->numero_pagina);
            if (marco != -1)
            {                                                                       // TLB HIT
                socket_memoria = crear_conexion_memoria(configuracion_cpu, logger); // creo la conexion con la memoria
                // send a memoria de la pagina, el marco, y el valor a escribir
            }
            else
            {                                                                                               // TLB MISS
                socket_memoria = crear_conexion_memoria(configuracion_cpu, logger);                         // creo la conexion con la memoria
                send_valor_y_num_pagina(socket_memoria, direccion_fisica->numero_pagina, valor->argumento); 
                // TODO: HACER EL SEND DESDE MEMORIA
                recv_valor_tb(socket_memoria, &marco); 
                // agrego una nueva entrada en la tlb con el nro de pagina y el marco
                t_tlb *entrada_tlb = malloc(sizeof(t_tlb));
                entrada_tlb->pagina = direccion_fisica->numero_pagina;
                entrada_tlb->marco = marco;
                agregar(entrada_tlb);

                pthread_mutex_lock(&mutex_running_cpu);
                running->program_counter--;
                pthread_mutex_unlock(&mutex_running_cpu);
            }
            pthread_mutex_lock(&mutex_running_cpu);
            running->program_counter++;
            pthread_mutex_unlock(&mutex_running_cpu);

            free(direccion_fisica);

            break;

        case COPY:
            pthread_mutex_lock(&mutex_running_cpu);
            running->program_counter++;
            pthread_mutex_unlock(&mutex_running_cpu);
            break;

        case EXIT:
            pthread_mutex_lock(&mutex_running_cpu);
            send_pcb(*cliente_socket, running, ENVIAR_PCB);
            running->program_counter++;
            running = NULL;
            pthread_mutex_unlock(&mutex_running_cpu);
            break;

        case ERROR:
            pthread_mutex_lock(&mutex_running_cpu);
            running = NULL;
            pthread_mutex_unlock(&mutex_running_cpu);

            pthread_mutex_lock(&mutex_logger_cpu);
            log_error(logger, "Error en la instruccion");
            pthread_mutex_unlock(&mutex_logger_cpu);

            break;
        }
        chequear_interrupciones(cliente_socket); // cuando termina de ejecutar una instruccion chequeo si hay interrupciones
        pthread_mutex_lock(&mutex_running_cpu);
    }
    pthread_mutex_unlock(&mutex_running_cpu);
}

void chequear_interrupciones(uint32_t *cliente_socket)
{
    pthread_mutex_lock(&mutex_interrupcion);
    if (interrupciones)
    { // si hay interrupciones hay que desalojar un proceso
        pthread_mutex_unlock(&mutex_interrupcion);

        pthread_mutex_lock(&mutex_running_cpu);
        send_pcb(*cliente_socket, running, INTERRUPCION); // desalojo el pcb y mando el pcb para que lo reciba el kernel
        pthread_mutex_unlock(&mutex_running_cpu);

        printf("Proceso %d interrumpido", running->id);
        destruir_pcb(running);

        pthread_mutex_lock(&mutex_running_cpu);
        running = NULL; // desalojo el pcb
        pthread_mutex_unlock(&mutex_running_cpu);

        pthread_mutex_lock(&mutex_interrupcion);
    }
    pthread_mutex_unlock(&mutex_interrupcion);
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

t_direccion_fisica *calcular_mmu(t_argumento *direc_logica)
{
    printf("calculando direccion fisica\n");
    t_direccion_fisica *direccion_fisica = malloc(sizeof(t_direccion_fisica));
    printf("calculando direccion fisica 2\n");
    direccion_fisica->numero_pagina = (uint32_t)floor((direc_logica->argumento) / tamanio_pagina); // calculo el numero de pagina donde voy a escribir el dato
    direccion_fisica->entrada_tabla_1er_nivel = (uint32_t)floor(direccion_fisica->numero_pagina / cant_entradas_por_tabla);
    direccion_fisica->entrada_tabla_2do_nivel = direccion_fisica->numero_pagina % cant_entradas_por_tabla;
    direccion_fisica->desplazamiento = direc_logica->argumento - (direccion_fisica->numero_pagina * tamanio_pagina);

    return direccion_fisica;
}
