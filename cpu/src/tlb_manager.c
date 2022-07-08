#include "../include/tlb_manager.h"

void agregar(t_tlb *elemento_a_agregar)
{
    ALGORITMO_REEMPLAZO algoritmo;
    t_tlb *victima;
    
    if (list_size_con_mutex_tlb(tlb, mutex_tlb) < configuracion_cpu->entradas_tlb)
    { // si la tlb no esta llena ===> tiene espacio libre ===> agrego la entrada nueva
        elemento_a_agregar->ultima_referencia = time(NULL); //tiempo actual 
        list_add_con_mutex_tlb(tlb, elemento_a_agregar, mutex_tlb);
    }
    else
    { // si la tlb esta llena, tengo que reemplazar usando algun algoritmo
        algoritmo = algoritmo_remplazo(configuracion_cpu->reemplazo_tlb);
        switch (algoritmo)
        {
            {
            case FIFO:
                free(list_remove(tlb, 0)); // saco el primero de la tlb (que es el primero que entro) y lo libero
                elemento_a_agregar->ultima_referencia = time(NULL);
                list_add_con_mutex_tlb(tlb, elemento_a_agregar, mutex_tlb); // agrego al final de la tlb
                break;
            case LRU:
                victima = elegir_victima_lru();
                borrar_entrada(victima); //borramos de la tlb
                elemento_a_agregar->ultima_referencia = time(NULL);
                list_add_con_mutex_tlb(tlb, elemento_a_agregar, mutex_tlb);
                break;
            default:
                loggear_error(logger_cpu, "Algoritmo de reemplazo no reconocido", mutex_logger_cpu);
                break;
            }
        }
    }
}

t_tlb* elegir_victima_lru(){
    t_tlb* victima = NULL;
    pthread_mutex_lock(&mutex_tlb);
    victima = list_get_minimum(tlb, (void *)comparar_ultimas_referencias); // ultima referencia mas chica = es la que hace mas tiempo que no uso
    pthread_mutex_unlock(&mutex_tlb);
    return victima;
}

void* comparar_ultimas_referencias(t_tlb* entrada1, t_tlb* entrada2){
    if(difftime(entrada1->ultima_referencia, entrada2->ultima_referencia) > 0){ //t1 - t2 devuelvo la de tiempo mas chica como victima.
        return entrada2;
    } else return entrada1;
}

void borrar_entrada(t_tlb* victima){
    uint32_t indice_victima = list_find_con_mutex_tlb_indice(tlb, victima->pagina, mutex_tlb);
    free(list_remove(tlb, indice_victima));
}

uint32_t buscar(uint32_t numero_pagina) //sirve para ver si esta en la TLB y si lo encuentra lo devuelve
{
    uint32_t marco_buscado = list_find_con_mutex_tlb(tlb, numero_pagina, mutex_tlb); //busca el numero de pagina en la TLB
    if (marco_buscado == -1)
    {
        loggear_error(logger_cpu, "No se encontro la pagina en la tlb", mutex_logger_cpu);
        return -1;
    }
    loggear_warning(logger_cpu, "Se encontro la pagina en la tlb", mutex_logger_cpu);
    return marco_buscado;
}

ALGORITMO_REEMPLAZO algoritmo_remplazo(char *algoritmo)
{
    if (!strcmp(algoritmo, "FIFO"))
    {
        return FIFO;
    }
    else if (!strcmp(algoritmo, "LRU"))
    {
        return LRU;
    }
    return -1;
}
