#ifndef ASSETS_H_INCLUDED
#define ASSETS_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

typedef enum{
    SONIDO_FANTASMA_01,
    SONIDO_FANTASMA_02,
    SONIDO_FANTASMA_03,
    SONIDO_JUGADOR_CAMINAR,
    SONIDO_JUGADOR_MUERTE_01,
    SONIDO_JUGADOR_MUERTE_02,
    SONIDO_JUGADOR_PREMIO,
    SONIDO_JUGADOR_VIDA,
    SONIDO_MUSICA,
    SONIDO_PUERTA_SALIDA,
    SONIDO_CANTIDAD
}eSonidos;

typedef enum{
    IMAGEN_FANTASMA,
    IMAGEN_JUGADOR,
    IMAGEN_PARED_LAD_GRIS,
    IMAGEN_PISO,
    IMAGEN_PREMIO,
    IMAGEN_PUERTA_ENTRADA,
    IMAGEN_PUERTA_SALIDA,
    IMAGEN_VIDA,
    IMAGEN_CANTIDAD
}eImagenes;

int assets_cargar_imagenes(SDL_Renderer *renderer, SDL_Texture **imagenes);
int assets_cargar_sonidos(Mix_Chunk **sonidos);
int assets_cargar_fuente(TTF_Font **fuente, int tamFuente);

void assets_destuir_imagenes(SDL_Texture **imagenes);
void assets_destuir_sonidos(Mix_Chunk **sonidos);
void assets_destruir_fuente(TTF_Font *fuente);

#endif // ASSETS_H_INCLUDED
