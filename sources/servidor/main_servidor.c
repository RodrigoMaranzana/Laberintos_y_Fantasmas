#include <stdio.h>
#include <stdlib.h>
#include "../../include/servidor/servidor.h"

#define SOCKET_ERR -10


int main()
{
    tBDatos bDatos;
    int servidorRet = SERVIDOR_TODO_OK;

    if ((servidorRet = servidor_inicializar(&bDatos)) != SERVIDOR_TODO_OK) {

        puts("Error: No se pudo inicializar Winsock");
        return servidorRet;
    }

    SOCKET serverSocket = servidor_crear_socket();
    if (serverSocket == INVALID_SOCKET) {

        puts("Error: No se pudo crear el socket del servidor");
        WSACleanup();
        return SOCKET_ERR;
    }

    printf("Servidor escuchando en puerto %d...\n", PUERTO);

    /// TEST
    int retorno;

    retorno = bdatos_procesar_solcitud(&bDatos, "CREAR TABLA jugadores (idJugador ENTERO, nombre TEXTO(16), puntajeMax ENTERO, PK(idJugador))");
    if (retorno != TODO_OK) puts("No se pudo CREAR");

    retorno = bdatos_procesar_solcitud(&bDatos, "ABRIR TABLA jugadores");
    if (retorno != TODO_OK) puts("No se pudo ABRIR");

    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (idJugador 21, nombre PEPE, puntajeMax 15)");
    if (retorno != TODO_OK) puts("No se pudo INSERTAR");

    retorno = bdatos_cerrar(&bDatos);
    if (retorno != TODO_OK) puts("No se pudo CERRAR");

    /// TEST

    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);

    SOCKET clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_size);
    if (clientSocket == INVALID_SOCKET) {

        printf("Error en accept()\n");
        closesocket(serverSocket);
        WSACleanup();
        return SOCKET_ERR;
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
    return 0;
}
