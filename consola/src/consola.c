#include "../include/consola.h"

void iniciar_consola(int tamanio, char* path){
    t_list* lista_instrucciones = obtener_instrucciones(path);

    list_iterate(lista_instrucciones, (void*) print_instrucciones); 
    
    /*
    cargar_configuracion();
    conectar_kernel();
    enviar_info_al_kernel(tamanio, lista_instrucciones);
    */

    list_destroy_and_destroy_elements(lista_instrucciones, free);
}

void print_instrucciones(char* instruccion){
    printf("Instruccion: %s\n", instruccion);
}

t_list* obtener_instrucciones(char* path){
    FILE* archivo = fopen(path, "r");
    t_list* lista_instrucciones = list_create();

    //leer linea por linea del archivo
    char* linea = NULL;
    size_t capacidad = 0;
    while(getline(&linea, &capacidad, archivo) != -1){
        list_add(lista_instrucciones, linea);
    }
    fclose(archivo);

    return lista_instrucciones;
}


