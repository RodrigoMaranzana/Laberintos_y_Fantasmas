#ifndef CLIENTE_LIB_H_INCLUDED
#define CLIENTE_LIB_H_INCLUDED

#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024

// Inicializa Winsock
int init_winsock();

// Conecta al servidor y devuelve el socket
SOCKET connect_to_server(const char *server_ip, int port);

// Envía una solicitud y recibe la respuesta
int send_request(SOCKET sock, const char *request, char *response);

// Cierra la conexión y limpia Winsock
void close_connection(SOCKET sock);

#endif // CLIENTE_LIB_H_INCLUDED
