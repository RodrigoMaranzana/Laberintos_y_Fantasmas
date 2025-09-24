#ifndef SERVIDOR_H_INCLUDED
#define SERVIDOR_H_INCLUDED
#include <winsock2.h>
#include "../../include/servidor/bdatos.h"

int servidor_inicializar();
SOCKET servidor_crear_socket();
void servidor_procesar_solicitud(tBDatos *bDatos, const char *request, char *response);
void servidor_iniciar();

#endif // SERVIDOR_H_INCLUDED
