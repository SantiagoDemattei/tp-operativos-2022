#include "../include/consola.h"

void iniciar_consola(int tamanio, char* path){
    t_list* lista_instrucciones = obtener_instrucciones(path);
    // faltaria verificar acá qué elementos se guardaron en la lista
    // mostrar_instrucciones(lista_instrucciones); VER ESTA FUNCION PARA MOSTRAR
    
    /*
    cargar_configuracion();
    conectar_kernel();
    enviar_info_al_kernel(tamanio, lista_instrucciones);
    */

}

void print_instrucciones(char* instruccion){
    printf("Instruccion: %s\n", instruccion);
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
    while(read = getline(&linea, &capacidad, archivo) != -1){
    	// printf("Retrieved line of length %zu:\n", read);
    	// printf("%s", linea);
        list_add(lista_instrucciones, linea);
    }

    fclose(archivo);

    return lista_instrucciones;
}

void mostrar_instrucciones(t_list* lista_instrucciones){
    list_iterate(lista_instrucciones, (void*)print_instrucciones);
}



