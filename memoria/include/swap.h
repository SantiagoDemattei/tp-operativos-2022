#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include<math.h>
#include<signal.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/utils.h"
#include "../../shared/include/estructuras.h"

bool crear_archivo_swap(t_estructura_proceso* estructura, uint32_t tamanio, t_log* logger, pthread_mutex_t mutex);
void* buscar_contenido_pagina_en_swap(void* archivo_mappeado, uint32_t nro_pagina, uint32_t tam_pagina);
void escribir_contenido_pagina_en_marco(void* memoria_del_proceso, void* contenido_pagina, uint32_t nro_frame, uint32_t tamanio_frame);
#endif