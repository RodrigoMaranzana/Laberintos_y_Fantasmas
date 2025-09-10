#include "../../include/cliente/cliente.h"
#include <stdio.h>
#include <string.h>

int cliente_inicializar()
{
    WSADATA wsa;
    int retorno;

    retorno = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (retorno) {

        printf("WSAStartup() %d\n", retorno);
    }

    return retorno;
}

SOCKET cliente_conectar_servidor(const char *ipServidor, int puerto)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {

        return INVALID_SOCKET;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(puerto);
    server_addr.sin_addr.s_addr = inet_addr(ipServidor);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {

        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

int cliente_enviar_solicitud(SOCKET sock, const char *solicitud, char *respuesta)
{
    int bytesRecividos;

    if (send(sock, solicitud, strlen(solicitud), 0) < 0) {

        return -1;
    }

    bytesRecividos = recv(sock, respuesta, TAM_BUFFER - 1, 0);
    if (bytesRecividos <= 0) {

        return -1;
    }

    respuesta[bytesRecividos] = '\0';
    return 0;
}

void cliente_cerrar_conexion(SOCKET sock)
{
    closesocket(sock);
    WSACleanup();
}
