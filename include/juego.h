#ifndef MOTOR_H_INCLUDED
#define MOTOR_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "../include/logica.h"

typedef enum{
    JUEGO_NO_INICIADO,
    JUEGO_CORRIENDO,
    JUEGO_CERRANDO
}eJuegoEstado;

typedef struct{
    SDL_Window *ventana;
    SDL_Renderer *renderer;
    SDL_Texture *framebuffer;
    SDL_Texture **imgAssets;
    Mix_Chunk **sndAssets;
    eJuegoEstado estado;
    const char *tituloVentana;
    unsigned anchoRes;
    unsigned altoRes;
}tJuego;

int juego_inicializar(tJuego *juego, unsigned anchoRes, unsigned altoRes, const char *tituloVentana);
int juego_ejecutar(tJuego *juego);
void juego_destruir(tJuego *juego);

#endif // MOTOR_H_INCLUDED
