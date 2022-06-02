#include "../include/sockets.h"

// INICIA SERVER ESCUCHANDO EN IP:PUERTO
uint32_t iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto) {
    uint32_t socket_servidor;
    struct addrinfo hints, *servinfo;

    // Inicializando hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe los addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    bool conecto = false;
    
    // Itera por cada addrinfo devuelto
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_servidor == -1) // fallo de crear socket
            continue; //vuelve a iterar hasta encontrar el socket mas estable (que no de -1) y ahi pasa al siguiente if

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {  //el bind recibe el puerto que debe ocupar a partir de los datos que le pasamos al getaddinfo previamente 
            // Si entra aca fallo el bind
            close(socket_servidor); //cierra el socket y vuelve a buscar otro
            continue;
        }
        // Ni bien conecta uno nos vamos del for
        conecto = true;
        
        break;
        
    }

    if(!conecto) {  
        free(servinfo);
        return 0;
    }
    
    listen(socket_servidor, SOMAXCONN); // Escuchando (hasta SOMAXCONN conexiones simultaneas) -> esperando hacer "match" con alguien 
    // Aviso al logger que esta esperando al cliente 
    
    log_info(logger, "Esperando a %s:%s (%s)\n", ip, puerto, name);
    
    freeaddrinfo(servinfo);
    
    return socket_servidor; 
}

// ESPERAR CONEXION DE CLIENTE EN UN SERVER ABIERTO
uint32_t esperar_cliente(t_log* logger, const char* name, uint32_t socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    uint32_t socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion); //accept devuelve la linea donde quedan conectados

    log_info(logger, "Cliente conectado (a %s)\n", name);

    return socket_cliente;
}

// CLIENTE SE INTENTA CONECTAR A SERVER ESCUCHANDO EN IP:PUERTO
uint32_t crear_conexion_cliente(t_log* logger, const char* server_name, char* ip, char* puerto) {
    struct addrinfo hints, *servinfo;

    // Init de hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    // Crea un socket con la informacion recibida (del primero, suficiente) 
    uint32_t socket_cliente = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol); //crea el socket con la info recibida 

    // Fallo en crear el socket
    if(socket_cliente == -1) {
        log_error(logger, "Error creando el socket para %s:%s", ip, puerto);
        return 0;
    }

    // Error conectando
    if(connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1) { //intenta conectar el socket creado con el servidor
        log_error(logger, "Error al conectar (a %s)\n", server_name);
        freeaddrinfo(servinfo);
        return 0;
    } else
        log_info(logger, "Cliente conectado en %s:%s (a %s)\n", ip, puerto, server_name);

    freeaddrinfo(servinfo);

    return socket_cliente; 
}

// CERRAR CONEXION
void liberar_conexion(uint32_t socket_cliente) {
    close(socket_cliente);
}