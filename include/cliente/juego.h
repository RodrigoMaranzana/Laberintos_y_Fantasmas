#ifndef MOTOR_H_INCLUDED
#define MOTOR_H_INCLUDED

#include <SDL.h>
#include <SDL_mixer.h>
#include "../../include/cliente/logica.h"
#include "../../include/cliente/texto.h"
#include "../../include/cliente/menu.h"

#define SDL_COLOR_BLANCO ((SDL_Color){255, 255, 255, 255})

typedef enum {
    JUEGO_NO_INICIADO,
    JUEGO_CORRIENDO,
    JUEGO_CERRANDO
} eJuegoEstado;

typedef struct {
    SDL_Window *ventana;
    SDL_Renderer *renderer;
    const char *tituloVentana;
    unsigned anchoRes;
    unsigned altoRes;

    SDL_Texture **imagenes;
    Mix_Chunk **sonidos;
    TTF_Font *fuente;

    tMenu *menu; //tMenu **menus; // Si se necesita mas de un menu
    tLogica logica;
    eJuegoEstado estado;
} tJuego;

int juego_inicializar(tJuego *juego, const char *tituloVentana);
int juego_ejecutar(tJuego *juego);
void juego_destruir(tJuego *juego);

#endif // MOTOR_H_INCLUDED
