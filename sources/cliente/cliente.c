#include "../../include/cliente/cliente.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/protocolo.h"
#include "../../include/comun/mensaje.h"
#include <stdio.h>
#include <string.h>

static int _cliente_recibir_respuesta(SOCKET sock, char *respuesta, int tamBuffer);

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

static int _cliente_recibir_respuesta(SOCKET sock, char *respuesta, int tamBuffer)
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

int cliente_enviar_solicitud(SOCKET sock, const char *solicitud)
{
    if (send(sock, solicitud, strlen(solicitud), 0) < 0) {
        mensaje_error("La solcitud al servidor ha fallado");
        closesocket(sock);
        return CE_ERR_SOCKET;
    }
    return CE_TODO_OK;
}

int cliente_recibir_respuesta(SOCKET sock, int *codigoRetorno, char *mensaje, int *cantRegistros, int *tamRegistro, char **bufferDatos)
{
    char mnsj[TAM_BUFFER], respuesta[TAM_BUFFER], *datos = NULL, *cursor;
    int codigoRet, cantReg, tamReg;

    if (_cliente_recibir_respuesta(sock, respuesta, sizeof(respuesta)) != 0) {
        mensaje_error("El servidor no ha respondido");
        closesocket(sock);
        return CE_ERR_SOCKET;
    }

    cursor = strchr(respuesta, '\n');
    if (!*cursor) {
        return ERR_LINEA_LARGA;
    }
    *cursor = '\0';

    cursor = strrchr(respuesta, ';');
    sscanf(cursor + 1, "%d", &tamReg);
    *cursor = '\0';

    cursor = strrchr(respuesta, ';');
    sscanf(cursor + 1, "%d", &cantReg);
    *cursor = '\0';

    cursor = strrchr(respuesta, ';');
    strncpy(mnsj, cursor + 1, TAM_BUFFER);
    mnsj[TAM_BUFFER - 1] = '\0';

    *cursor = '\0';
    sscanf(respuesta, "%d", &codigoRet);

    printf(COLOR_AMARILLO "Respuesta:" COLOR_RESET " retorno: %d, %s Mensaje: %s\n" COLOR_RESET, codigoRet, codigoRet > BD_ERROR_SIN_RESULTADOS ? COLOR_ROJO : COLOR_VERDE, mnsj);

    if (codigoRet == BD_DATOS_OBTENIDOS && cantReg > 0 && tamReg > 0) {

        int tamDatos = (cantReg) * (tamReg);
        datos = (char*)malloc(tamDatos);
        if (!datos) {
            return CE_ERR_SIN_MEM;
        }

        if (cliente_recibir_datos(sock, datos, tamDatos) != 0) {
            free(datos);
            datos = NULL;
            return CE_ERR_SOCKET;
        }

        return CE_DATOS;

    } else if (codigoRet == BD_ERROR_SIN_RESULTADOS) {

        return CE_SIN_RESULTADOS;
    }

    if (codigoRetorno) {
        *codigoRetorno = codigoRet;
    }
    if (mensaje) {
        strcpy(mensaje, mnsj);
    }
    if (cantRegistros) {
        *cantRegistros = cantReg;
    }
    if (bufferDatos) {
        *bufferDatos = datos;
    }

    return CE_ERR_SERVIDOR;
}


