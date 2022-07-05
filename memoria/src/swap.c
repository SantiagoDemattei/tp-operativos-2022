#include "../include/swap.h"

bool crear_archivo_swap(t_estructura_proceso* estructura, uint32_t tamanio, t_log* logger, pthread_mutex_t mutex)
{
    loggear_info(logger, "Creando archivo SWAP\n", mutex);
    printf("ruta: %s\n", estructura->nombre_archivo_swap);
    int fd = open(estructura->nombre_archivo_swap, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // creo el archivo en la ruta indicada con esos permisos salvo que ya lo este y directamente lo leo con los permisos de usuario 
 
    if(fd == -1){
        loggear_error(logger, "No se pudo crear el area de SWAP\n", mutex);
        return false;
    }

    ftruncate(fd, tamanio); // ajusto el tamaño del archivo creado al tamaño del proceso

    estructura->archivo_swap = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // mapeo el archivo creado en memoria
    if (errno!=0) log_error(logger, "Error en mmap: errno %i", errno); // si hay un error al hacer mmap devuelve el codigo de error (errno = "error number")

    memset(estructura->archivo_swap, 0, tamanio); // inicializo el archivo con 0's BYTES

    close(fd); // cierro el archivo
    return true;
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

