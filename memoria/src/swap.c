#include "../include/swap.h"

bool crear_archivo_swap(t_estructura_proceso *estructura, uint32_t tamanio, t_log *logger, pthread_mutex_t mutex)
{
    loggear_info(logger, "Creando archivo SWAP\n", mutex);
    int fd = open(estructura->nombre_archivo_swap, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // creo el archivo en la ruta indicada con esos permisos salvo que ya lo este y directamente lo leo con los permisos de usuario

    if (fd == -1)
    {
        loggear_error(logger, "No se pudo crear el area de SWAP\n", mutex);
        return false;
    }

    ftruncate(fd, tamanio); // ajusto el tamaño del archivo creado al tamaño del proceso
    estructura->archivo_swap = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // mapeo el archivo creado en memoria
    if (errno != 0)
        log_error(logger, "Error en mmap: errno %i", errno); // si hay un error al hacer mmap devuelve el codigo de error (errno = "error number")

    memset(estructura->archivo_swap, 0, tamanio); // inicializo el archivo con 0's BYTES

    close(fd); // cierro el archivo
    return true;
}

void *buscar_contenido_pagina_en_swap(void *archivo_mappeado, uint32_t nro_pagina, size_t tam_pagina)
{
    void *contenido_pagina = malloc(tam_pagina);
    printf("voy a hacer el memcpy\n");
    memcpy(contenido_pagina, archivo_mappeado + nro_pagina * tam_pagina, tam_pagina); // pagina ocupa desde donde arranca + tamanio de la pagina
    printf("hice el memcpy\n");
    //mostrar_contenido(contenido_pagina, tam_pagina);
    return contenido_pagina; // chorizo de bytes
}

void escribir_contenido_pagina_en_marco(uint32_t inicio, void *contenido_pagina, uint32_t nro_frame, size_t tamanio_frame)
{ // cargas el contenido de la pagina en memoria RAM
    char* mensaje;
    pthread_mutex_lock(&mutex_espacio_memoria);
    size_t inicio_real = inicio * tamanio_frame;                                                        // donde arranca el proceso en memoria RAM
    memcpy(espacio_memoria + inicio_real + nro_frame * tamanio_frame, contenido_pagina, tamanio_frame); // copio el contenido de la pagina dentro del frame
    mensaje = string_from_format("Se escribio en marco %d\n", nro_frame);
    loggear_info(logger, mensaje, mutex_logger_memoria);
    free(mensaje);
    free(contenido_pagina); // libero la variable del contenido de la pagina
    pthread_mutex_unlock(&mutex_espacio_memoria);
}

void *buscar_contenido_pagina_en_memoria(uint32_t inicio, uint32_t nro_frame, size_t tamanio_frame)
{
    pthread_mutex_lock(&mutex_espacio_memoria);
    void *contenido_pagina = malloc(tamanio_frame);
    size_t inicio_real = inicio * tamanio_frame;
    memcpy(contenido_pagina, espacio_memoria + inicio_real + nro_frame * tamanio_frame, tamanio_frame);
    pthread_mutex_unlock(&mutex_espacio_memoria);
    // mostrar_contenido(contenido_pagina, tamanio_frame);
    return contenido_pagina;
}

void escribir_contenido_pagina_en_swap(void *archivo_mappeado, void *contenido_pagina, uint32_t nro_pagina, size_t tam_pagina)
{ // descargas la pagina de la RAM a swap
    // mostrar_contenido(contenido_pagina, tam_pagina);
    memcpy(archivo_mappeado + nro_pagina * tam_pagina, contenido_pagina, tam_pagina);
    loggear_info(logger, "Se escribio en swap\n", mutex_logger_memoria);
    free(contenido_pagina);
}

void mostrar_contenido(void *contenido, size_t tam_pagina)
{
    // mostrar cada byte del void*
    int i;
    for (i = 0; i < tam_pagina; i++)
    {
        printf("%d ", ((char *)contenido)[i]);
    }
}

void swap()
{
    while (true)
    {
        sem_wait(&sem_swap);
        printf("despues del semaforo\n");
        switch (variable_global->indicador)
        {
        case CREAR_ARCHIVO_SWAP:
            printf("WDGIOWHDFHNSDHNFJSHDFJSHDFJHNSDJF\n");
            crear_archivo_swap(variable_global->proceso, variable_global->tamanio_proceso, logger, mutex_logger_memoria);
            sem_post(&sem_fin_swap);
            usleep(configuracion_memoria->retardo_swap * 1000); // tiempo que se debera esperar para cada operacion del SWAP
            break;

        case ESCRIBIR_PAGINA_SWAP:
            escribir_contenido_pagina_en_swap(variable_global->proceso->archivo_swap, variable_global->contenido_pagina_que_esta_cargada, variable_global->nro_pagina, configuracion_memoria->tam_pagina);
            if (variable_global->es_de_cpu)
            {
                usleep(configuracion_memoria->retardo_swap * 1000);
            }
            sem_post(&sem_fin_swap);
            break;

        case BUSCAR_CONTENIDO_PAGINA_EN_SWAP:
            variable_global->contenido_pagina = buscar_contenido_pagina_en_swap(variable_global->proceso->archivo_swap, variable_global->nro_pagina, configuracion_memoria->tam_pagina);
            usleep(configuracion_memoria->retardo_swap * 1000);
            sem_post(&sem_fin_swap);
            break;
        }
    }
}

/*
ENCICLOPEDIA: NO BORRAR

flags y modos de open:
    O_RDWR = 00200 user has read/write permission
    O_CREATE = If pathname does not exist, create it as a regular file.
    S_IRUSR =  00400 user has read permission
    S_IWUSR = 00200 user has write permission

parametros de mmap:
    1) desde que direccion quiero que empiece el "pedazo de memoria"; si pongo NULL, el kernel de mi SO decide donde poner la memoria asignada.
    2) tamanio del pedazo de memoria que quiero que me asigne el kernel.
    3) PROT_READ | PROT_WRITE = permisos de lectura y escritura.
    4) MAP_SHARED = permito que el mapeo pueda ser accedido por otros procesos, ademas del SO.
    5) fd = file descriptor del archivo que quiero mapear.
    6) offset = desde que posicion quiero que se pueda acceder el archivo. Al ponerlo en 0, hago que cuando yo quiera leer / escribir en el archivo, comience desde el principio.

parametros de memset:
    1) el pedazo de bytes que queres escribir (en este caso inicializar)
    2) valor que quiero que se ponga en la toda la memoria que le pase como primer parametro.
    3) tamanio del pedazo de memoria que estoy por escribir.

*/
