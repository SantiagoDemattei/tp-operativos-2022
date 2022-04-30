#include "../include/protocolo.h"

// inicio: INICIAR CONSOLA

static void* serializar_t_list_instrucciones(size_t* size, t_list* lista){

    // calculo tamaño en bytes de la lista de instrucciones (el tamaño que va a tener el stream de instrucciones)
    *size = 0;
    t_list_iterator* list_it = list_iterator_create(lista);
    for(uint32_t i=0; list_iterator_has_next(list_it); i++){
        t_instruccion* instruccion = list_iterator_next(list_it);
        *size += sizeof(uint32_t) + strlen(instruccion->identificador) + sizeof(uint32_t) + sizeof(uint32_t)*list_size(instruccion->argumentos); // tamanio instruccion = longitud identificador + tamanio identificador + cantidad argumentos + tamanio argumentos
    }
    list_iterator_destroy(list_it);

    void* stream = malloc(*size); // stream = listade(longitud + identificador + lista argumentos) = stream que contiene lista de instrucciones para el kernel

    // Serializo las instrucciones
    list_it = list_iterator_create(lista);
    uint32_t offset = 0; // desplazamiento
    for(uint32_t i=0; list_iterator_has_next(list_it); i++){
        t_instruccion* instruccion = list_iterator_next(list_it);
        uint32_t tamanioCadena = strlen(instruccion->identificador);
        memcpy(stream + offset, &tamanioCadena, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(stream + offset, instruccion->identificador, strlen(instruccion->identificador));
        offset += strlen(instruccion->identificador); // desplazo en la longitud del identificador (= bytes ocupados por el identificador), recordar que sizeof(char) = 1
        uint32_t cantidadArgumentos = list_size(instruccion->argumentos);
        memcpy(stream + offset, &cantidadArgumentos, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        for(uint32_t j=0; j<list_size(instruccion->argumentos); j++){
            memcpy(stream + offset, list_get(instruccion->argumentos, j), sizeof(uint32_t));
            offset += sizeof(uint32_t);
        }
    }
    list_iterator_destroy(list_it);
    return stream;
}

static void* serializar_iniciar_consola(size_t* size, t_list* instrucciones, uint32_t tamanioConsola) {

    size_t size_instrucciones;
    void* stream_instrucciones = serializar_t_list_instrucciones(&size_instrucciones, instrucciones);

    // stream completo
    size_t size_total = 
        sizeof(op_code) +   // tamanio del op_code
        sizeof(size_t) + // size total del stream
        sizeof(uint32_t) + // tamanio de la consola
        sizeof(size_t) + // size del stream de instrucciones
        size_instrucciones; // tamanio de la lista de instrucciones
    
    void* stream = malloc(size_total);
    // Payload (todo lo que va despues del op code)
    size_t size_payload = size_total - sizeof(op_code) - sizeof(size_t); // el tamanio del payload no incluye el op_code ni el size
    
    op_code cop = INICIAR_PROCESO;

    // Ahora lleno el stream
    memcpy(stream, &cop, sizeof(op_code)); // op_code
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t)); // size del payload
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &tamanioConsola, sizeof(uint32_t)); // tamanio de la consola
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &size_instrucciones, sizeof(size_t)); // size del stream de instrucciones
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) + sizeof(size_t), stream_instrucciones, size_instrucciones); // stream de instrucciones

    free(stream_instrucciones);
    *size=size_total;

    return stream;

}

static t_list* deserializar_t_list_instrucciones(void* stream, size_t size){ // Uninitialised value was created by a heap allocation
    t_list* lista = list_create();
    uint32_t offset = 0;
    while(offset < size){ // mientras el desplazamiento sea menor al tamanio del stream
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        uint32_t tamanioCadena = 0;
        memcpy(&tamanioCadena, stream + offset, sizeof(uint32_t));
        instruccion->identificador = malloc(tamanioCadena + 1); // sizeof(char) = 1 ==> tamaño id = tamanioCadena + 1
        offset += sizeof(uint32_t);
        memcpy(instruccion->identificador, stream + offset, tamanioCadena);
        offset += tamanioCadena;
        instruccion->argumentos = list_create(); 
        uint32_t cantidad_argumentos = 0;
        memcpy(&cantidad_argumentos, stream + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        for(uint32_t i=0; i<cantidad_argumentos; i++){
            uint32_t* argumento = malloc(sizeof(uint32_t));
            memcpy(argumento, stream + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            list_add(instruccion->argumentos, argumento);
        }
        list_add(lista, instruccion);
    }
    return lista;
}

static void deserializar_iniciar_consola(void* stream, t_list** instrucciones, uint32_t* tamanioConsola) {
  
    size_t size_instrucciones; 
    memcpy(tamanioConsola, stream, sizeof(uint32_t)); // tamanio de la consola
    memcpy(&size_instrucciones, stream + sizeof(uint32_t), sizeof(size_t)); // size del stream de instrucciones
    void* stream_instrucciones = malloc(size_instrucciones);
    memcpy(stream_instrucciones, stream + sizeof(uint32_t) + sizeof(size_t), size_instrucciones); // stream de instrucciones
    *instrucciones = deserializar_t_list_instrucciones(stream_instrucciones, size_instrucciones);
    free(stream_instrucciones);
}

bool send_iniciar_consola(uint32_t fd, t_list* instrucciones, uint32_t tamanioConsola) {
    size_t size;
    void* stream = serializar_iniciar_consola(&size, instrucciones, tamanioConsola);
    if (send(fd, stream, size, 0) == -1) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_iniciar_consola(uint32_t fd, t_list** instrucciones, uint32_t* tamanioConsola) {
    
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)) {
        return false;
    }
    // recibe el stream sin el opcode (size_payload + payload)
    void* stream = malloc(size);
    if(recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    t_list* r_instrucciones;
    deserializar_iniciar_consola(stream, &r_instrucciones, tamanioConsola);
    *instrucciones = r_instrucciones;
    free(stream);
    return true;
}
// fin: INICIAR_CONSOLA

// DEBUG
bool send_debug(uint32_t fd) {
    op_code cop = DEBUG;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}