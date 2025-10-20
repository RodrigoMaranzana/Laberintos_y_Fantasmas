#include "../../include/juego/ventana.h"

#include <math.h>
#include <string.h>

struct sVentana {
    tVentanaAccion accion;
    SDL_Renderer *renderer;
    SDL_Rect dimensiones;
    SDL_Color cFondo;
    unsigned tiempoApertura;
    unsigned tiempoActual;
    float animValor;
    char conSombra;
    char abierta;
};

static float _interpolacion_lineal(float valorInicial, float valorFinal, float factor);
static SDL_Rect _ventana_dibujar(tVentana *ventana);

tVentana* ventana_crear(SDL_Renderer *renderer, tVentanaAccion accion, SDL_Rect dimensiones, SDL_Color cFondo, char conSombra)
{
    tVentana *ventana = malloc(sizeof(tVentana));
    if (!ventana) {
        return NULL;
    }

    ventana->accion = accion;
    ventana->renderer = renderer;
    ventana->dimensiones = dimensiones;
    ventana->cFondo = cFondo;
    ventana->conSombra = conSombra;
    ventana->animValor = 0;
    ventana->tiempoApertura = 0;
    ventana->tiempoActual = 0;
    ventana->abierta = 0;

    if (ventana->accion.crearDatos) {
        if (ventana->accion.crearDatos(ventana->accion.datos) == -1) {
            free(ventana);
            return NULL;
        }
    }

    return ventana;
}

void ventana_destruir(tVentana *ventana)
{
    if (ventana->accion.destruirDatos && ventana->accion.datos) {
        ventana->accion.destruirDatos(ventana->accion.datos);
    }

    free(ventana);
    ventana = NULL;
}

void ventana_abrir(tVentana *ventana)
{
    ventana->tiempoApertura = SDL_GetTicks();
    ventana->animValor = 0;
    ventana->abierta = 1;
}

void ventana_cerrar(tVentana *ventana)
{
    ventana->abierta = 0;
}

void ventana_dibujar(tVentana *ventana)
{
    if (!ventana->abierta) {
        return;
    }

    SDL_Rect area = _ventana_dibujar(ventana);

    SDL_RenderSetViewport(ventana->renderer, &area);

    if (ventana->accion.dibujar) {
        ventana->accion.dibujar(ventana->accion.datos);
    }

    SDL_RenderSetViewport(ventana->renderer, NULL);
}

void ventana_actualizar(tVentana *ventana, SDL_Event *event)
{
    if (!ventana->abierta) {
        return;
    }

    _ventana_dibujar(ventana);

    if (ventana->accion.actualizar) {

        ventana->accion.actualizar(event, ventana->accion.datos);
    }
}

static SDL_Rect _ventana_dibujar(tVentana *ventana)
{
    SDL_Rect rect = ventana->dimensiones, rectActual = ventana->dimensiones;
    SDL_Color cBorde;

    if (ventana->animValor != 1) {

        float factor;
        ventana->tiempoActual = SDL_GetTicks();
        factor = (float)(ventana->tiempoActual - ventana->tiempoApertura) / 250.0;
        ventana->animValor = _interpolacion_lineal(0, 1, factor);

        rectActual.x = ventana->dimensiones.x + (ventana->dimensiones.w / 2);
        rectActual.y = ventana->dimensiones.y + (ventana->dimensiones.h / 2);
        rectActual.w = ventana->dimensiones.w * ventana->animValor;
        rectActual.h = ventana->dimensiones.h * ventana->animValor;
        rect.x = (rectActual.x - (rectActual.w / 2));
        rect.y = (rectActual.y - (rectActual.h / 2));
        rect.w = rectActual.w;
        rect.h = rectActual.h;
    }

    if (ventana->conSombra) {
        SDL_Rect sombra = rect;
        sombra.x += 16;
        sombra.y += 16;

        SDL_SetRenderDrawColor(ventana->renderer, 0, 0, 0, 128);
        SDL_RenderFillRect(ventana->renderer, &sombra);
    }

    cBorde = ventana->cFondo;
    cBorde.r /= 2;
    cBorde.g /= 2;
    cBorde.b /= 2;

    SDL_SetRenderDrawColor(ventana->renderer, cBorde.r, cBorde.g, cBorde.b, cBorde.a);
    SDL_RenderFillRect(ventana->renderer, &rect);

    rect.x += 4;
    rect.y += 4;
    rect.w -= 8;
    rect.h -= 8;

    SDL_SetRenderDrawColor(ventana->renderer, ventana->cFondo.r, ventana->cFondo.g, ventana->cFondo.b, ventana->cFondo.a);
    SDL_RenderFillRect(ventana->renderer, &rect);

    return rect;
}

static float _interpolacion_lineal(float valorInicial, float valorFinal, float factor)
{

    factor = fmaxf(0.0f, fminf(1.0f, factor)); // Limita el factor al rango [0.0, 1.0]

    return valorInicial + (factor  * (valorFinal - valorInicial));
}
