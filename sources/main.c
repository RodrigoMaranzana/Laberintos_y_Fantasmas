#include <stdio.h>
#include <stdlib.h>
#include "..\include\juego.h"
#include "..\include\logica.h"
#include "..\include\retornos.h"

#define TITULO_VENTANA "Laberintos y Fantasmas"
//#define SDL
//#define CONSOLA

/// MACROS TEMPORALES
#define CANT_COLUMNAS 16
#define CANT_FILAS 16

int main(int argc, char* argv[])
{
    // Bloque declarativo
    eRetornos ret = TODO_OK;
    tJuego juego;
    unsigned anchoRes, altoRes;

    puts("Laberintos y Fantasmas\n");

    logica_calc_resolucion(CANT_COLUMNAS, CANT_FILAS, &anchoRes, &altoRes);

    ret = juego_inicializar(&juego, anchoRes, altoRes, TITULO_VENTANA);
    if(ret != TODO_OK){

        puts("Error: Ha fallado la inicializacion del juego");
        return ret;
    }

    juego_ejecutar(&juego);
    if(ret != TODO_OK){

        puts("Error: Ha fallado el juego");
        return ret;
    }

    juego_destruir(&juego);
    return ret;
}
