#include <stdio.h>
#include <stdlib.h>
#include "../../include/cliente/juego.h"
#include "../../include/cliente/logica.h"
#include "../../include/cliente/cliente.h"

#define TITULO_VENTANA "Laberintos y Fantasmas"

int main(int argc, char* argv[])
{
    eRetorno ret = TODO_OK;
    tJuego juego;
    char buffer[TAM_BUFFER];

    puts("Laberintos y Fantasmas\n");

    if (cliente_inicializar() != 0) {
        puts("Error: No se pudo inicializar Winsock\n");
        return 1;
    }

    juego.sock = cliente_conectar_servidor(IP_SERVIDOR, PUERTO);
    if (juego.sock == INVALID_SOCKET) {
        puts("Error: No se pudo conectar al servidor\nIniciando el juego en modo Offline..\n");
        WSACleanup();
    }else{
        puts("Conectado al servidor.\n");
    }

    /// TEST
    printf("Solicitud del cliente: %s\n", "CREAR TABLA jugadores (idJugador ENTERO PK AI, username TEXTO(16), record ENTERO, cantPartidas ENTERO)");
    cliente_enviar_solicitud(juego.sock, "CREAR TABLA jugadores (idJugador ENTERO PK AI, username TEXTO(16), record ENTERO, cantPartidas ENTERO)", buffer);
    printf("Respuesta del servidor: %s\n", buffer);

    printf("Solicitud del cliente: %s\n", "ABRIR TABLA jugadores");
    cliente_enviar_solicitud(juego.sock, "ABRIR TABLA jugadores", buffer);
    printf("Respuesta del servidor: %s\n", buffer);
    /// TEST

    ret = juego_inicializar(&juego, TITULO_VENTANA);
    if (ret != TODO_OK) {
        puts("Error: Ha fallado la inicializacion del juego");
        return ret;
    }

    ret = juego_ejecutar(&juego);
    if (ret != TODO_OK) {
        puts("Error: Ha fallado la ejecucion del juego");
        return ret;
    }

    juego_destruir(&juego);
    cliente_cerrar_conexion(juego.sock);
    return ret;
}
