#include <stdio.h>
#include <stdlib.h>
#include "../../include/cliente/juego.h"
#include "../../include/cliente/logica.h"
#include "../../include/comun/retorno.h"
#include "../../include/cliente/cliente.h"

#define TITULO_VENTANA "Laberintos y Fantasmas"

int main(int argc, char* argv[])
{
    eRetorno ret = TODO_OK;
    tJuego juego;

    puts("Laberintos y Fantasmas\n");

    if (cliente_inicializar() != 0) {

        puts("Error: No se pudo inicializar Winsock\n");
        return 1;
    }

    SOCKET sock = cliente_conectar_servidor(IP_SERVIDOR, PUERTO);
    if (sock == INVALID_SOCKET) {

        puts("Error: No se pudo conectar al servidor");
        puts("Iniciando el juego en modo Offline..\n");
        WSACleanup();

    }else{

        puts("Conectado al servidor.\n");
    }

    ret = juego_inicializar(&juego, TITULO_VENTANA);
    if (ret != TODO_OK) {

        puts("Error: Ha fallado la inicializacion del juego");
        return ret;
    }

    juego_ejecutar(&juego);
    if (ret != TODO_OK) {

        puts("Error: Ha fallado la ejecucion del juego");
        return ret;
    }

    juego_destruir(&juego);
    cliente_cerrar_conexion(sock);

    return ret;
}
