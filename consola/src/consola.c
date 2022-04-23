#include "../include/consola.h"

void iniciar_consola(int tamanio, char* path){
    t_log* logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    t_list* lista_instrucciones = obtener_instrucciones(path, logger);
    t_config_consola* datos_conexion = leer_configuracion();
    mostrar_lista_instrucciones(lista_instrucciones, logger);

    int conexion = crear_conexion_consola(datos_conexion, logger);


    send_aprobar_operativos(conexion, 7, 8); // ES PARA PRUEBA, TENEMOS QUE CREAR UN OP_CODE Y MANDAR LA LISTA DE INSTRUCCIONES CON UNA ACCION ASOCIADA A ESE OP_CODE

    printf("Estoy aca %d\n", conexion);
    liberar_conexion(conexion);
    

    /*enviar_info_al_kernel(tamanio, lista_instrucciones);
    */
    log_destroy(logger);
    list_destroy_and_destroy_elements(lista_instrucciones, (void*) destruir_instruccion);
    liberar_estructura_datos(datos_conexion);
}

t_list* obtener_instrucciones(char* path, t_log* logger){
    FILE* archivo = fopen(path, "r");
    t_list* lista_instrucciones = list_create();

    if (archivo == NULL){
        printf("Error: No se pudo abrir el archivo de instrucciones.\n");
        //loggear error
        log_info(logger, "Error: No se pudo abrir el archivo de instrucciones.");
        exit(EXIT_FAILURE);
    }

    //leer linea por linea del archivo
    char* linea = NULL;
    size_t capacidad = 0;
    ssize_t read;

    while((read = getline(&linea, &capacidad, archivo)) != -1){
        t_instruccion* instruccion = crear_instruccion(linea, logger);
        list_add(lista_instrucciones, instruccion);
    }
    free(linea);
    fclose(archivo);
    return lista_instrucciones;
}

t_instruccion* crear_instruccion(char* instruccion, t_log* logger){
    t_instruccion* instruccion_nueva = malloc(sizeof(t_instruccion)); // creo una instruccion
    t_list* lista_argumentos = list_create();
    char* token = strtok(instruccion, " ");
    if(token == NULL){
        printf("Error: No se pudo leer la instruccion.\n");
        log_info(logger, "Error: No se pudo leer la instruccion.");
        exit(EXIT_FAILURE);
    }
    instruccion_nueva->instruccion = malloc(strlen(token) + 1);
    strcpy(instruccion_nueva->instruccion, token);
    while(token != NULL){
        token = strtok(NULL, " "); // pongo null para no volver a leer el mismo token
        if(token != NULL){
            //insertar token al final de la lista de argumentos
            t_argumento* argumento = malloc(sizeof(t_argumento));
            argumento->argumento = atoi(token);
            list_add(lista_argumentos, argumento);
        }
    }
    instruccion_nueva->argumentos = lista_argumentos;
    return instruccion_nueva;
    /*
    instruccion_nueva->instruccion = malloc(sizeof(char) * (strlen(instruccion) + 1)); // le asigno memoria para la instruccion (para el char en particular)
    strcpy(instruccion_nueva->instruccion, instruccion); // copio el texto en la instruccion
    return instruccion_nueva; 
    */
}

void destruir_argumentos(t_argumento* argumento){
    free(argumento);
}

void destruir_instruccion(t_instruccion* instruccionS){
    free(instruccionS->instruccion);
    list_destroy_and_destroy_elements(instruccionS->argumentos, (void*) destruir_argumentos);
    free(instruccionS);
}

void mostrar_lista_instrucciones(t_list* lista_instrucciones, t_log* logger){
    int i;
    log_info(logger, "Lista de instrucciones cargadas:\n");
    for(i = 0; i < list_size(lista_instrucciones); i++){
        t_instruccion* instruccion = list_get(lista_instrucciones, i);
        //loggear
        log_info(logger, "Instruccion: %s", instruccion->instruccion);
        int j;
        for(j = 0; j < list_size(instruccion->argumentos); j++){
            t_argumento* argumento = list_get(instruccion->argumentos, j);
            //loggear
            log_info(logger, "Argumento: %d", argumento->argumento);
        }
        log_info(logger, "\n"); // Meto un \n para separar las instrucciones
    }
}

