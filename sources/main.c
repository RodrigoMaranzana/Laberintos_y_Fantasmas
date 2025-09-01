#include <stdio.h>
#include <stdlib.h>
#include "..\include\juego.h"
#include "..\include\logica.h"
#include "..\include\retorno.h"

#define TITULO_VENTANA "Laberintos y Fantasmas"

int main(int argc, char* argv[])
{
    eRetorno ret = TODO_OK;
    tJuego juego;

    puts("Laberintos y Fantasmas\n");

    ret = juego_inicializar(&juego, TITULO_VENTANA);
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
