#include "../include/configuracion.h"


t_configuracion_cpu* leer_configuracion(t_log* logger) {

    t_config* nuevo_config; // revisar struct (no importa el de commons)
    nuevo_config = config_create("./cpu.config");
    if(nuevo_config == NULL){
        log_info(logger, "Error: No se pudo abrir el archivo de configuracion de la cpu");
        exit(EXIT_FAILURE);
    }

    uint32_t entradas_tlb = config_get_int_value(nuevo_config, "ENTRADAS_TLB");
    char* reemplazo_tlb = config_get_string_value(nuevo_config, "REEMPLAZO_TLB");
    float retardo_noop = config_get_int_value(nuevo_config, "RETARDO_NOOP");
    char* ip_memoria = config_get_string_value(nuevo_config, "IP_MEMORIA");
    char* ip_cpu = config_get_string_value(nuevo_config, "IP_CPU");
    char* puerto_memoria = config_get_string_value(nuevo_config, "PUERTO_MEMORIA");
    char* puerto_escucha_dispatch = config_get_string_value(nuevo_config, "PUERTO_ESCUCHA_DISPATCH");
    char* puerto_escucha_interrupt = config_get_string_value(nuevo_config, "PUERTO_ESCUCHA_INTERRUPT");

    t_configuracion_cpu* datos = malloc(sizeof(t_configuracion_cpu)); // creo estructura de datos de configuracion
    datos->entradas_tlb = entradas_tlb; // copio la entrada del tlb
    datos->reemplazo_tlb = malloc(sizeof(char) * (strlen(reemplazo_tlb) + 1)); // le asigno memoria para el reemplazo
    strcpy(datos->reemplazo_tlb, reemplazo_tlb); // copio el reemplazo
    datos->retardo_noop = retardo_noop; // le asigno el retardo
    datos->ip_memoria = malloc(sizeof(char) * (strlen(ip_memoria) + 1)); // le asigno memoria para la ip
    strcpy(datos->ip_memoria, ip_memoria); // copio la ip
    datos->ip_cpu = malloc(sizeof(char) * (strlen(ip_cpu) + 1)); // le asigno memoria para la ip
    strcpy(datos->ip_cpu, ip_cpu); // copio la ip
    datos->puerto_memoria = malloc(sizeof(char) * (strlen(puerto_memoria) + 1)); // le asigno el puerto
    strcpy(datos->puerto_memoria,puerto_memoria); 
    datos->puerto_escucha_dispatch = malloc(sizeof(char) * (strlen(puerto_escucha_dispatch) + 1)); // le asigno el puerto
    strcpy(datos->puerto_escucha_dispatch, puerto_escucha_dispatch);
    datos->puerto_escucha_interrupt = malloc(sizeof(char) * (strlen(puerto_escucha_interrupt) + 1)); // le asigno el puerto
    strcpy(datos->puerto_escucha_interrupt, puerto_escucha_interrupt);


    config_destroy(nuevo_config); // libero la memoria del config
    return datos;
    
}

void liberar_estructura_datos(t_configuracion_cpu* datos){
    free(datos->reemplazo_tlb);
    free(datos->ip_memoria);
    free(datos->ip_cpu);
    free(datos->puerto_memoria);
    free(datos->puerto_escucha_dispatch);
    free(datos->puerto_escucha_interrupt);
    free(datos);
}

void inicializar_semaforos(){
    pthread_mutex_init(&mutex_logger_cpu, NULL);
    pthread_mutex_init(&mutex_running_cpu, NULL);
    pthread_mutex_init(&mutex_interrupcion, NULL);
    pthread_mutex_init(&mutex_tlb, NULL);
}