#include <stdio.h>
#include <stdlib.h>
#include "../../include/servidor/servidor.h"

#define SOCKET_ERR -10


int main()
{
    tBDatos bDatos;
    int servidorRet = SERVIDOR_TODO_OK;
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    char buffer[TAM_BUFFER];
    char respuesta[TAM_BUFFER];
    int bytesRecibidos;

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
//    int retorno;
//
//    retorno = bdatos_procesar_solcitud(&bDatos, "CREAR TABLA jugadores (idJugador ENTERO PK AI, username TEXTO(16), record ENTERO, cantPartidas ENTERO)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo CREAR");
//
//    retorno = bdatos_procesar_solcitud(&bDatos, "ABRIR TABLA jugadores");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo ABRIR");

//    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (username PEPE, record 15, cantPartidas 5)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");
//    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (username PEPE, record 22, cantPartidas 51)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");
//    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (username PEPE, record 150, cantPartidas 111)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");
//    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (username PANCHO, record 40, cantPartidas 3)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");
//    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (username ANA, record 0, cantPartidas 0)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");
//    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (username LALO, record 6, cantPartidas 2)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");
//    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (username MANU, record 3, cantPartidas 1)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");
//    retorno = bdatos_procesar_solcitud(&bDatos, "INSERTAR EN jugadores (username LITO, record 100, cantPartidas 11)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");

//    retorno = bdatos_procesar_solcitud(&bDatos, "SELECCIONAR DESDE jugadores DONDE (username IGUAL PEPE)");
//    if (retorno != ERROR_TODO_OK) puts("No se pudo INSERTAR");
//
//    retorno = bdatos_cerrar(&bDatos);
//    if (retorno != ERROR_TODO_OK) puts("No se pudo CERRAR");
    /// TEST

    SOCKET clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_size);
    if (clientSocket == INVALID_SOCKET) {

        printf("Error en accept()\n");
        closesocket(serverSocket);
        WSACleanup();
        return SOCKET_ERR;
    }

    printf("Cliente conectado.\n");

    while ((bytesRecibidos = recv(clientSocket, buffer, TAM_BUFFER - 1, 0)) > 0) {

        buffer[bytesRecibidos] = '\0';
        printf("Recibido: %s\n", buffer);
        servidor_procesar_solicitud(&bDatos, buffer, respuesta);
        send(clientSocket, respuesta, strlen(respuesta), 0);
        printf("Enviado:  %s\n", respuesta);
    }

    printf("Conexion cerrada.\n");
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    bdatos_cerrar(&bDatos);
    return 0;
}
