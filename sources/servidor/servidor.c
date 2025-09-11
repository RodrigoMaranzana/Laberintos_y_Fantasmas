#include "../../include/comun/comun.h"
#include "../../include/servidor/servidor.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Funciones auxiliares
static void to_upper(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = toupper((unsigned char)str[i]);
    }
}

static void to_lower(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower((unsigned char)str[i]);
    }
}

static void reverse(char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}


int servidor_inicializar()
{
    WSADATA wsa;
    int retorno;

    retorno = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (retorno) {

        printf("WSAStartup() %d\n", retorno);
    }

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

    if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        closesocket(s);
        return INVALID_SOCKET;
    }

    if (listen(s, 1) == SOCKET_ERROR)
    {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;
}

void servidor_procesar_solicitud(const char *request, char *response)
{
    char operation[16], text[TAM_BUFFER];
    sscanf(request, "%15s %[^\n]", operation, text);

    if (strcmp(operation, "MAYUS") == 0)
    {
        to_upper(text);
        snprintf(response, TAM_BUFFER, "%s", text);
    }
    else if (strcmp(operation, "MINUS") == 0)
    {
        to_lower(text);
        snprintf(response, TAM_BUFFER, "%s", text);
    }
    else if (strcmp(operation, "INV") == 0)
    {
        reverse(text);
        snprintf(response, TAM_BUFFER, "%s", text);
    }
    else
    {
        snprintf(response, TAM_BUFFER, "Operacion no valida");
    }
}

void servidor_iniciar()
{
    if (servidor_inicializar() != 0) {

        puts("Error: No se pudo inicializar Winsock");
        return;
    }

    SOCKET server_socket = servidor_crear_socket();
    if (server_socket == INVALID_SOCKET) {

        puts("Error: No se pudo crear el socket del servidor");
        WSACleanup();
        return;
    }

    printf("Servidor escuchando en puerto %d...\n", PUERTO);

    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);

    SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_socket == INVALID_SOCKET) {

        printf("Error en accept()\n");
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    printf("Cliente conectado.\n");

    char buffer[TAM_BUFFER];
    char response[TAM_BUFFER];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, TAM_BUFFER - 1, 0)) > 0) {

        buffer[bytes_received] = '\0';
        printf("Recibido: %s\n", buffer);
        servidor_procesar_solicitud(buffer, response);
        send(client_socket, response, strlen(response), 0);
        printf("Enviado:  %s\n", response);
    }

    printf("Conexion cerrada.\n");
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}
