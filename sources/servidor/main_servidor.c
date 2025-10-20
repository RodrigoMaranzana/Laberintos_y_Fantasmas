#include <stdio.h>
#include <stdlib.h>
#include "../../include/servidor/servidor.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/mensaje.h"

#define SOCKET_ERR -10

int main()
{
    tBDatos bDatos;
    u_long modo = 1; // para que la conexion sea no bloqueante
    struct sockaddr_in client_addr;
    int servidorRet = SERVIDOR_BD_TODO_OK, client_addr_size = sizeof(client_addr), bytesRecibidos;
    char buffer[TAM_BUFFER];
    SOCKET escuchaCliente, escuchaTerminal, conexionCliente, conexionTerminal;

    if ((servidorRet = servidor_inicializar(&bDatos)) != SERVIDOR_BD_TODO_OK) {
        mensaje_error("No se pudo inicializar Winsock");
        return servidorRet;
    }

    escuchaCliente = servidor_crear_socket(PUERTO);
    escuchaTerminal = servidor_crear_socket(PUERTO_TERMINAL);
    if (escuchaCliente == INVALID_SOCKET || escuchaTerminal == INVALID_SOCKET) {
        mensaje_error("No se pudieron crear los sockets");
        WSACleanup();
        return SOCKET_ERR;
    }

    ioctlsocket(escuchaCliente, FIONBIO, &modo);
    ioctlsocket(escuchaTerminal, FIONBIO, &modo);
    conexionCliente = INVALID_SOCKET;
    conexionTerminal = INVALID_SOCKET;

    mensaje_titulo("LyfDB - Monitor del Servicio");
    printf(FONDO_AMARILLO "Escuchando puertos %d (Cliente) y %d (Terminal)...\n" FONDO_NEGRO, PUERTO, PUERTO_TERMINAL);

    while (1) {

        if (conexionCliente == INVALID_SOCKET) {

            conexionCliente = accept(escuchaCliente, (struct sockaddr *)&client_addr, &client_addr_size);

            if (conexionCliente != INVALID_SOCKET) {

                mensaje_color(FONDO_VERDE, "Cliente conectado.");
                ioctlsocket(conexionCliente, FIONBIO, &modo);
            }
        }

        if (conexionTerminal == INVALID_SOCKET) {

            conexionTerminal = accept(escuchaTerminal, (struct sockaddr *)&client_addr, &client_addr_size);

            if (conexionTerminal != INVALID_SOCKET) {

                mensaje_color(FONDO_AZUL, "Terminal conectado.");
                ioctlsocket(conexionTerminal, FIONBIO, &modo);
            }
        }

        if (conexionCliente != INVALID_SOCKET) {

            bytesRecibidos = recv(conexionCliente, buffer, TAM_BUFFER - 1, 0);

            if (bytesRecibidos > 0) {

                buffer[bytesRecibidos] = '\0';
                mensaje_color(FONDO_CIAN, "[Cliente] %s");
                servidor_procesar_solicitud(&bDatos, &conexionCliente, buffer);

            } else if (bytesRecibidos == 0 || (bytesRecibidos == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {

                mensaje_color(FONDO_MAGENTA, "Cliente desconectado.");
                closesocket(conexionCliente);
                conexionCliente = INVALID_SOCKET;
            }
        }

        if (conexionTerminal != INVALID_SOCKET) {

            bytesRecibidos = recv(conexionTerminal, buffer, TAM_BUFFER - 1, 0);

            if (bytesRecibidos > 0) {

                buffer[bytesRecibidos] = '\0';
                mensaje_color(FONDO_CIAN, "[Terminal] %s");
                servidor_procesar_solicitud(&bDatos, &conexionTerminal, buffer);

            } else if (bytesRecibidos == 0 || (bytesRecibidos == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {

                mensaje_color(FONDO_MAGENTA, "Terminal desconectada.");
                closesocket(conexionTerminal);
                conexionTerminal = INVALID_SOCKET;
            }
        }

        Sleep(1);
    }

    closesocket(escuchaCliente);
    closesocket(escuchaTerminal);
    closesocket(conexionCliente);
    closesocket(conexionTerminal);
    WSACleanup();
    bdatos_apagar(&bDatos);
    return 0;
}
