#ifndef WIDGET_H_INCLUDED
#define WIDGET_H_INCLUDED

#include <SDL.h>
#include <SDL_ttf.h>

typedef enum {
    WIDGET_ERR_TODO_OK,
    WIDGET_INCORRECTO,
} eWidgetErr;

typedef struct sWidget tWidget;

tWidget* widget_crear_campo_texto(SDL_Renderer *renderer, SDL_Point coords, TTF_Font *fuente, SDL_Color cTexto, SDL_Color cFondo, char visible);
tWidget* widget_crear_boton_texto(SDL_Renderer *renderer, SDL_Point coords, const char* texto, TTF_Font *fuente, SDL_Rect rect, SDL_Color cTexto, SDL_Color cFondo, char visible);
tWidget* widget_crear_contador(SDL_Renderer *renderer, SDL_Point coords, SDL_Texture *icono, TTF_Font *fuente, SDL_Color cTexto, int valor, char visible);
tWidget* widget_crear_texto(SDL_Renderer *renderer, const char *texto, SDL_Point coords, TTF_Font *fuente, SDL_Color cTexto, SDL_Color cFondo, char visible);


int widget_modificar_valor(tWidget *widget, void *valor);
void widget_modificar_visibilidad(tWidget *widget, char visible);
void widget_alternar_visibilidad(tWidget *widget);
void widget_dibujar(tWidget *widget);
void widget_destruir(tWidget *widget);
#endif // WIDGET_H_INCLUDED
