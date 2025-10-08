#include <stdio.h>
#include <stdlib.h>
#include "../../include/servidor/servidor.h"
#include "../../include/comun/comun.h"

#define SOCKET_ERR -10

int main()
{
    tBDatos bDatos;
    int servidorRet = SERVIDOR_BD_TODO_OK;
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    char buffer[TAM_BUFFER];
    int bytesRecibidos;

    if ((servidorRet = servidor_inicializar(&bDatos)) != SERVIDOR_BD_TODO_OK) {
        puts(FONDO_ROJO "Error:" COLOR_RESET " No se pudo inicializar Winsock");
        return servidorRet;
    }

    SOCKET serverSocket = servidor_crear_socket();
    if (serverSocket == INVALID_SOCKET) {
        puts(FONDO_ROJO "Error:" COLOR_RESET " No se pudo crear el socket del servidor");
        WSACleanup();
        return SOCKET_ERR;
    }

    printf(FONDO_AMARILLO "Servidor escuchando en puerto %d...\n" FONDO_NEGRO, PUERTO);

    while (1) {

        SOCKET socketCliente = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (socketCliente == INVALID_SOCKET) {
            printf(FONDO_ROJO "Error:" COLOR_RESET " accept() ha fallado\n");
        }

        printf(FONDO_VERDE "Cliente conectado.\n" FONDO_NEGRO);

        while ((bytesRecibidos = recv(socketCliente, buffer, TAM_BUFFER - 1, 0)) > 0) {
            buffer[bytesRecibidos] = '\0';
            printf(FONDO_CIAN "Recibido:" COLOR_RESET " %s\n", buffer);
            servidor_procesar_solicitud(&bDatos, &socketCliente, buffer);
        }

        printf(FONDO_MAGENTA "\n -- Cliente desconectado --" FONDO_NEGRO "\n");
        closesocket(socketCliente);
    }

    closesocket(serverSocket);
    WSACleanup();
    bdatos_apagar(&bDatos);
    return 0;
}
