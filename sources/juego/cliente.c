#include "../../include/cliente/cliente.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/protocolo.h"
#include "../../include/comun/mensaje.h"
#include <stdio.h>
#include <string.h>

static int _cliente_recibir_respuesta(SOCKET sock, char *respuesta, int tamBuffer);
static int _cliente_recibir_datos(SOCKET sock, char *bufferDatos, int bytesEsperados);


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

void cliente_destruir_conexion(SOCKET sock)
{
    closesocket(sock);
    WSACleanup();
}

int cliente_enviar_solicitud(SOCKET sock, const char *solicitud)
{
    if (send(sock, solicitud, strlen(solicitud), 0) < 0) {
        mensaje_error("La solcitud al servidor ha fallado");
        closesocket(sock);
        return CE_ERR_SOCKET;
    }
    return CE_TODO_OK;
}

int cliente_recibir_respuesta(SOCKET sock, tCola *colaRespuestas)
{
    char respuesta[TAM_BUFFER], *datos = NULL, *cursor;
    int cantReg, tamReg;

    if (_cliente_recibir_respuesta(sock, respuesta, sizeof(respuesta)) != 0) {
        mensaje_error("El servidor no ha respondido");
        closesocket(sock);
        return CE_ERR_SOCKET;
    }

    cola_encolar(colaRespuestas, respuesta, strlen(respuesta));

    cursor = strchr(respuesta, '\n');
    if (!cursor) {
        return CE_ERR_RESPUESTA_CORRUPTA;
    }
    *cursor = '\0';

    cursor = strrchr(respuesta, ';');
    sscanf(cursor + 1, "%d", &tamReg);
    *cursor = '\0';

    cursor = strrchr(respuesta, ';');
    sscanf(cursor + 1, "%d", &cantReg);
    *cursor = '\0';

    if (cantReg > 0 && tamReg > 0) {

        unsigned tamDatos = (cantReg) * (tamReg);
        datos = (char*)malloc(tamDatos);
        if (!datos) {
            return CE_ERR_SIN_MEM;
        }

        if (_cliente_recibir_datos(sock, datos, tamDatos) != 0) {
            free(datos);
            datos = NULL;
            return CE_ERR_SOCKET;
        }

        cola_encolar(colaRespuestas, &datos, sizeof(char*));

        return CE_DATOS;
    }

    return CE_ERR_SERVIDOR;
}



static int _cliente_recibir_respuesta(SOCKET sock, char *respuesta, int tamBuffer)
{
    int bytesRecibidos = recv(sock, respuesta, tamBuffer - 1, 0);

    if (bytesRecibidos <= 0) {
        return -1;
    }

    respuesta[bytesRecibidos] = '\0';
    return 0;
}

static int _cliente_recibir_datos(SOCKET sock, char *bufferDatos, int bytesEsperados)
{
    int bytesRestantes = bytesEsperados;
    char *pBuffer = bufferDatos;

    while (bytesRestantes > 0) {

        int leidos = recv(sock, pBuffer, bytesRestantes, 0);

        if (leidos <= 0) {
            return -1;
        }

        pBuffer += leidos;
        bytesRestantes -= leidos;
    }
    return 0;
}
