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
        printf(FONDO_ROJO "ERROR: WSAStartup() %d\n" COLOR_RESET, retorno);
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

void servidor_procesar_solicitud(tBDatos *bDatos, SOCKET *sock, const char *solicitud)
{
    int retorno, cantRegistrosDatos, tamRegistroDatos;
    const char *mensaje;
    tLista listaDatos;
    char *serializacionDatos = NULL, *registroDatos = NULL, respuesta[TAM_BUFFER];

    lista_crear(&listaDatos);

    retorno = bdatos_procesar_solcitud(bDatos, solicitud, &listaDatos, &cantRegistrosDatos, &tamRegistroDatos);
    mensaje = bdatos_obtener_mensaje(retorno);

    if (retorno == BD_DATOS_OBTENIDOS) {

        serializacionDatos = malloc(cantRegistrosDatos * tamRegistroDatos);
        if (!serializacionDatos || !(registroDatos = malloc(tamRegistroDatos))) {
            mensaje = bdatos_obtener_mensaje(BD_ERROR_SIN_MEMO);
            free(serializacionDatos);
            lista_vaciar(&listaDatos);
        } else{
            char *pSerializacionDatos = serializacionDatos;
            memset(serializacionDatos, 0, cantRegistrosDatos * tamRegistroDatos);
            while (lista_sacar_primero(&listaDatos, registroDatos, tamRegistroDatos) != LISTA_VACIA) {

                memcpy(pSerializacionDatos, registroDatos, tamRegistroDatos);
                pSerializacionDatos += tamRegistroDatos;
            }
        }

        snprintf(respuesta, TAM_BUFFER, "%d;%s;%d;%d\n", retorno, mensaje, cantRegistrosDatos, tamRegistroDatos);
    } else {

        snprintf(respuesta, TAM_BUFFER, "%d;%s\n", retorno, mensaje);
    }

    send(*sock, respuesta, strlen(respuesta), 0);
    printf(FONDO_AMARILLO "Enviado:" COLOR_RESET " %s\n", respuesta);

    if (serializacionDatos) {

        unsigned tamSerializacion = cantRegistrosDatos * tamRegistroDatos;

        send(*sock, serializacionDatos, tamSerializacion, 0);
        printf(FONDO_BLANCO "Datos enviados:" COLOR_RESET " %d bytes\n", tamSerializacion);
    }

    free(serializacionDatos);
    free(registroDatos);
}











