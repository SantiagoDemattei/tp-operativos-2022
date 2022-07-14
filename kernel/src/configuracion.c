#include "../include/configuracion.h"

t_configuracion_kernel *leer_configuracion()
{

    t_config *nuevo_config; // revisar struct (no importa el de commons)
    char* mensaje;
    nuevo_config = config_create("./kernel.config");
    mensaje = string_from_format("Error: No se pudo abrir el archivo de configuracion de kernel \n");
    if (nuevo_config == NULL)
    {
        loggear_error(logger, mensaje, mutex_logger_kernel);
        free(mensaje);
        exit(EXIT_FAILURE);
    }
    free(mensaje);

    char *ip_memoria = config_get_string_value(nuevo_config, "IP_MEMORIA");                           // leo ip de memoria
    char *ip_kernel = config_get_string_value(nuevo_config, "IP_KERNEL");                             // leo ip de memoria
    char *puerto_memoria = config_get_string_value(nuevo_config, "PUERTO_MEMORIA");                   // leo puerto
    char *ip_cpu = config_get_string_value(nuevo_config, "IP_CPU");                                   // leo ip de cpu
    char *puerto_cpu_dispatch = config_get_string_value(nuevo_config, "PUERTO_CPU_DISPATCH");         // leo puerto de cpu dispatch
    char *puerto_cpu_interrupt = config_get_string_value(nuevo_config, "PUERTO_CPU_INTERRUPT");       // leo puerto de cpu interrupt
    char *puerto_escucha = config_get_string_value(nuevo_config, "PUERTO_ESCUCHA");                   // leo puerto de escucha
    char *algoritmo_planificacion = config_get_string_value(nuevo_config, "ALGORITMO_PLANIFICACION"); // leo algoritmo de planificacion
    char *estimacion_inicial = config_get_string_value(nuevo_config, "ESTIMACION_INICIAL");           // leo estimacion inicial
    char *alfa = config_get_string_value(nuevo_config, "ALFA");                                       // leo alfa
    uint32_t grado_multiprogramacion = config_get_int_value(nuevo_config, "GRADO_MULTIPROGRAMACION"); // leo grado multiprogramacion
    char *tiempo_maximo_bloqueado = config_get_string_value(nuevo_config, "TIEMPO_MAXIMO_BLOQUEADO"); // leo tiempo maximo bloqueado

    t_configuracion_kernel *datos = malloc(sizeof(t_configuracion_kernel)); // creo estructura de datos de conexion
    datos->ip_memoria = malloc(strlen(ip_memoria) + 1);
    strcpy(datos->ip_memoria, ip_memoria);
    datos->ip_kernel = malloc(strlen(ip_kernel) + 1);
    strcpy(datos->ip_kernel, ip_kernel);
    datos->puerto_memoria = malloc(strlen(puerto_memoria) + 1);
    strcpy(datos->puerto_memoria, puerto_memoria);
    datos->ip_cpu = malloc(strlen(ip_cpu) + 1);
    strcpy(datos->ip_cpu, ip_cpu);
    datos->puerto_cpu_dispatch = malloc(strlen(puerto_cpu_dispatch) + 1);
    strcpy(datos->puerto_cpu_dispatch, puerto_cpu_dispatch);
    datos->puerto_cpu_interrupt = malloc(strlen(puerto_cpu_interrupt) + 1);
    strcpy(datos->puerto_cpu_interrupt, puerto_cpu_interrupt);
    datos->puerto_escucha = malloc(strlen(puerto_escucha) + 1);
    strcpy(datos->puerto_escucha, puerto_escucha);
    datos->algoritmo_planificacion = malloc(strlen(algoritmo_planificacion) + 1);
    strcpy(datos->algoritmo_planificacion, algoritmo_planificacion);
    datos->estimacion_inicial = malloc(strlen(estimacion_inicial) + 1);
    strcpy(datos->estimacion_inicial, estimacion_inicial);
    datos->alfa = malloc(strlen(alfa) + 1);
    strcpy(datos->alfa, alfa);
    datos->grado_multiprogramacion = grado_multiprogramacion;
    datos->tiempo_maximo_bloqueado = malloc(strlen(tiempo_maximo_bloqueado) + 1);
    strcpy(datos->tiempo_maximo_bloqueado, tiempo_maximo_bloqueado);

    config_destroy(nuevo_config); // libero la memoria del config
    return datos;
}

void inicializar_semaforos()
{
    pthread_mutex_init(&mutex_cantidad_procesos, NULL);
    pthread_mutex_init(&mutex_estado_running, NULL);
    pthread_mutex_init(&mutex_logger_kernel, NULL);
    pthread_mutex_init(&mutex_cola_new, NULL);
    pthread_mutex_init(&mutex_cola_ready, NULL);
    pthread_mutex_init(&mutex_cola_blocked, NULL);
    pthread_mutex_init(&mutex_cola_exit, NULL);
    pthread_mutex_init(&mutex_cola_ready_suspendido, NULL);
    pthread_mutex_init(&mutex_variable_contador, NULL);
    pthread_mutex_init(&mutex_info_desalojado, NULL);
    sem_init(&sem_planificar, 0, 0);
    sem_init(&sem_nuevo_ready, 0, 0);
    sem_init(&sem_nuevo_bloqued, 0, 0);
    sem_init(&sem_recibir, 0, 0);
    sem_init(&sem_running, 0, 1);
    sem_init(&sem_desalojo, 0, 0);
    sem_init(&sem_esperar_confirmacion, 0, 0);
    sem_init(&sem_queue_suspended, 0, 0);
    // sem_init(&finalizar_planificar, 0, 0);
}
