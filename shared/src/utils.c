#include "../include/utils.h"

void loggear_lista_instrucciones(t_list* lista_instrucciones, t_log* logger){
    log_info(logger, "Lista de instrucciones cargadas:");
    for(uint32_t i = 0; i < list_size(lista_instrucciones); i++){
        t_instruccion* instruccion = list_get(lista_instrucciones, i);
        log_info(logger, "Instruccion: %s", instruccion->identificador);
        for(uint32_t j = 0; j < list_size(instruccion->argumentos); j++){
            t_argumento* argumento = list_get(instruccion->argumentos, j);
            log_info(logger, "Argumento: %d", argumento->argumento);
        }
        log_info(logger, "\n"); // Meto un \n para separar las instrucciones
    }
}

void destruir_argumentos(t_argumento* argumento){
    free(argumento);
}

void destruir_instruccion(t_instruccion* instruccionS){
    if(instruccionS->identificador != NULL){
        free(instruccionS->identificador);
    }
    list_destroy_and_destroy_elements(instruccionS->argumentos, (void*) destruir_argumentos);
    free(instruccionS);
}


void destructor_queue(t_pcb* pcb){
    list_destroy_and_destroy_elements(pcb->instrucciones, (void*) destruir_instruccion);
}

