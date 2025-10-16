#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../../include/cliente/juego.h"
#include "../../include/cliente/logica.h"
#include "../../include/cliente/cliente.h"
#include "../../include/comun/mensaje.h"

#define TITULO_VENTANA "Laberintos y Fantasmas"

int main(int argc, char* argv[])
{
    SOCKET sock;
    char conectado;
    eRetorno ret = ERR_TODO_OK;
    tJuego juego;

    mensaje_titulo("Laberintos y Fantasmas");

    if (cliente_inicializar() != 0) {
        mensaje_error("No se pudo inicializar Winsock.");
        juego_destruir(&juego);
        return 1;
    }

    sock = cliente_conectar_servidor(IP_SERVIDOR, PUERTO);
    if (sock == INVALID_SOCKET) {
        mensaje_error("No se pudo conectar al servidor.");
        mensaje_advertencia("Iniciando el juego en modo Offline.");
        WSACleanup();
        conectado = SESION_OFFLINE;
    }else{
        mensaje_todo_ok("Conectado al servidor.");
        conectado = SESION_ONLINE;
    }

    ret = juego_inicializar(&juego, TITULO_VENTANA, sock, conectado);
    if (ret != ERR_TODO_OK) {
        mensaje_error("Ha fallado la inicializacion del juego");
        juego_destruir(&juego);
        return ret;
    }

    ret = juego_ejecutar(&juego);
    if (ret != ERR_TODO_OK) {
        mensaje_error("Ha fallado la ejecucion del juego");
        juego_destruir(&juego);
        return ret;
    }

    juego_destruir(&juego);
    cliente_destruir_conexion(juego.sock);
    return ret;
}
