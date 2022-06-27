#include "../include/protocolo.h"

#pragma region INICIAR CONSOLA
// inicio: INICIAR CONSOLA
static void *serializar_t_list_instrucciones(size_t *size, t_list *lista)
{
    // calculo tamaño en bytes de la lista de instrucciones (el tamanio que va a tener el stream de instrucciones)
    *size = 0;
    t_list_iterator *list_it = list_iterator_create(lista); // para iterar la lista y calcular su tamaño para el stream de instrucciones
    for (uint32_t i = 0; list_iterator_has_next(list_it); i++)
    {
        t_instruccion *instruccion = list_iterator_next(list_it);
        *size += sizeof(uint32_t) + strlen(instruccion->identificador) + sizeof(uint32_t) + sizeof(uint32_t) * list_size(instruccion->argumentos); // tamanio instruccion = longitud identificador + tamanio identificador + cantidad argumentos + tamanio lista argumentos
    }
    list_iterator_destroy(list_it);

    void *stream = malloc(*size); // stream = listade(longitud  identificador + identificador + cantArgumentos + lista argumentos) = stream que contiene lista de instrucciones para el kernel

    // Serializo las instrucciones
    list_it = list_iterator_create(lista);
    uint32_t offset = 0; // desplazamiento
    for (uint32_t i = 0; list_iterator_has_next(list_it); i++)
    {
        t_instruccion *instruccion = list_iterator_next(list_it);
        uint32_t tamanioCadena = strlen(instruccion->identificador);
        memcpy(stream + offset, &tamanioCadena, sizeof(uint32_t)); // longitud identificador
        offset += sizeof(uint32_t);

        memcpy(stream + offset, instruccion->identificador, strlen(instruccion->identificador)); // identificador
        offset += strlen(instruccion->identificador);                                            // desplazo en la longitud del identificador (= bytes ocupados por el identificador), recordar que sizeof(char) = 1
        uint32_t cantidadArgumentos = list_size(instruccion->argumentos);
        memcpy(stream + offset, &cantidadArgumentos, sizeof(uint32_t)); // cant argumentos
        offset += sizeof(uint32_t);
        for (uint32_t j = 0; j < list_size(instruccion->argumentos); j++)
        {
            memcpy(stream + offset, list_get(instruccion->argumentos, j), sizeof(uint32_t)); // cada uno de los argumentos
            offset += sizeof(uint32_t);
        }
    }
    list_iterator_destroy(list_it);
    return stream;
}

static t_list *deserializar_t_list_instrucciones(void *stream, size_t size)
{
    t_list *lista = list_create();
    uint32_t offset = 0;
    while (offset < size)
    { // mientras el desplazamiento sea menor al tamanio del stream
        t_instruccion *instruccion = malloc(sizeof(t_instruccion));
        uint32_t tamanioCadena = 0;
        memcpy(&tamanioCadena, stream + offset, sizeof(uint32_t));
        instruccion->identificador = malloc(tamanioCadena + 1); // sizeof(char) = 1 ==> tamaño id = tamanioCadena + 1
        offset += sizeof(uint32_t);
        strcpy(instruccion->identificador, stream + offset);
        instruccion->identificador[tamanioCadena] = '\0';
        offset += tamanioCadena;
        instruccion->argumentos = list_create();
        uint32_t cantidad_argumentos = 0;
        memcpy(&cantidad_argumentos, stream + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        for (uint32_t i = 0; i < cantidad_argumentos; i++)
        {
            uint32_t *argumento = malloc(sizeof(uint32_t));
            memcpy(argumento, stream + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            list_add(instruccion->argumentos, argumento);
        }
        list_add(lista, instruccion);
    }
    return lista;
}

static void *serializar_iniciar_consola(size_t *size, t_list *instrucciones, uint32_t tamanioConsola)
{
    size_t size_instrucciones; // size del stream de instrucciones
    void *stream_instrucciones = serializar_t_list_instrucciones(&size_instrucciones, instrucciones);
    // tamaño stream completo
    size_t size_total =
        sizeof(op_code) +   // tamanio del op_code
        sizeof(size_t) +    // size total del payload
        sizeof(uint32_t) +  // tamanio de la consola
        sizeof(size_t) +    // tamanio del stream de instrucciones
        size_instrucciones; // stream de instrucciones

    void *stream = malloc(size_total);
    // Payload (todo lo que va despues del op code)
    size_t size_payload = size_total - sizeof(op_code) - sizeof(size_t); // el tamanio del payload no incluye el op_code ni su size (arranca dsp de su tamanio)

    op_code cop = INICIAR_PROCESO; // codigo para iniciar el proceso

    // Ahora lleno el stream COMPLETO
    memcpy(stream, &cop, sizeof(op_code));                                                                                           // op_code
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));                                                                 // size del payload
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &tamanioConsola, sizeof(uint32_t));                                            // tamanio de la consola
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &size_instrucciones, sizeof(size_t));                       // size del stream de instrucciones
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) + sizeof(size_t), stream_instrucciones, size_instrucciones); // stream de instrucciones

    free(stream_instrucciones);
    *size = size_total;

    return stream; // devuelve el stream COMPLETO: opcode + sizepayload + tamanioconsola + sizeStreamInstrucciones + streaminstrucciones
}

static void deserializar_iniciar_consola(void *stream, t_list **instrucciones, uint32_t *tamanioConsola)
{

    size_t size_instrucciones;
    memcpy(tamanioConsola, stream, sizeof(uint32_t));                       // tamanio de la consola
    memcpy(&size_instrucciones, stream + sizeof(uint32_t), sizeof(size_t)); // size del stream de instrucciones
    void *stream_instrucciones = malloc(size_instrucciones);
    memcpy(stream_instrucciones, stream + sizeof(uint32_t) + sizeof(size_t), size_instrucciones); // stream de instrucciones
    *instrucciones = deserializar_t_list_instrucciones(stream_instrucciones, size_instrucciones);
    free(stream_instrucciones);
}

bool send_iniciar_consola(uint32_t fd, t_list *instrucciones, uint32_t tamanioConsola) // solo puedo mandar bytes por sockets -> serializo (transformar en stream)
{
    size_t size;
    void *stream = serializar_iniciar_consola(&size, instrucciones, tamanioConsola); // size tiene el tamaño del stream completo
    if (send(fd, stream, size, 0) == -1)                                             // le mando el stream completo y su tamaño al server (por la conexion donde estan (fd))
    {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_iniciar_consola(uint32_t fd, t_list **instrucciones, uint32_t *tamanioConsola)
{

    size_t size;
    if (recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)) // size del payload
    {
        return false;
    }

    void *stream = malloc(size);
    if (recv(fd, stream, size, 0) != size) // payload (tamanioConsola + sizeStreamInstrucciones + streamInstrucciones)
    {
        free(stream);
        return false;
    }
    t_list *r_instrucciones;
    deserializar_iniciar_consola(stream, &r_instrucciones, tamanioConsola);
    *instrucciones = r_instrucciones;
    free(stream);
    return true;
}
// fin: INICIAR_CONSOLA
#pragma endregion

#pragma region DEBUG
// DEBUG
bool send_debug(uint32_t fd)
{
    op_code cop = DEBUG;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}
// fin: DEBUG
#pragma endregion

#pragma region ENVIO_PCB
// ENVIO_PCB
static void *serializar_pcb(size_t *size, t_pcb *pcb, op_code cop)
{
    size_t size_instrucciones;
    void *stream_instrucciones = serializar_t_list_instrucciones(&size_instrucciones, pcb->instrucciones);

    size_t size_total =
        sizeof(op_code) +      // tamanio del op_code
        sizeof(size_t) +       // size  del payload
        sizeof(uint32_t) * 6 + // size de todos los enteros del pcb
        sizeof(double) * 2 +      
        sizeof(bool) +         // size del bool de suspendido
        sizeof(size_t) +       // size del stream de instrucciones
        size_instrucciones;    // tamanio de la lista de instrucciones

    void *stream = malloc(size_total);
    size_t size_payload = size_total - sizeof(op_code)- sizeof(size_t); // el tamanio del payload no incluye el op_code ni su size (arranca dsp de su tamanio)

    // Ahora lleno el stream
    memcpy(stream, &cop, sizeof(op_code)); // op_code
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));                                                                   // size del payload
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &(pcb->id), sizeof(uint32_t));                         // pid
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &(pcb->tamanio), sizeof(uint32_t)); // tamanio
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 2, &(pcb->program_counter), sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 3, &(pcb->tabla_pagina), sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 4, &(pcb->tiempo_bloqueo), sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 5, &(pcb->cliente_socket), sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 6, &(pcb->estimacion_rafaga_anterior), sizeof(double));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 6 + sizeof(double), &(pcb->rafaga_real_anterior), sizeof(double));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 6 + sizeof(double) *2, &(pcb->blocked_suspendido), sizeof(bool)); //size del bool de suspendido
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 6 + sizeof(double) * 2 + sizeof(bool) , &size_instrucciones, sizeof(size_t));                       // size del stream de instrucciones
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 6 + sizeof(double) * 2  + sizeof(bool) + sizeof(size_t) , stream_instrucciones, size_instrucciones); // stream de instrucciones

    free(stream_instrucciones);
    *size = size_total;

    return stream;
}

static void deserializar_pcb(void *stream, t_pcb **pcbF)
{
    t_pcb *pcb = malloc(sizeof(t_pcb));
    size_t size_instrucciones;

    memcpy(&(pcb->id), stream, sizeof(uint32_t));                                       // proceso id
    memcpy(&(pcb->tamanio), stream + sizeof(uint32_t), sizeof(uint32_t));               // tamanio
    memcpy(&(pcb->program_counter), stream + sizeof(uint32_t) * 2, sizeof(uint32_t));   // program_counter
    memcpy(&(pcb->tabla_pagina), stream + sizeof(uint32_t) * 3, sizeof(uint32_t));      // tabla pagina
    memcpy(&(pcb->tiempo_bloqueo), stream + sizeof(uint32_t) * 4, sizeof(uint32_t));    // tiempo de bloqueo
    memcpy(&(pcb->cliente_socket), stream + sizeof(uint32_t) * 5, sizeof(uint32_t));    // cliente socket
    memcpy(&(pcb->estimacion_rafaga_anterior), stream + sizeof(uint32_t) * 6 , sizeof(double));
    memcpy(&(pcb->rafaga_real_anterior), stream + sizeof(uint32_t) * 6 + sizeof(double), sizeof(double));
    memcpy(&(pcb->blocked_suspendido), stream + sizeof(uint32_t) * 6 + sizeof(double) * 2, sizeof(bool)); //suspendido
    memcpy(&size_instrucciones, stream + sizeof(uint32_t) * 6 + sizeof(double) * 2 + sizeof(bool) , sizeof(size_t)); // tamanio de la lista de instrucciones
    void *stream_instrucciones = malloc(size_instrucciones);
    memcpy(stream_instrucciones, stream + sizeof(uint32_t) * 6 + sizeof(double) * 2 + sizeof(bool) + sizeof(size_t), size_instrucciones); // stream de instrucciones
    t_list *instrucciones = deserializar_t_list_instrucciones(stream_instrucciones, size_instrucciones);
    pcb->instrucciones = instrucciones;

    // copiar pcb en pcbF
    *pcbF = pcb;

    free(stream_instrucciones);
}

bool send_pcb(uint32_t fd, t_pcb *pcb, op_code cop)
{
    size_t size;
    void *stream = serializar_pcb(&size, pcb, cop);
    if (send(fd, stream, size, 0) == -1)
    {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_pcb(uint32_t fd, t_pcb **pcb)
{
    size_t size;

    if (recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)) // payload
    {
        return false;
    }

    void *stream = malloc(size);
    if (recv(fd, stream, size, 0) != size)
    {
        free(stream);
        return false;
    }
    // recibe el  payload
    t_pcb *pcbF;
    deserializar_pcb(stream, &pcbF);
    *pcb = pcbF;
    free(stream);
    return true;
}

// fin: ENVIO_PCB
#pragma endregion

#pragma region INICIALIZAR_ESTRUCTURAS
// INICIALIZAR_ESTRUCTURAS (memoria)

bool send_inicializar_estructuras(uint32_t fd, uint32_t tamanio_proceso, uint32_t id_proceso)
{   
    op_code op = INICIALIZAR_ESTRUCTURAS;
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) * 2;
    void *stream = malloc(size);
    
    memcpy(stream, &op, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &tamanio_proceso, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &id_proceso, sizeof(uint32_t));
    if (send(fd, stream, size, 0) == -1)
    {
        free(stream);
        return false;
    }
    free(stream);
    return true;

}

bool recv_inicializar_estructuras(uint32_t fd, uint32_t* tamanio_proceso, uint32_t* id_proceso){
    size_t size;
    if (recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)) // payload
    {
        return false;
    }

    void *stream = malloc(size);
    if (recv(fd, stream, size, 0) != size)
    {
        free(stream);
        return false;
    }
    // recibe el  payload
    memcpy(tamanio_proceso, stream, sizeof(uint32_t));
    memcpy(id_proceso, stream + sizeof(uint32_t), sizeof(uint32_t));
    free(stream);
    return true;
}

// fin: INICIALIZAR_ESTRUCTURAS
#pragma endregion

#pragma region FIN_PROCESO
bool send_fin_proceso(uint32_t fd)
{
    op_code cop = LIBERAR_ESTRUCTURAS;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}
#pragma endregion

#pragma region VALOR_TB
bool send_valor_tb(uint32_t fd, uint32_t valor_tb) // la memoria le tiene que mandar el valor de la tabla de paginas serializado al kernel y tambien la usamos para pasarle a la cpu el tamanio de las paginas 
{
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t); // stream: cop + sizePayload + valorTb
    void *stream = malloc(size);
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    op_code cop = VALOR_TB;
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &valor_tb, sizeof(uint32_t));
    if (send(fd, stream, size, 0) == -1)
    {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_valor_tb(uint32_t fd, uint32_t *valor_tb)
{
    uint32_t valor;
    size_t size;
    if (recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t))
    {
        return false;
    }
    void *stream = malloc(size);
    if (recv(fd, stream, size, 0) != sizeof(size))
    {
        free(stream);
        return false;
    }
    memcpy(&valor, stream, sizeof(uint32_t));
    *valor_tb = valor;
    free(stream);
    return true;
}

#pragma endregion

#pragma region INT_NUEVO_READY

bool send_interrupcion_por_nuevo_ready(uint32_t fd)
{
    op_code cop = INT_NUEVO_READY;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}

#pragma endregion

#pragma region SUSPENSION

bool send_suspension(uint32_t fd, uint32_t id)
{
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t); // stream: cop + sizePayload + id
    void *stream = malloc(size);

    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    op_code cop = SUSPENSION;

    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &id, sizeof(uint32_t));

    if (send(fd, stream, size, 0) == -1)
    {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_suspension(uint32_t fd, uint32_t *id)
{
    uint32_t valor;
    size_t size;
    if (recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t))
    {
        return false;
    }
    void *stream = malloc(size);
    if (recv(fd, stream, size, 0) != sizeof(size))
    {
        free(stream);
        return false;
    }
    memcpy(&valor, stream, sizeof(uint32_t));

    *id = valor;
    free(stream);
    return true;
}

#pragma endregion

#pragma region CONFIRMACION_SUSPENSION

bool send_confirmacion_suspension(uint32_t fd){
    op_code cod = CONFIRMACION_SUSPENSION;
    if(send(fd, &cod, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}

#pragma endregion

#pragma region OBTENER_TAMANIO
bool send_tamanio_y_cant_entradas(uint32_t fd, uint32_t tamanio, uint32_t cant_entradas){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) + sizeof(uint32_t); // stream: cop + sizePayload + numPagina + valor
    void *stream = malloc(size);
    op_code cop = OBTENER_TAMANIO;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &tamanio, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &cant_entradas, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_tamanio_y_cant_entradas(uint32_t fd, uint32_t *tamanio, uint32_t *cant_entradas){
    uint32_t cant_entradas_recibida;
    uint32_t tamanio_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&tamanio_recibido, stream, sizeof(uint32_t));
    memcpy(&cant_entradas_recibida, stream + sizeof(uint32_t), sizeof(uint32_t));
    *tamanio = tamanio_recibido;
    *cant_entradas = cant_entradas_recibida;
    free(stream);
    return true;
}

#pragma endregion

#pragma region ORDEN_ENVIO_TAMANIO

bool send_orden_envio_tamanio(uint32_t fd)
{
    op_code cop = ORDEN_ENVIO_TAMANIO;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}

#pragma endregion

#pragma region PRIMER_ACCESO

bool send_entrada_tabla_1er_nivel(uint32_t fd, uint32_t id_tabla1, uint32_t entrada_tabla_1er_nivel){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) + sizeof(uint32_t); // stream: cop + sizePayload + idTabla1 + entradaTabla1erNivel
    void *stream = malloc(size);
    op_code cop = PRIMER_ACCESO;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &id_tabla1, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &entrada_tabla_1er_nivel, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;
}


bool recv_entrada_tabla_1er_nivel(uint32_t fd, uint32_t *id_tabla, uint32_t *entrada_tabla_1er_nivel){
    uint32_t id_tabla_recibido;
    uint32_t entrada_tabla_1er_nivel_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&id_tabla_recibido, stream, sizeof(uint32_t));
    memcpy(&entrada_tabla_1er_nivel_recibido, stream + sizeof(uint32_t), sizeof(uint32_t));
    *id_tabla = id_tabla_recibido;
    *entrada_tabla_1er_nivel = entrada_tabla_1er_nivel_recibido;
    free(stream);
    return true;
}

#pragma endregion

#pragma region NUM_TABLA_SEGUNDO_NIVEL
bool send_num_tabla_2do_nivel(uint32_t fd, uint32_t num_tabla_2do_nivel){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t); // stream: cop + sizePayload + numTabla2doNivel
    void *stream = malloc(size);
    op_code cop = NUM_TABLA_SEGUNDO_NIVEL;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &num_tabla_2do_nivel, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_num_tabla_2do_nivel(uint32_t fd, uint32_t *num_tabla_2do_nivel){
    uint32_t num_tabla_2do_nivel_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&num_tabla_2do_nivel_recibido, stream, sizeof(uint32_t));
    *num_tabla_2do_nivel = num_tabla_2do_nivel_recibido;
    free(stream);
    return true;
}

#pragma endregion

#pragma region SEGUNDO_ACCESO

bool send_entrada_tabla_2do_nivel(uint32_t fd, uint32_t num_segundo_nivel, uint32_t entrada_tabla_2do_nivel){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) + sizeof(uint32_t); // stream: cop + sizePayload + idTabla1 + entradaTabla1erNivel
    void *stream = malloc(size);
    op_code cop = SEGUNDO_ACCESO;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &num_segundo_nivel, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &entrada_tabla_2do_nivel, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;
}


bool recv_entrada_tabla_2do_nivel(uint32_t fd, uint32_t *num_segundo_nivel, uint32_t *entrada_tabla_2do_nivel){
    uint32_t num_tabla_segundo;
    uint32_t entrada_tabla_2do_nivel_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&num_tabla_segundo, stream, sizeof(uint32_t));
    memcpy(&entrada_tabla_2do_nivel, stream + sizeof(uint32_t), sizeof(uint32_t));
    *num_segundo_nivel = num_tabla_segundo;
    *entrada_tabla_2do_nivel = entrada_tabla_2do_nivel_recibido;
    free(stream);
    return true;
}

#pragma endregion

#pragma region EJECUTAR_WRITE

bool send_ejecutar_write(uint32_t fd, uint32_t marco, uint32_t desplazamiento, uint32_t valor_a_escribir){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t)*3; // stream: cop + sizePayload + marco + desplazamiento + valorAEscribir
    void *stream = malloc(size);
    op_code cop = EJECUTAR_WRITE;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &marco, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &desplazamiento, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t) + sizeof(uint32_t), &valor_a_escribir, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_ejecutar_write(uint32_t fd, uint32_t* marco, uint32_t* desplazamiento, uint32_t* valor_a_escribir){
    uint32_t marco_recibido;
    uint32_t desplazamiento_recibido;
    uint32_t valor_a_escribir_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&marco_recibido, stream, sizeof(uint32_t));
    memcpy(&desplazamiento_recibido, stream + sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&valor_a_escribir_recibido, stream + sizeof(uint32_t) + sizeof(uint32_t), sizeof(uint32_t));
    *marco = marco_recibido;
    *desplazamiento = desplazamiento_recibido;
    *valor_a_escribir = valor_a_escribir_recibido;
    free(stream);
    return true;
}

#pragma endregion 

#pragma region OK

bool send_ok(uint32_t fd){
    op_code cop = OK;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}

#pragma endregion

#pragma region EJECUTAR_READ

bool send_ejecutar_read(uint32_t fd, uint32_t marco, uint32_t desplazamiento){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t)*2; // stream: cop + sizePayload + marco + desplazamiento
    void *stream = malloc(size);
    op_code cop = EJECUTAR_READ;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &marco, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &desplazamiento, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_ejecutar_read(uint32_t fd, uint32_t* marco, uint32_t* desplazamiento){
    uint32_t marco_recibido;
    uint32_t desplazamiento_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&marco_recibido, stream, sizeof(uint32_t));
    memcpy(&desplazamiento_recibido, stream + sizeof(uint32_t), sizeof(uint32_t));
    *marco = marco_recibido;
    *desplazamiento = desplazamiento_recibido;
    free(stream);
    return true;
}

#pragma endregion

#pragma region OK_READ

bool send_ok_read(uint32_t fd, uint32_t valor_leido){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t); // stream: cop + sizePayload + valorLeido
    void *stream = malloc(size);
    op_code cop = OK_READ;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &valor_leido, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_ok_read(uint32_t fd, uint32_t* valor_leido){
    uint32_t valor_leido_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&valor_leido_recibido, stream, sizeof(uint32_t));
    *valor_leido = valor_leido_recibido;
    free(stream);
    return true;
}

#pragma endregion

#pragma region FRAME

bool send_frame(uint32_t fd, uint32_t frame){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t); // stream: cop + sizePayload + frame
    void *stream = malloc(size);
    op_code cop = FRAME;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &frame, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;    
}

bool recv_frame(uint32_t fd, uint32_t* frame){
    uint32_t frame_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&frame_recibido, stream, sizeof(uint32_t));
    *frame = frame_recibido;
    free(stream);
    return true;
}

#pragma endregion

#pragma region EJECUTAR_COPY

bool send_ejecutar_copy(uint32_t fd, uint32_t marco_origen, uint32_t desplazamiento_origen, uint32_t marco_destino, uint32_t desplazamiento_destino){
    size_t size = sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t)*4; // stream: cop + sizePayload + marcoOrigen + desplazamientoOrigen + marcoDestino + desplazamientoDestino
    void *stream = malloc(size);
    op_code cop = EJECUTAR_COPY;
    size_t size_payload = size - sizeof(op_code) - sizeof(size_t);
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &size_payload, sizeof(size_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t), &marco_origen, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t), &desplazamiento_origen, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t)*2, &marco_destino, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(size_t) + sizeof(uint32_t)*3, &desplazamiento_destino, sizeof(uint32_t));
    if(send(fd, stream, size, 0) == -1){
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_ejecutar_copy(uint32_t fd, uint32_t* marco_origen, uint32_t* desplazamiento_origen, uint32_t* marco_destino, uint32_t* desplazamiento_destino){
    uint32_t marco_origen_recibido;
    uint32_t desplazamiento_origen_recibido;
    uint32_t marco_destino_recibido;
    uint32_t desplazamiento_destino_recibido;
    size_t size;
    if(recv(fd, &size, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void *stream = malloc(size);
    if(recv(fd, stream, size, 0) != size){
        free(stream);
        return false;
    }
    memcpy(&marco_origen_recibido, stream, sizeof(uint32_t));
    memcpy(&desplazamiento_origen_recibido, stream + sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&marco_destino_recibido, stream + sizeof(uint32_t)*2, sizeof(uint32_t));
    memcpy(&desplazamiento_destino_recibido, stream + sizeof(uint32_t)*3, sizeof(uint32_t));
    *marco_origen = marco_origen_recibido;
    *desplazamiento_origen = desplazamiento_origen_recibido;
    *marco_destino = marco_destino_recibido;
    *desplazamiento_destino = desplazamiento_destino_recibido;
    free(stream);
    return true;
}


#pragma endregion