#include "../../include/comun/comun.h"
#include "../../include/comun/mensaje.h"
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

SOCKET servidor_crear_socket(int puerto)
{
    SOCKET skt = socket(AF_INET, SOCK_STREAM, 0);
    if (skt == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(puerto);

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
    int retorno, cantRegistros = 0;
    char respuesta[TAM_BUFFER];
    const char *mensaje;
    tLista listaRegistros;

    printf(FONDO_MAGENTA "Recibido:" COLOR_RESET " %s\n", solicitud);

    lista_crear(&listaRegistros);

    retorno = bdatos_procesar_solicitud(bDatos, solicitud, &listaRegistros, &cantRegistros);
    mensaje = bdatos_obtener_mensaje(retorno);

    snprintf(respuesta, TAM_BUFFER, "%d;%s;%d\n", retorno, mensaje, cantRegistros);
    send(*sock, respuesta, strlen(respuesta), 0);
    printf(FONDO_AMARILLO "Enviado:" COLOR_RESET " %s", respuesta);

    if (retorno == BD_DATOS_OBTENIDOS) {

        char registroBinario[bDatos->tablaAbierta.encabezado.tamRegistro];

        mensaje_color(FONDO_BLANCO, "Enviando %d registro%s.", cantRegistros, cantRegistros > 1 ? "s" : "");

        while (lista_sacar_primero(&listaRegistros, registroBinario, bDatos->tablaAbierta.encabezado.tamRegistro) != LISTA_VACIA) {

            char *registroTexto = bdatos_registro_a_texto(&bDatos->tablaAbierta.encabezado, registroBinario);

            if (registroTexto) {

                mensaje_color(FONDO_CIAN, "%s", registroTexto);
                send(*sock, registroTexto, strlen(registroTexto), 0);
                free(registroTexto);
            }
        }
    }
}









