#ifndef CLIENTE_H_INCLUDED
#define CLIENTE_H_INCLUDED
#include <winsock2.h>

typedef enum {
    CE_TODO_OK,
    CE_DATOS,
    CE_SIN_RESULTADOS,
    CE_ERR_SIN_MEM,
    CE_ERR_ENVIO_SOLICITUD,
    CE_ERR_RECEPCION_RESPUESTA,
    CE_ERR_RECEPCION_DATOS,
    CE_ERR_SERVIDOR,
} eErrCliente;

int cliente_inicializar();
SOCKET cliente_conectar_servidor(const char *ipServidor, int puerto);
int cliente_enviar_solicitud(SOCKET sock, const char *solicitud);
void cliente_cerrar_conexion(SOCKET sock);
int cliente_recibir_respuesta(SOCKET sock, char *respuesta, int tamBuffer);
int cliente_recibir_datos(SOCKET sock, char *bufferDatos, int bytesEsperados);
int cliente_ejecutar_solicitud(SOCKET sock, const char *solicitud, int* cantRegistros, char** bufferDatos);
#endif // CLIENTE_H_INCLUDED
