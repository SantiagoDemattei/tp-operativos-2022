#include "../include/configuracion.h"


t_configuracion_consola* leer_configuracion(t_log* logger) { 

    t_config* nuevo_config; // revisar struct (no importa el de commons)
    nuevo_config = config_create("./consola.config");
    if(nuevo_config == NULL){
        log_info(logger, "Error: No se pudo abrir el archivo de configuracion de consola");
        exit(EXIT_FAILURE);
    }

    char* ip_kernel = config_get_string_value(nuevo_config, "IP_KERNEL"); // leo ip
    char* puerto_kernel = config_get_string_value(nuevo_config, "PUERTO_KERNEL"); // leo puerto

    t_configuracion_consola* datos = malloc(sizeof(t_configuracion_consola)); // creo estructura de datos de configuracion
    datos->ip = malloc(sizeof(char) * (strlen(ip_kernel) + 1)); // le asigno memoria para la ip
    strcpy(datos->ip, ip_kernel); // copio la ip
    datos->puerto = malloc(sizeof(char) * (strlen(puerto_kernel) + 1)); // le asigno memoria para el puerto
    strcpy(datos->puerto,puerto_kernel); //copio el puerto
    config_destroy(nuevo_config); // libero la memoria del config
    return datos;
    
}

void liberar_estructura_datos(t_configuracion_consola* datos){
    free(datos->ip);
    free(datos->puerto);
    free(datos);
}
