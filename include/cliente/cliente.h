#ifndef CLIENTE_H_INCLUDED
#define CLIENTE_H_INCLUDED
#include <winsock2.h>

#define IP_SERVIDOR "127.0.0.1"
#define PUERTO 12345
#define TAM_BUFFER 1024

int cliente_inicializar();
SOCKET cliente_conectar_servidor(const char *ipServidor, int puerto);
int cliente_enviar_solicitud(SOCKET sock, const char *solicitud, char *respuesta);
void cliente_cerrar_conexion(SOCKET sock);

#endif // CLIENTE_H_INCLUDED
