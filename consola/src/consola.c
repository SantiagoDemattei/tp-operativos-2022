#include "../include/consola.h"

uint32_t iniciar_consola(uint32_t tamanio, char *path, t_log *logger)
{

    t_list *lista_instrucciones = obtener_instrucciones(path, logger);
    t_configuracion_consola *datos_conexion = leer_configuracion(logger);
    loggear_lista_instrucciones(lista_instrucciones, logger);

    uint32_t conexion = crear_conexion_consola(datos_conexion, logger);

    send_iniciar_consola(conexion, lista_instrucciones, tamanio);

    log_info(logger, "Se envio la lista de instrucciones al kernel");

    list_destroy_and_destroy_elements(lista_instrucciones, (void *)destruir_instruccion);
    liberar_estructura_datos(datos_conexion);

    return conexion;
}

t_list *obtener_instrucciones(char *path, t_log *logger)
{
    FILE *archivo = fopen(path, "r");
    t_list *lista_instrucciones = list_create();
    t_list *lista_instrucciones_aux = list_create();
    if (archivo == NULL)
    {
        log_info(logger, "Error: No se pudo abrir el archivo de instrucciones.");
        exit(EXIT_FAILURE);
    }

    // leer linea por linea del archivo
    char *linea = NULL;
    size_t capacidad = 0;
    ssize_t read;

    while ((read = getline(&linea, &capacidad, archivo)) != -1)
    {
        t_instruccion *instruccion = crear_instruccion(linea, logger);
        list_add(lista_instrucciones_aux, instruccion);
    }

    int incremento = 0;
    int i = 0;
    t_instruccion *instruccion;  
    int incremento_aux = 0;
    for (i = 0; i < (list_size(lista_instrucciones_aux)); i++)
    {
        instruccion = list_get(lista_instrucciones_aux, i);
        if (strcmp(instruccion->identificador, "NO_OP") == 0)
        {   
            t_argumento *argumento = instruccion->argumentos;
            incremento_aux = list_get(argumento->argumento, 0);
            incremento += (incremento_aux);
            while(incremento_aux > 0){
                t_instruccion *copia = malloc(sizeof(t_instruccion));   
                copia->identificador = malloc(strlen("NO_OP") + 1);
                strcpy(copia->identificador, "NO_OP");
                copia->argumentos = list_create();
                list_add(lista_instrucciones, copia);
                incremento_aux--;
            }
        }
        else
        {
            list_add(lista_instrucciones, instruccion);
        }
    }              

    free(linea);
    fclose(archivo);
    return lista_instrucciones;
}

t_instruccion *crear_instruccion(char *instruccion, t_log *logger)
{
    t_instruccion *instruccion_nueva = malloc(sizeof(t_instruccion)); // creo una instruccion
    t_list *lista_argumentos = list_create();
    char *token = strtok(instruccion, " ");
    if (token == NULL)
    {
        log_info(logger, "Error: No se pudo leer la instruccion.");
        exit(EXIT_FAILURE);
    }
    instruccion_nueva->identificador = malloc(strlen(token) + 1);
    strcpy(instruccion_nueva->identificador, token);
    while (token != NULL)
    {
        token = strtok(NULL, " "); // pongo null para no volver a leer el mismo token
        if (token != NULL)
        {
            // insertar token al final de la lista de argumentos
            t_argumento *argumento = malloc(sizeof(t_argumento));
            argumento->argumento = atoi(token);
            list_add(lista_argumentos, argumento);
        }
    }
    instruccion_nueva->argumentos = lista_argumentos;
    return instruccion_nueva;
}
