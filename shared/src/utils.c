#include "../include/utils.h"

void loggear_lista_instrucciones(t_list* lista_instrucciones, t_log* logger){
    log_info(logger, "Lista de instrucciones cargadas:");
    for(uint32_t i = 0; i < list_size(lista_instrucciones); i++){
        t_instruccion* instruccion = list_get(lista_instrucciones, i);
        log_info(logger, "Instruccion: %s", instruccion->identificador);
        log_info(logger, "Tamanio de la instruccion: %d", strlen(instruccion->identificador));
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
    destruir_pcb(pcb);
}

int buscar_indice_pcb_en_lista_bloqueados(t_list* lista_bloqueados, t_pcb* pcb){
    int indice = -1;
    for(int i = 0; i < list_size(lista_bloqueados); i++){
        t_pcb* pcb_bloqueado = list_get(lista_bloqueados, i);
        if(pcb_bloqueado->id == pcb->id){
            indice = i; //devuelve el indice del proceso que queremos desbloquear
            break;
        }
    }
    return indice; //si recibe -1 no lo encontro
}

void loggear_info(t_log* logger, char* mensaje, pthread_mutex_t mutex){
    pthread_mutex_lock(&mutex);
    log_info(logger, mensaje);
    pthread_mutex_unlock(&mutex);
}

void loggear_error(t_log* logger, char* mensaje, pthread_mutex_t mutex){
    pthread_mutex_lock(&mutex);
    log_error(logger, mensaje);
    pthread_mutex_unlock(&mutex);
}

void queue_push_con_mutex(t_queue* queue, t_pcb* pcb, pthread_mutex_t mutex){
    pthread_mutex_lock(&mutex);
    queue_push(queue, pcb);
    pthread_mutex_unlock(&mutex);
}

t_pcb* queue_pop_con_mutex(t_queue* queue, pthread_mutex_t mutex){
    t_pcb* pcb;
    pthread_mutex_lock(&mutex);
    pcb = queue_pop(queue);
    pthread_mutex_unlock(&mutex);
    return pcb;
}

int queue_size_con_mutex(t_queue* queue, pthread_mutex_t mutex){
    int size;
    pthread_mutex_lock(&mutex);
    size = queue_size(queue);
    pthread_mutex_unlock(&mutex);
    return size;
}

bool queue_vacia_con_mutex(t_queue* queue, pthread_mutex_t mutex){
    bool is_empty;
    pthread_mutex_lock(&mutex);
    is_empty = queue_is_empty(queue);
    pthread_mutex_unlock(&mutex);
    return is_empty;
}

void destruir_pcb(t_pcb* pcb){
    if(list_size(pcb->instrucciones) > 0){
        list_destroy_and_destroy_elements(pcb->instrucciones, (void*) destruir_instruccion);
    }
    free(pcb);
}

