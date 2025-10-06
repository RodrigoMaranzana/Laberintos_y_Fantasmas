#include <stdio.h>
#include <stdlib.h>
#include "../../include/cliente/juego.h"
#include "../../include/cliente/logica.h"
#include "../../include/cliente/cliente.h"

#define TITULO_VENTANA "Laberintos y Fantasmas"

#pragma pack(push, 1)
typedef struct {
    char username[16];
    int record;
    int cantPartidas;
} tJugador;
typedef struct {
    int idPartida;
    char username[16];
    int puntaje;
} tPartida;
#pragma pack(pop)

static void _lote_pruebas_comunicacion_servidor(SOCKET *sock);

int main(int argc, char* argv[])
{
    eRetorno ret = ERR_TODO_OK;
    tJuego juego;

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

    _lote_pruebas_comunicacion_servidor(&juego.sock);

    ///


    ret = juego_inicializar(&juego, TITULO_VENTANA);
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
    cliente_cerrar_conexion(juego.sock);
    return ret;
}










static void _lote_pruebas_comunicacion_servidor(SOCKET *sock)
{
    char *datos = NULL;
    int cantidad = 0, retorno, i;

    /* CREACIÓN DE TABLAS - CASOS VALIDOS */

    retorno = cliente_ejecutar_solicitud(*sock, "CREAR jugadores (username TEXTO(16) PK, record ENTERO, cantPartidas ENTERO)", &cantidad, &datos);
    if(datos) free(datos);

    retorno = cliente_ejecutar_solicitud(*sock, "CREAR partidas (idPartida ENTERO PK AI, username TEXTO(16), puntaje ENTERO)", &cantidad, &datos);
    if(datos) free(datos);

    /* CREACIÓN DE TABLAS - CASOS INVALIDOS */

    retorno = cliente_ejecutar_solicitud(*sock, "CREAR jugadores (dummy TEXTO(1))", &cantidad, &datos);
    if(datos) free(datos);

    retorno = cliente_ejecutar_solicitud(*sock, "CREAR puntajes (valor ENTERO)", &cantidad, &datos);
    if(datos) free(datos);


    /* INSERCIÓN DE DATOS - CASOS VALIDOS */

    retorno = cliente_ejecutar_solicitud(*sock, "INSERTAR jugadores (username PEPE, record 100, cantPartidas 3)", &cantidad, &datos);
    if(datos) free(datos);

    retorno = cliente_ejecutar_solicitud(*sock, "INSERTAR jugadores (username PANCHO, record 60, cantPartidas 11)", &cantidad, &datos);
    if(datos) free(datos);

    retorno = cliente_ejecutar_solicitud(*sock, "INSERTAR jugadores (cantPartidas 5, username ANA, record 250)", &cantidad, &datos);
    if(datos) free(datos);

    retorno = cliente_ejecutar_solicitud(*sock, "INSERTAR partidas (username PEPE, puntaje 100)", &cantidad, &datos);
    if(datos) free(datos);

    retorno = cliente_ejecutar_solicitud(*sock, "INSERTAR partidas (puntaje 120, username ANA)", &cantidad, &datos);
    if(datos) free(datos);

    retorno = cliente_ejecutar_solicitud(*sock, "INSERTAR partidas (username PEPE, puntaje 50)", &cantidad, &datos);
    if(datos) free(datos);


    /* INSERCIÓN DE DATOS - CASOS INVALIDOS */

    retorno = cliente_ejecutar_solicitud(*sock, "INSERTAR jugadores (username PEPE, record 999, cantPartidas 9)", &cantidad, &datos);
    if(datos) free(datos);

    retorno = cliente_ejecutar_solicitud(*sock, "INSERTAR partidas (idPartida 21, puntaje 120, username ANA)", &cantidad, &datos);
    if(datos) free(datos);


    /* SELECCIÓN DE DATOS */

    retorno = cliente_ejecutar_solicitud(*sock, "SELECCIONAR jugadores (username IGUAL ANA)", &cantidad, &datos);
    if (retorno == 1 && cantidad > 0 && datos != NULL) {

        printf("Datos Recibidos:\n");
        for (i = 0; i < cantidad; i++) {
            tJugador* j = ((tJugador*)datos) + i;
            printf(COLOR_AZUL"\t- Jugador: %s, Record: %d, Partidas: %d\n"COLOR_RESET, j->username, j->record, j->cantPartidas);
        }

        free(datos);
    }

    retorno = cliente_ejecutar_solicitud(*sock, "SELECCIONAR jugadores (username IGUAL HOMERO)", &cantidad, &datos);
    if (retorno == CE_TODO_OK && cantidad > 0 && datos != NULL) {

        printf("Datos Recibidos:\n");
        for (i = 0; i < cantidad; i++) {
            tJugador* j = ((tJugador*)datos) + i;
            printf(COLOR_AZUL"\t- Jugador: %s, Record: %d, Partidas: %d\n"COLOR_RESET, j->username, j->record, j->cantPartidas);
        }

        free(datos);
    }

    retorno = cliente_ejecutar_solicitud(*sock, "SELECCIONAR partidas (idPartida IGUAL 2)", &cantidad, &datos);
    if (retorno == CE_TODO_OK && cantidad > 0 && datos != NULL) {

        printf("Datos Recibidos:\n");
        for (i = 0; i < cantidad; i++) {
            tPartida* p = ((tPartida*)datos) + i;
            printf(COLOR_AZUL"\t- Partida: %d, Jugador: %s, Puntaje: %d\n"COLOR_RESET, p->idPartida, p->username, p->puntaje);
        }

        free(datos);
    }

    retorno = cliente_ejecutar_solicitud(*sock, "SELECCIONAR partidas (username IGUAL PEPE)", &cantidad, &datos);
    if (retorno == CE_TODO_OK && cantidad > 0 && datos != NULL) {

        printf("Datos Recibidos:\n");
        for (i = 0; i < cantidad; i++) {
            tPartida* p = ((tPartida*)datos) + i;
            printf(COLOR_AZUL"\t- Partida: %d, Jugador: %s, Puntaje: %d\n"COLOR_RESET, p->idPartida, p->username, p->puntaje);
        }

        free(datos);
    }
}
