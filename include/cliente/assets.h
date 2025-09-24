#ifndef ASSETS_H_INCLUDED
#define ASSETS_H_INCLUDED

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

typedef enum {
    //SONIDO_FANTASMA_01,
    //SONIDO_FANTASMA_02,
    SONIDO_FANTASMA_03,
    SONIDO_JUGADOR_MOV,
    //SONIDO_JUGADOR_MUERTE_01,
    //SONIDO_JUGADOR_MUERTE_02,
    //SONIDO_JUGADOR_PREMIO,
    //SONIDO_JUGADOR_VIDA,
    SONIDO_MENU_CONFIRMAR,
    SONIDO_MENU_MOVIMIENTO,
    //SONIDO_MUSICA,
    //SONIDO_PUERTA_SALIDA,
    SONIDO_CANTIDAD
} eSonidos;

typedef enum {
    IMAGEN_JUGADOR,
    IMAGEN_FANTASMA_01,
    IMAGEN_FANTASMA_02,
    IMAGEN_FANTASMA_03,
    IMAGEN_FANTASMA_04,
    IMAGEN_PREMIO,
    IMAGEN_VIDA,
    IMAGEN_TSET_CASTILLO,
    IMAGEN_ICO_VIDAS,
    IMAGEN_ICO_PREMIOS,
    IMAGEN_CANTIDAD
} eImagen;

int assets_cargar_imagenes(SDL_Renderer *renderer, SDL_Texture **imagenes);
int assets_cargar_sonidos(Mix_Chunk **sonidos);
int assets_cargar_fuente(TTF_Font **fuente, int tamFuente);

void assets_destuir_imagenes(SDL_Texture **imagenes);
void assets_destuir_sonidos(Mix_Chunk **sonidos);
void assets_destruir_fuente(TTF_Font *fuente);

#endif // ASSETS_H_INCLUDED
