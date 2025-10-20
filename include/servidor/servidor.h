#ifndef SERVIDOR_H_INCLUDED
#define SERVIDOR_H_INCLUDED
#include <winsock2.h>
#include "../../include/servidor/bdatos.h"

typedef enum {
    SERVIDOR_BD_TODO_OK,
}eServidorRet;

int servidor_inicializar();
SOCKET servidor_crear_socket(int puerto);
void servidor_procesar_solicitud(tBDatos *bDatos, SOCKET *sock, const char *solicitud);

#endif // SERVIDOR_H_INCLUDED
