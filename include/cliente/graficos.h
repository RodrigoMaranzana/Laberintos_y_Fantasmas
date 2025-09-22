#ifndef GRAFICOS_H_INCLUDED
#define GRAFICOS_H_INCLUDED

#include <SDL.h>

typedef struct {
    SDL_Texture *textura;
    SDL_Rect rect;
} tTextura;

typedef void (*tEfectoGrafico)(SDL_Texture *textura);

SDL_Texture* graficos_crear_textura(SDL_Renderer *renderer, int ancho, int alto, int modoAcceso, unsigned formato);
void graficos_dibujar_textura(SDL_Texture *textura, SDL_Renderer *renderer, SDL_Rect *rectFuente, SDL_Rect *rectDestino, tEfectoGrafico efecto);
void graficos_dibujar_borde(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color);
void graficos_dibujar_relleno(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color);

#endif // GRAFICOS_H_INCLUDED
