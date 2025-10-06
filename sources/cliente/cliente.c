#include "../../include/cliente/cliente.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/protocolo.h"
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

void cliente_cerrar_conexion(SOCKET sock)
{
    closesocket(sock);
    WSACleanup();
}

int cliente_enviar_solicitud(SOCKET sock, const char *solicitud)
{
    if (send(sock, solicitud, strlen(solicitud), 0) < 0) {
        return -1;
    }
    return 0;
}

int cliente_recibir_respuesta(SOCKET sock, char *respuesta, int tamBuffer)
{
    int bytesRecibidos = recv(sock, respuesta, tamBuffer - 1, 0);

    if (bytesRecibidos <= 0) {
        return -1;
    }

    respuesta[bytesRecibidos] = '\0';
    return 0;
}

int cliente_recibir_datos(SOCKET sock, char *bufferDatos, int bytesEsperados)
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

int cliente_ejecutar_solicitud(SOCKET sock, const char *solicitud, int* cantRegistros, char** bufferDatos)
{
    char respuesta[TAM_BUFFER], mensaje[TAM_BUFFER];
    int codigoRetorno = -1, tamRegistro = 0;

    *cantRegistros = 0;
    *bufferDatos = NULL;
    mensaje[0] = '\0';

    if (cliente_enviar_solicitud(sock, solicitud) != 0) {
        strncpy(mensaje, "Error al enviar la solicitud al servidor", TAM_BUFFER - 1);
        return CE_ERR_ENVIO_SOLICITUD;
    }

    if (cliente_recibir_respuesta(sock, respuesta, sizeof(respuesta)) != 0) {
        strncpy(mensaje, "Error al recibir la respuesta del servidor", TAM_BUFFER - 1);
        return CE_ERR_RECEPCION_RESPUESTA;
    }

    sscanf(respuesta, "%d;%[^;];%d;%d\n", &codigoRetorno, mensaje, cantRegistros, &tamRegistro);

    printf(COLOR_AMARILLO "Respuesta:" COLOR_RESET "retorno: %d,%s Mensaje: %s\n", codigoRetorno, codigoRetorno > BD_ERROR_SIN_RESULTADOS ? COLOR_ROJO:COLOR_VERDE, mensaje);

    if (codigoRetorno == BD_DATOS_OBTENIDOS && *cantRegistros > 0 && tamRegistro > 0)
    {
        int totalBytesDatos = (*cantRegistros) * tamRegistro;
        *bufferDatos = (char*)malloc(totalBytesDatos);
        if (!*bufferDatos) {
            return CE_ERR_SIN_MEM;
        }

        if (cliente_recibir_datos(sock, *bufferDatos, totalBytesDatos) != 0) {
            free(*bufferDatos);
            *bufferDatos = NULL;
            return CE_ERR_RECEPCION_DATOS;
        }

        return CE_DATOS;

    } else if (codigoRetorno == BD_ERROR_SIN_RESULTADOS) {

        return CE_SIN_RESULTADOS;
    }

    return CE_ERR_SERVIDOR;
}


