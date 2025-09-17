#ifndef BDATOS_H_INCLUDED
#define BDATOS_H_INCLUDED
#include <stdio.h>
#include "..\..\include\comun\lista.h"
#include "..\..\include\comun\arbol.h"
#include "..\..\include\comun\comun.h"

#define ARCH_NOMBRE "jugadores.dat"
#define ARCH_IDX_NOMBRE "jugadores.idx"

typedef struct {
    char usuario[TAM_USUARIO];
    long offset;
} tJugador;

typedef struct {
    FILE *arch;
    FILE *archIdx;
    tArbol arbol;
    tJugador jugadorSesion;
} tBDatos;

typedef struct {
    int puntaje;
    int numRonda;
    int cantMovsJugador;
    long offsetSig;
} tPartida;

int bdatos_crear(tBDatos *bDatos);
int bdatos_iniciar(tBDatos *bDatos);
int bdatos_insertar_jugador(tBDatos *bDatos, tJugador *jugador);
int bdatos_insertar_partida(tBDatos *bDatos, const tPartida *partida);
int bdatos_buscar(const tBDatos *bDatos, tJugador *jugador)
;int bdatos_cerrar(tBDatos *bDatos);
int bdatos_cargar_idx_en_arbol(tBDatos *bDatos);

#endif // BDATOS_H_INCLUDED
