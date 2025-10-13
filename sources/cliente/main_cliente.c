#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../../include/cliente/juego.h"
#include "../../include/cliente/logica.h"
#include "../../include/cliente/cliente.h"

#define TITULO_VENTANA "Laberintos y Fantasmas"

int main(int argc, char* argv[])
{
    SOCKET sock;
    char conectado;
    eRetorno ret = ERR_TODO_OK;
    tJuego juego;

    puts("Laberintos y Fantasmas\n");

    if (cliente_inicializar() != 0) {
        puts("Error: No se pudo inicializar Winsock\n");
        return 1;
    }

    sock = cliente_conectar_servidor(IP_SERVIDOR, PUERTO);
    if (sock == INVALID_SOCKET) {
        puts("Error: No se pudo conectar al servidor\nIniciando el juego en modo Offline..\n");
        WSACleanup();
        conectado = 0;
    }else{
        puts("Conectado al servidor.\n");
        conectado = 1;
    }

    ret = juego_inicializar(&juego, TITULO_VENTANA, sock, conectado);
    if (ret != ERR_TODO_OK) {
        puts("Error: Ha fallado la inicializacion del juego");
        return ret;
    }

    ret = juego_ejecutar(&juego);
    if (ret != ERR_TODO_OK) {
        puts("Error: Ha fallado la ejecucion del juego");
        return ret;
    }

    juego_destruir(&juego);
    cliente_destruir_conexion(juego.sock);
    return ret;
}
