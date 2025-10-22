#include "../../include/comun/cliente.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/protocolo.h"
#include "../../include/comun/mensaje.h"
#include <stdio.h>
#include <string.h>

static int _cliente_recibir(SOCKET sock, char *respuesta, int tamBuffer);

int cliente_inicializar()
{
    WSADATA wsa;
    int retorno;

    retorno = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (retorno) {
        mensaje_error("Fallo en WSAStartup()");
    }

    return retorno;
}

SOCKET cliente_conectar_servidor(const char *ipServidor, int puerto)
{
    struct sockaddr_in direccionSocket;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    direccionSocket.sin_family = AF_INET;
    direccionSocket.sin_port = htons(puerto);
    direccionSocket.sin_addr.s_addr = inet_addr(ipServidor);

    if (connect(sock, (struct sockaddr*)&direccionSocket, sizeof(direccionSocket)) < 0) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

void cliente_destruir_conexion(SOCKET sock)
{
    closesocket(sock);
    WSACleanup();
}

int cliente_enviar_solicitud(SOCKET sock, const char *solicitud)
{
    unsigned bytesEnviados = 0, bytesRestantes = strlen(solicitud);

    while (bytesRestantes != 0) {

        int cantBytes = send(sock, solicitud + bytesEnviados, bytesRestantes, 0);
        if (cantBytes < 0) {
            mensaje_error("La solcitud al servidor ha fallado");
            closesocket(sock);
            return CE_ERR_SOCKET;
        }

        bytesEnviados += cantBytes;
        bytesRestantes -= cantBytes;
    }

    return CE_TODO_OK;
}

int cliente_recibir_respuesta(SOCKET sock, tCola *colaRespuestas)
{
    char respuesta[TAM_BUFFER], *cursor;
    int cantReg = 0, codigoRetorno;

    if (_cliente_recibir(sock, respuesta, sizeof(respuesta)) != 0) {
        mensaje_error("El servidor no ha respondido");
        closesocket(sock);
        return CE_ERR_SOCKET;
    }

    cola_encolar(colaRespuestas, respuesta, strlen(respuesta) + 1);

    sscanf(respuesta, "%d", &codigoRetorno);
    cursor = strrchr(respuesta, ';');
    if (cursor) {
        sscanf(cursor + 1, "%d", &cantReg);
    }

    if (codigoRetorno == BD_DATOS_OBTENIDOS && cantReg > 0) {

        for (int i = 0; i < cantReg; i++) {

            if (_cliente_recibir(sock, respuesta, sizeof(respuesta)) != CE_TODO_OK) {
                mensaje_error("No se pudo recibir los datos.");
                return CE_ERR_SOCKET;
            }

            cola_encolar(colaRespuestas, respuesta, strlen(respuesta) + 1);
        }

        return CE_DATOS;
    }

    if (codigoRetorno == BD_TODO_OK) {
        return CE_TODO_OK;
    }

    return CE_ERR_SERVIDOR;
}

static int _cliente_recibir(SOCKET sock, char *respuesta, int tamBuffer)
{
    int bytesRecibidos = recv(sock, respuesta, tamBuffer - 1, 0);

    if (bytesRecibidos <= 0) {
        return CE_SIN_RESPUESTA;
    }

    respuesta[bytesRecibidos] = '\0';
    return CE_TODO_OK;
}
