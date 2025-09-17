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
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PUERTO);

    if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    if (listen(s, 1) == SOCKET_ERROR) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;
}

void servidor_procesar_solicitud(tBDatos *bDatos, const char *solicitud, char *respuesta)
{
    int retorno = TODO_OK;
    char comando[TAM_COMANDO_MAX], buffer[TAM_BUFFER];
    sscanf(solicitud, "%[^|]|%[^\n]", comando, buffer);

    if (strcmp(comando, SOLICITUD_INI_SESION) == 0) {

        tJugador jugador;

        puts(SOLICITUD_INI_SESION);

        strncpy(jugador.usuario, buffer, TAM_USUARIO);
        *(jugador.usuario + TAM_USUARIO - 1) = '\0';

        printf("Usuario que intenta iniciar sesion: %s\n", jugador.usuario);

        if (bdatos_buscar(bDatos, &jugador) == TODO_OK) {

            bDatos->jugadorSesion = jugador;
            printf("%s inició sesion correctamente\n", jugador.usuario);

        } else {

            jugador.offset = -1;
            retorno = bdatos_insertar_jugador(bDatos, &jugador);
            printf("%s es un jugador nuevo.\n\nBienvenido.\n\n", jugador.usuario);
        }

        snprintf(respuesta, TAM_BUFFER, "%s", retorno == TODO_OK ? RESPUESTA_OK : RESPUESTA_ERROR);

    } else if (strcmp(comando, SOLICITUD_INS_PARTIDA) == 0) {

        if (!*bDatos->jugadorSesion.usuario) {

            snprintf(respuesta, TAM_BUFFER, "%s", RESPUESTA_ERROR);
            return;
        }

        tPartida partida;

        puts(SOLICITUD_INS_PARTIDA);

        sscanf(buffer, "%d|%d|%d", &partida.puntaje, &partida.numRonda, &partida.cantMovsJugador);
        partida.offsetSig = -1;

        retorno = bdatos_insertar_partida(bDatos, &partida);

        snprintf(respuesta, TAM_BUFFER, "%s", retorno == TODO_OK ? RESPUESTA_OK : RESPUESTA_ERROR);

    } else {

        puts(RESPUESTA_SOLIC_INVALIDA);

        snprintf(respuesta, TAM_BUFFER, RESPUESTA_SOLIC_INVALIDA);
    }
}

void servidor_iniciar()
{
    tBDatos bDatos;

    if (servidor_inicializar(&bDatos) != 0) {

        puts("Error: No se pudo inicializar Winsock");
        return;
    }

    SOCKET serverSocket = servidor_crear_socket();
    if (serverSocket == INVALID_SOCKET) {

        puts("Error: No se pudo crear el socket del servidor");
        WSACleanup();
        return;
    }

    printf("Servidor escuchando en puerto %d...\n", PUERTO);

    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);

    SOCKET clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_size);
    if (clientSocket == INVALID_SOCKET) {

        printf("Error en accept()\n");
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    printf("Cliente conectado.\n");

    char buffer[TAM_BUFFER];
    char response[TAM_BUFFER];
    int bytes_received;

    while ((bytes_received = recv(clientSocket, buffer, TAM_BUFFER - 1, 0)) > 0) {

        buffer[bytes_received] = '\0';
        printf("Recibido: %s\n", buffer);
        servidor_procesar_solicitud(&bDatos, buffer, response);
        send(clientSocket, response, strlen(response), 0);
        printf("Enviado:  %s\n", response);
    }

    printf("Conexion cerrada.\n");
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    bdatos_cerrar(&bDatos);
}



