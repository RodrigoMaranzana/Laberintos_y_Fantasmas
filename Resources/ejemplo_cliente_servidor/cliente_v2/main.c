#include <stdio.h>
#include <stdlib.h>

#include "cliente_lib.h"
#include <stdio.h>
#include <string.h>

int main() {

    if (init_winsock() != 0) {
        printf("Error al inicializar Winsock\n");
        return 1;
    }

    SOCKET sock = connect_to_server(SERVER_IP, PORT);
    if (sock == INVALID_SOCKET) {
        printf("No se pudo conectar al servidor\n");
        WSACleanup();
        return 1;
    }

    printf("Conectado al servidor.\n");
    printf("Formato: OPERACION texto (MAYUS/MINUS/INV)\n");

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    while (1) {
        printf("> ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;

        buffer[strcspn(buffer, "\n")] = '\0'; // quitar salto de línea
        if (strlen(buffer) == 0) continue;

        if (send_request(sock, buffer, response) == 0) {
            printf("Respuesta: %s\n", response);
        } else {
            printf("Error al enviar o recibir datos\n");
            break;
        }
    }

    close_connection(sock);
    return 0;
}
