#include "../include/consola.h"

void iniciar_consola(int tamanio, char* path){
    t_list* lista_instrucciones = obtener_instrucciones(path);
    t_config_consola* datos_conexion = leer_configuracion();

    t_log* logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    int conexion = crear_conexion_consola(datos_conexion, logger);

    send_aprobar_operativos(conexion, 7, 8);
    // t_paquete* paquete = crear_paquete();

    // Ahora tenemos que hacer un agregar_a_paquete(paquete, (un struct que tenga las instrucciones y el tamanio a la vez), longitud del mensaje)
    printf("Estoy aca %d\n", conexion);
    liberar_conexion(conexion);
    

    /*enviar_info_al_kernel(tamanio, lista_instrucciones);
    */
    log_destroy(logger);
    list_destroy_and_destroy_elements(lista_instrucciones, (void*) destruir_instruccion);
    liberar_estructura_datos(datos_conexion);
}

t_list* obtener_instrucciones(char* path){
    FILE* archivo = fopen(path, "r");
    t_list* lista_instrucciones = list_create();

    if (archivo == NULL){
        printf("Error: No se pudo abrir el archivo.\n");
        exit(EXIT_FAILURE);
    }

    //leer linea por linea del archivo
    char* linea = NULL;
    size_t capacidad = 0;
    ssize_t read;

    while((read = getline(&linea, &capacidad, archivo)) != -1){
        t_instruccion* instruccion = crear_instruccion(linea);
        list_add(lista_instrucciones, instruccion);
    }
    free(linea);
    fclose(archivo);
    return lista_instrucciones;
}

t_instruccion* crear_instruccion(char* instruccion){
    t_instruccion* instruccion_nueva = malloc(sizeof(t_instruccion)); // creo una instruccion
    instruccion_nueva->instruccion = malloc(sizeof(char) * (strlen(instruccion) + 1)); // le asigno memoria para la instruccion (para el char en particular)
    strcpy(instruccion_nueva->instruccion, instruccion); // copio el texto en la instruccion
    return instruccion_nueva;
}

void destruir_instruccion(t_instruccion* instruccion){
    free(instruccion->instruccion);
    free(instruccion);
}



