#include "../include/configuracion.h"


t_datos_conexion* leer_configuracion(){

    t_config* nuevo_config; // revisar struct (no importa el de commons)
    nuevo_config = config_create("./consola.config");
    if(nuevo_config == NULL){
        printf("Error: No se pudo abrir el archivo de configuracion.\n");
        exit(EXIT_FAILURE);
    }

    char* ip_kernel = config_get_string_value(nuevo_config, "IP_KERNEL"); // leo ip
    char* puerto_kernel = config_get_string_value(nuevo_config, "PUERTO_KERNEL"); // leo puerto
    
    t_datos_conexion* datos = malloc(sizeof(t_datos_conexion)); // creo estructura de datos de conexion
    datos->ip = malloc(sizeof(char) * (strlen(ip_kernel) + 1)); // le asigno memoria para la ip
    datos->puerto = malloc(sizeof(char) * (strlen(puerto_kernel) + 1)); // le asigno memoria para el puerto
    strcpy(datos->ip, ip_kernel); // copio la ip
    strcpy(datos->puerto, puerto_kernel); // copio el puerto 

    printf("%s\n",datos->ip); // LOS DATOS SE GUARDAN BIEN!!!
    printf("%s\n",datos->puerto); // LOS DATOS SE GUARDAN BIEN!!!

    config_destroy(nuevo_config); // libero la memoria del config
    return datos;
    
}

void liberar_estructura_datos(t_datos_conexion* datos){
    free(datos->ip);
    free(datos->puerto);
    free(datos);
}
/*
// Creamos una conexión hacia el servidor
	conexion = crear_conexion(ip, puerto);

	// Enviamos al servidor el valor de CLAVE como mensaje

	enviar_mensaje(valor, conexion);
// Armamos y enviamos el paquete
	paquete(conexion);
terminar_programa(conexion, logger, config);


void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete;

	paquete = crear_paquete();

	// Leemos y esta vez agregamos las lineas al paquete

	leido = readline("> ");
    while(strcmp(leido, "") != 0){
    	agregar_a_paquete(paquete, leido, strlen(leido) + 1);
    	leido = readline("> ");
    };

    enviar_paquete(paquete, conexion);

	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	
    free(1);
    eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
		  con las funciones de las commons y del TP mencionadas en el enunciado */

//	log_destroy(logger);
//	config_destroy(config);

//}











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