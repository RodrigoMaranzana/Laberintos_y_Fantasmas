#include "../../include/comun/comun.h"
#include "../../include/servidor/servidor.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int servidor_inicializar(tBDatos *bDatos)
{
    WSADATA wsa;
    int retorno;

    retorno = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (retorno) {

        printf("WSAStartup() %d\n", retorno);
        return retorno;
    }

    retorno = bdatos_iniciar(bDatos);

    return retorno;
}

SOCKET servidor_crear_socket()
{
    SOCKET skt = socket(AF_INET, SOCK_STREAM, 0);
    if (skt == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PUERTO);

    if (bind(skt, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(skt);
        return INVALID_SOCKET;
    }

    if (listen(skt, 1) == SOCKET_ERROR) {
        closesocket(skt);
        return INVALID_SOCKET;
    }

    return skt;
}

void servidor_procesar_solicitud(tBDatos *bDatos, const char *solicitud, char *respuesta)
{
    int retorno;

    retorno = bdatos_procesar_solcitud(bDatos, solicitud);

    snprintf(respuesta, TAM_BUFFER, "%s", retorno == ERROR_TODO_OK ? "Solicitud completada" : "Error");
}











