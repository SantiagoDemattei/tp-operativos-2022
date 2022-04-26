#include "../include/protocolo.h"

// INICIAR CONSOLA

static void* serializar_t_list_instrucciones(size_t* size, t_list* lista){

    // calculo tamaño en bytes de la lista de instrucciones (el tamaño que va a tener el stream de instrucciones)
    *size = 0;
    t_list_iterator* list_it = list_iterator_create(lista);
    for(int i=0; list_iterator_has_next(list_it); i++){
        t_instruccion* instruccion = list_iterator_next(list_it);
        *size += sizeof(int) + strlen(instruccion->identificador) + sizeof(int) + sizeof(int)*list_size(instruccion->argumentos); // tamanio instruccion = longitud identificador + tamanio identificador + cantidad argumentos + tamanio argumentos
    }
    list_iterator_destroy(list_it);

    void* stream = malloc(*size); // stream = listade(longitud + identificador + lista argumentos) = stream que contiene lista de instrucciones para el kernel

    // Serializo las instrucciones
    list_it = list_iterator_create(lista);
    int offset = 0; // desplazamiento
    for(int i=0; list_iterator_has_next(list_it); i++){
        t_instruccion* instruccion = list_iterator_next(list_it);
        int tamanioCadena = strlen(instruccion->identificador);
        memcpy(stream + offset, &tamanioCadena, sizeof(int));
        offset += sizeof(int);
        memcpy(stream + offset, instruccion->identificador, strlen(instruccion->identificador));
        offset += strlen(instruccion->identificador); // desplazo en la longitud del identificador (= bytes ocupados por el identificador), recordar que sizeof(char) = 1
        int cantidadArgumentos = list_size(instruccion->argumentos);
        memcpy(stream + offset, &cantidadArgumentos, sizeof(int));
        offset += sizeof(int);
        for(int j=0; j<list_size(instruccion->argumentos); j++){
            memcpy(stream + offset, list_get(instruccion->argumentos, j), sizeof(int));
            offset += sizeof(int);
        }
    }
    list_iterator_destroy(list_it);
    return stream;
}

static void* serializar_iniciar_consola(size_t* size, t_list* instrucciones, int tamanioConsola) {

    size_t size_instrucciones;
    void* stream_instrucciones = serializar_t_list_instrucciones(&size_instrucciones, instrucciones);

    // stream completo
    size_t size_total = 
        sizeof(op_code) +   // tamanio del op_code
        sizeof(size_t) + // size total del stream
        sizeof(int) + // tamanio de la consola
        sizeof(size_t) + // size del stream de instrucciones
        size_instrucciones; // tamanio de la lista de instrucciones
    
    void* stream = malloc(size_total);
    // Payload (todo lo que va despues del op code)
    size_t size_payload = size_total - sizeof(op_code) - sizeof(size_t); // el tamanio del payload no incluye el op_code ni el size
    
    op_code cop = INICIAR_PROCESO;

    // Ahora lleno el stream
    memcpy(stream, &cop, sizeof(op_code)); // op_code
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t)); // size del payload
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &tamanioConsola, sizeof(int)); // tamanio de la consola
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(int), &size_instrucciones, sizeof(size_t)); // size del stream de instrucciones
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(int) + sizeof(size_t), stream_instrucciones, size_instrucciones); // stream de instrucciones

    free(stream_instrucciones);
    *size=size_total;

    return stream;

}

static t_list* deserializar_t_list_instrucciones(void* stream, size_t size){ // ACA ESTA EL PROBLEMA CREO (Ya no hay mas segfault)
    t_list* lista = list_create();
    int offset = 0;
    while(offset < size){ // mientras el desplazamiento sea menor al tamanio del stream
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
<<<<<<< HEAD
        int tamanioCadena;
        memcpy(&tamanioCadena, stream + offset, sizeof(int));
        instruccion->identificador = malloc(tamanioCadena + 1); // sizeof(char) = 1 ==> tamaño id = tamanioCadena + 1
        offset += sizeof(int);
        memcpy(instruccion->identificador, stream + offset, tamanioCadena);
        offset += tamanioCadena;
        instruccion->argumentos = list_create(); 
        int cantidad_argumentos;
        memcpy(&cantidad_argumentos, stream + offset, sizeof(int));
=======
        instruccion->identificador = malloc(sizeof(char)); // no se como reservar memoria para la cadena (esta mal que sea sizeof(char) xq es una cadena y no un caracter)
        /*
        Para hacer el malloc del identificador deberiamos guardar el size de cada identificador, serializarlo y enviarlo por la conexion. Sino no hay forma de reservar
        la memoria exacta para la longitud del identificador porque los char* tiene tamanio variable (por eso siempre se hace malloc(strlen(identificador) + 1)).
        */

        memcpy(instruccion->identificador, stream + offset, sizeof(char));
        offset += sizeof(char);
        instruccion->argumentos = list_create();
        int cantidad_argumentos = *(int*)(stream + offset);
>>>>>>> ddb1f93fad660804b371ae8aef025d0a7351453c
        offset += sizeof(int);
        for(int i=0; i<cantidad_argumentos; i++){
            int* argumento = malloc(sizeof(int));
            memcpy(argumento, stream + offset, sizeof(int));
            offset += sizeof(int);
            list_add(instruccion->argumentos, argumento);
        }
        list_add(lista, instruccion);
    }
    return lista;
}

static void deserializar_iniciar_consola(void* stream, t_list** instrucciones, int* tamanioConsola) {
  
    size_t size_instrucciones; 
    memcpy(tamanioConsola, stream, sizeof(int)); // tamanio de la consola
    memcpy(&size_instrucciones, stream + sizeof(int), sizeof(size_t)); // size del stream de instrucciones
    void* stream_instrucciones = malloc(size_instrucciones);
    memcpy(stream_instrucciones, stream + sizeof(int) + sizeof(size_t), size_instrucciones); // stream de instrucciones
    *instrucciones = deserializar_t_list_instrucciones(stream_instrucciones, size_instrucciones);
    free(stream_instrucciones);
}

bool send_iniciar_consola(int fd, t_list* instrucciones, int tamanioConsola) {
    size_t size;
    void* stream = serializar_iniciar_consola(&size, instrucciones, tamanioConsola);
    if (send(fd, stream, size, 0) == -1) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_iniciar_consola(int fd, t_list** instrucciones, int* tamanioConsola) {
    
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


// DEBUG
bool send_debug(int fd) {
    op_code cop = DEBUG;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}