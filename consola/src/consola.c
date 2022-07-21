#include "../include/consola.h"

uint32_t iniciar_consola(uint32_t tamanio, char *path, char* path_config, t_log *logger) 
{
    op_code cop;
    t_list *lista_instrucciones = obtener_instrucciones(path, logger); //lista con instrucciones del archivo 
    t_configuracion_consola *datos_conexion = leer_configuracion(path_config, logger); //datos del servidor (kernel en este caso) al que queremos que se conecte
    loggear_lista_instrucciones(lista_instrucciones, logger); 

    uint32_t conexion = crear_conexion_consola(datos_conexion, logger); //numero de socket del servidor (kernel) al que se conecto 

    send_iniciar_consola(conexion, lista_instrucciones, tamanio); //una vez que se conecta con el server le manda la lista de instrucciones y el tamaño del proceso

    log_info(logger, "Se envio la lista de instrucciones al kernel");

    log_info(logger, "Estoy esperando a que el kernel me avise que ya ejecuto mis instrucciones...");

    if(recv(conexion, &cop, sizeof(op_code), 0) != sizeof(op_code)) //se queda esperando a que el kernel le avise que se ejecutaron las instrucciones
    {
        log_error(logger, "No se pudo recibir el codigo de operacion");
        return -1;
    }
    log_info(logger, "El kernel me aviso que ya ejecute mis instrucciones :D");
    liberar_conexion(conexion);
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
        t_instruccion *instruccion = crear_instruccion(linea, logger); //crea la instruccion con su identificador y sus argumentos
        list_add(lista_instrucciones_aux, instruccion); 
    } //tenemos la lista completa con todas las instrucciones del proceso

   // int incremento = 0;
    int i = 0;
    int j = 0;
    t_instruccion *instruccion;
    t_argumento* argumento1;
    
    int incremento_aux = 0;
    for (i = 0; i < (list_size(lista_instrucciones_aux)); i++) //itera por cada instruccion
    {
        instruccion = list_get(lista_instrucciones_aux, i); //tomo instruccion de la lista 
        if (strcmp(instruccion->identificador, "NO_OP") == 0) //si la instruccion es NO_OP 
        {   
            t_list *lista_argumentos = instruccion->argumentos;
            argumento1 = list_get(lista_argumentos, 0);
            incremento_aux = argumento1->argumento; //tomo el argumento de la instruccion NO_OP
          // incremento += (incremento_aux); 
            while(incremento_aux > 0){  // agrega la instruccion n veces a la lista segun el argumento
                t_instruccion *copia = malloc(sizeof(t_instruccion));   
                copia->identificador = malloc(strlen("NO_OP") + 1);
                strcpy(copia->identificador, "NO_OP");
                copia->argumentos = list_create(); 
                list_add(lista_instrucciones, copia);
                incremento_aux--;
            }
        }
        else //si no es NO_OP agrego la instruccion a la lista
        {   
            t_instruccion *instruccion_aux = malloc(sizeof(t_instruccion));//para poder borrar la lista aux y que no se borren los elementos.
            instruccion_aux->identificador = malloc(strlen(instruccion->identificador) + 1);
            strcpy(instruccion_aux->identificador, instruccion->identificador);
            instruccion_aux->argumentos = list_create();
            for(j = 0; j < list_size(instruccion->argumentos); j++)
            {
                t_argumento *argumento_aux = malloc(sizeof(t_argumento));
                t_argumento* provisorio = list_get(instruccion->argumentos, j);
                argumento_aux->argumento = provisorio->argumento;
                list_add(instruccion_aux->argumentos, argumento_aux);
            }
            list_add(lista_instrucciones, instruccion_aux);
        }
    } 
    // destruir lista auxiliar
    list_destroy_and_destroy_elements(lista_instrucciones_aux, (void *)destruir_instruccion);
    

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
    strcpy(instruccion_nueva->identificador, token); //guardo el identificador
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
