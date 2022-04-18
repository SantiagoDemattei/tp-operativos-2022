#include "../include/configuracion.h"


void cargar_configuracion(){

    t_config* nuevo_config; // revisar struct (no importa el de commons)
    nuevo_config = config_create("consola.config");
    if(nuevo_config == NULL){
        printf("Error: No se pudo abrir el archivo de configuracion.\n");
        exit(EXIT_FAILURE);
    }
    char* ip_kernel = config_get_string_value(nuevo_config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(nuevo_config, "PUERTO_KERNEL");



}










/*
	// ---------------- ARCHIVOS DE CONFIGURACION ---------------- 

    int conexion;
	char* ip;
	char* puerto;
	char* valor;

	config = iniciar_config();

	// Usando el config creado previamente, leemos los valores del config y los 
	// dejamos en las variables 'ip', 'puerto' y 'valor'

	valor = config_get_string_value(config, "CLAVE");

	// Loggeamos el valor de config

	log_info(logger, valor);

	// Obtengo ip y puertos

	ip = config_get_string_value(config, "IP_KERNEL");
	puerto = config_get_string_value(config, "PUERTO_KERNEL");
    
*/