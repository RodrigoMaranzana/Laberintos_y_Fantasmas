#ifndef CLIENTE_H_INCLUDED
#define CLIENTE_H_INCLUDED
#include <winsock2.h>
#include "../../include/comun/cola.h"

typedef enum {
    CE_TODO_OK,
    CE_DATOS,
    CE_SIN_RESPUESTA,
    CE_SIN_RESULTADOS,
    CE_ERR_SIN_MEM,
    CE_ERR_SOCKET,
    CE_ERR_RECEPCION_DATOS,
    CE_ERR_RESPUESTA_CORRUPTA,
    CE_ERR_SERVIDOR,
} eErrCliente;

typedef enum {
    SESION_ONLINE,
    SESION_OFFLINE,
}eEstadoSesion;

int cliente_inicializar();
SOCKET cliente_conectar_servidor(const char *ipServidor, int puerto);
int cliente_enviar_solicitud(SOCKET sock, const char *solicitud);
void cliente_destruir_conexion(SOCKET sock);
int cliente_recibir_respuesta(SOCKET sock, tCola *colaRespuestas);
#endif // CLIENTE_H_INCLUDED
