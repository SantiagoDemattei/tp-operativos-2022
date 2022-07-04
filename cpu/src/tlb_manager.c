#include "../include/tlb_manager.h"

void agregar(t_tlb *elemento_a_agregar)
{
    ALGORITMO_REEMPLAZO algoritmo;
    if (list_size_con_mutex_tlb(tlb, mutex_tlb) < configuracion_cpu->entradas_tlb)
    { // si la tlb no esta llena ===> tiene espacio libre ===> agrego la entrada nueva
        list_add_con_mutex_tlb(tlb, elemento_a_agregar, mutex_tlb);
    }
    else
    { // si la tlb esta llena, tengo que reemplazar usando algun algoritmo
        algoritmo = algoritmo_remplazo(configuracion_cpu->reemplazo_tlb);
        switch (algoritmo)
        {
            {
            case FIFO:
                break;
            case LRU:
                break;
            default:
                loggear_error(logger_cpu, "Algoritmo de reemplazo no reconocido", mutex_logger_cpu);
                break;
            }
        }
    }
}

uint32_t buscar(uint32_t numero_pagina)
{
    uint32_t marco_buscado = list_find_con_mutex_tlb(tlb, numero_pagina, mutex_tlb); 
    if (marco_buscado == -1)
    {
        loggear_error(logger_cpu, "No se encontro la pagina en la tlb", mutex_logger_cpu);
        return -1;
    }
    loggear_success(logger_cpu, "Se encontro la pagina en la tlb", mutex_logger_cpu);
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
