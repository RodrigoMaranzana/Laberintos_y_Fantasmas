#include "../../include/juego/widget.h"
#include "../../include/juego/graficos.h"
#include "../../include/juego/texto.h"
#include <string.h>
#include <stdio.h>

#define TAM_TEXTO 33
#define TAM_BUFFER_TEXTO 17

typedef int (*_widget_actualizar)(tWidget *widget);
typedef void (*_widget_dibujar)(tWidget *widget);
typedef void (*_widget_destruir)(tWidget *widget);

typedef enum {
    WIDGET_CAMPO_TEXTO,
    WIDGET_BOTON_TEXTO,
    WIDGET_CONTADOR,
    WIDGET_GRAFICO_BARRAS,
    WIDGET_TEXTO,
}eWidgetTipo;

struct sWidget {
    eWidgetTipo tipo;
    SDL_Point coords;

    void *datos;

    SDL_Rect rectTexto;
    TTF_Font *fuente;
    SDL_Texture *texTexto;
    SDL_Color cTexto;

    SDL_Renderer *renderer;

    _widget_actualizar actualizar;
    _widget_dibujar dibujar;
    _widget_destruir destruir;
    char visible;
};

typedef struct {
    char buffer[TAM_BUFFER_TEXTO];
    int cursorPos;
    SDL_Color cFondo;
    char activo;
    char modificado;
} tCampoTexto;

typedef struct {
    char texto[TAM_TEXTO];
    char posado;
    SDL_Rect rect;
    SDL_Color cFondo;
    char habilitado;
} tBotonTexto;

typedef struct {
    int valor;
    SDL_Texture *icono;
    char modificado;
} tContador;

typedef struct {
    char texto[TAM_TEXTO];
    float valor;
    int ancho;
    int alto;
    SDL_Color cFondo;
    SDL_Color cRelleno;
} tGraficoBarras;

typedef struct {
    char buffer[TAM_TEXTO];
    SDL_Color cFondo;
    char modificado;
} tTexto;

static tWidget* _widget_crear_base(SDL_Renderer *renderer, SDL_Point coords, TTF_Font *fuente, const char *texto, SDL_Color cTexto, eWidgetTipo tipo, char visible);

static int _widget_campo_texto_actualizar(tWidget *widget);
static void _widget_campo_texto_dibujar(tWidget *widget);

static int _widget_boton_actualizar(tWidget *widget);
static void _widget_boton_dibujar(tWidget *widget);
static void _widget_boton_destruir(tWidget *widget);

static int _widget_contador_actualizar(tWidget *widget);
static void _widget_contador_dibujar(tWidget *widget);

static int _widget_texto_actualizar(tWidget *widget);
static void _widget_texto_dibujar(tWidget *widget);


tWidget* widget_crear_campo_texto(SDL_Renderer *renderer, SDL_Point coords, TTF_Font *fuente, SDL_Color cTexto, SDL_Color cFondo, char visible)
{
    tCampoTexto *campoTexto;

    tWidget *widget = _widget_crear_base(renderer, coords, fuente, NULL, cTexto, WIDGET_CAMPO_TEXTO, visible);
    if (!widget) return NULL;

    widget->actualizar = _widget_campo_texto_actualizar;
    widget->dibujar = _widget_campo_texto_dibujar;
    widget->destruir = NULL;

    campoTexto = (tCampoTexto*)widget->datos;
    *campoTexto->buffer = '\0';
    campoTexto->cFondo = cFondo;
    campoTexto->cursorPos = 0;
    campoTexto->activo = 0;
    campoTexto->modificado = 0;

    return widget;
}

tWidget* widget_crear_boton_texto(SDL_Renderer *renderer, SDL_Point coords, const char* texto, TTF_Font *fuente, SDL_Rect rect, SDL_Color cTexto, SDL_Color cFondo, char visible)
{
    tBotonTexto *boton;

    tWidget *widget = _widget_crear_base(renderer, coords, fuente, texto, cTexto, WIDGET_BOTON_TEXTO, visible);
    if (!widget) {
        return NULL;
    }

    widget->actualizar = _widget_boton_actualizar;
    widget->dibujar = _widget_boton_dibujar;
    widget->destruir = _widget_boton_destruir;

    boton = (tBotonTexto*)widget->datos;
    strncpy(boton->texto, texto, TAM_TEXTO);
    *(boton->texto + TAM_TEXTO - 1) = '\0';
    boton->rect = rect;
    boton->cFondo = cFondo;
    boton->posado = 0;
    boton->habilitado = 0;

    return widget;
}

tWidget* widget_crear_contador(SDL_Renderer *renderer, SDL_Point coords, SDL_Texture *icono, TTF_Font *fuente, SDL_Color cTexto, int valor, char visible)
{
    tContador *contador;
    char buffer[12];
    snprintf(buffer, 12, "%d", valor);

    tWidget *widget = _widget_crear_base(renderer, coords, fuente, buffer, cTexto, WIDGET_CONTADOR, visible);
    if (!widget) {
        return NULL;
    }

    widget->actualizar = _widget_contador_actualizar;
    widget->dibujar = _widget_contador_dibujar;
    widget->destruir = NULL;

    contador = (tContador*)widget->datos;
    contador->valor = valor;
    contador->icono = icono;
    contador->modificado = 0;

    return widget;
}

tWidget* widget_crear_texto(SDL_Renderer *renderer, const char *texto, SDL_Point coords, TTF_Font *fuente, SDL_Color cTexto, SDL_Color cFondo, char visible)
{
    tTexto *textoW;

    tWidget *widget = _widget_crear_base(renderer, coords, fuente, texto, cTexto, WIDGET_TEXTO, visible);
    if (!widget) return NULL;

    widget->actualizar = _widget_texto_actualizar;
    widget->dibujar = _widget_texto_dibujar;
    widget->destruir = NULL;

    textoW = (tTexto*)widget->datos;
    *textoW->buffer = '\0';
    textoW->cFondo = cFondo;
    textoW->modificado = 0;

    return widget;
}

void widget_modificar_visibilidad(tWidget *widget, char visible)
{
    widget->visible = visible;
}

void widget_alternar_visibilidad(tWidget *widget)
{
    widget->visible = !widget->visible;
}

int widget_modificar_valor(tWidget *widget, void *valor)
{
    char modificado = 0;

    switch (widget->tipo) {

        case WIDGET_CAMPO_TEXTO: {
            tCampoTexto *campoTexto = (tCampoTexto*)widget->datos;
            const char *nuevoTexto = (const char*)valor;

            if (strcmp(campoTexto->buffer, nuevoTexto) != 0) {
                strncpy(campoTexto->buffer, nuevoTexto, TAM_BUFFER_TEXTO - 1);
                campoTexto->buffer[TAM_BUFFER_TEXTO - 1] = '\0';
                modificado = 1;
            }
            break;
        }
        case WIDGET_CONTADOR: {
            tContador *contador = (tContador*)widget->datos;
            int nuevoValor = *(int*)valor;

            if (contador->valor != nuevoValor) {
                contador->valor = nuevoValor;
                modificado = 1;
            }
            break;
        }
        case WIDGET_TEXTO: {
            tTexto *texto = (tTexto*)widget->datos;
            const char *nuevoTexto = (const char*)valor;

            if (strcmp(texto->buffer, nuevoTexto) != 0) {
                strncpy(texto->buffer, nuevoTexto, TAM_TEXTO - 1);
                texto->buffer[TAM_TEXTO - 1] = '\0';
                modificado = 1;
            }
            break;
        }
        default:
            return WIDGET_INCORRECTO;
    }

    if (modificado) {
        return widget->actualizar(widget);
    }

    return WIDGET_ERR_TODO_OK;
}

void widget_destruir(tWidget *widget)
{
    if (widget->destruir) {
        widget->destruir(widget);
    }

    if (widget->texTexto) {
        SDL_DestroyTexture(widget->texTexto);
    }

    free(widget->datos);
    free(widget);
}

void widget_dibujar(tWidget *widget)
{
    if (!widget->visible) return;

    widget->dibujar(widget);
}

void widget_modificar_posicion(tWidget *widget, SDL_Point nuevaPos) {

    if (widget) {
        widget->coords = nuevaPos;
    }
}

/*************************
    FUNCIONES ESTATICAS
*************************/

static tWidget* _widget_crear_base(SDL_Renderer *renderer, SDL_Point coords, TTF_Font *fuente, const char *texto, SDL_Color cTexto, eWidgetTipo tipo, char visible)
{
    SDL_Point tamTexto;

    tWidget *widget = (tWidget*)malloc(sizeof(tWidget));
    if (!widget) {
        return NULL;
    }

    switch (tipo) {
        case WIDGET_CAMPO_TEXTO:
            widget->datos = malloc(sizeof(tCampoTexto));
            break;
        case WIDGET_BOTON_TEXTO:
            widget->datos = malloc(sizeof(tBotonTexto));
            break;
        case WIDGET_CONTADOR:
            widget->datos = malloc(sizeof(tContador));
            break;
        case WIDGET_GRAFICO_BARRAS:
            widget->datos = malloc(sizeof(tGraficoBarras));
            break;
        case WIDGET_TEXTO:
            widget->datos = malloc(sizeof(tTexto));
            break;
        default:
            widget->datos = NULL;
    }

    if (!widget->datos) {
        free(widget);
        widget = NULL;
        return NULL;
    }

    widget->coords.x = coords.x;
    widget->coords.y = coords.y;
    widget->rectTexto.x = coords.x;
    widget->rectTexto.y = coords.y;

    if (texto) {
        widget->texTexto = texto_crear_textura(renderer, fuente, texto, cTexto);
        texto_obtener_tam(fuente, texto, &tamTexto);
        widget->rectTexto.w = tamTexto.x;
        widget->rectTexto.h = tamTexto.y;
    } else {
        widget->texTexto = NULL;
        widget->rectTexto.w = 0;
        widget->rectTexto.h = 0;
    }

    widget->cTexto = cTexto;
    widget->fuente = fuente;
    widget->renderer = renderer;
    widget->visible = visible;
    widget->tipo = tipo;

    return widget;
}

static int _widget_campo_texto_actualizar(tWidget *widget)
{
    SDL_Point tamTexto;
    tCampoTexto *campoTexto = (tCampoTexto*)widget->datos;

    if (widget->texTexto != NULL) {

        SDL_DestroyTexture(widget->texTexto);
        widget->texTexto = NULL;
    }

    if (strlen(campoTexto->buffer)) {

        widget->texTexto = texto_crear_textura(widget->renderer, widget->fuente, campoTexto->buffer, widget->cTexto);
    }

    texto_obtener_tam(widget->fuente, campoTexto->buffer, &tamTexto);
    widget->rectTexto.w = tamTexto.x;
    widget->rectTexto.h = tamTexto.y;

    return 0;
}

static void _widget_campo_texto_dibujar(tWidget *widget)
{
    int anchoTexTexto, altoTexTexto;
    SDL_Color cBorde;
    SDL_Rect rectDestinoTexto, rectDestinoFondo;
    tCampoTexto *campoTexto = (tCampoTexto*)widget->datos;

    cBorde = campoTexto->cFondo;
    cBorde.r /= 2;
    cBorde.g /= 2;
    cBorde.b /= 2;

    if (widget->texTexto) {
        SDL_QueryTexture(widget->texTexto, NULL, NULL, &anchoTexTexto, &altoTexTexto);
    } else {
        anchoTexTexto = 0;
        altoTexTexto = TTF_FontHeight(widget->fuente);
    }

    rectDestinoFondo.w = anchoTexTexto + (altoTexTexto * 2);
    rectDestinoFondo.h = (int)(altoTexTexto * 1.5);

    rectDestinoFondo.x = widget->coords.x - (rectDestinoFondo.w / 2);
    rectDestinoFondo.y = widget->coords.y - (rectDestinoFondo.h / 2);

    rectDestinoTexto.x = rectDestinoFondo.x + altoTexTexto;
    rectDestinoTexto.y = rectDestinoFondo.y + (rectDestinoFondo.h - altoTexTexto) / 2;
    rectDestinoTexto.w = anchoTexTexto;
    rectDestinoTexto.h = altoTexTexto;

    graficos_dibujar_relleno(widget->renderer, rectDestinoFondo, campoTexto->cFondo);
    graficos_dibujar_borde(widget->renderer, rectDestinoFondo, cBorde);

    if ((SDL_GetTicks() / 500) % 2) {

        SDL_SetRenderDrawColor(widget->renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(widget->renderer,
                           rectDestinoTexto.x + rectDestinoTexto.w + 4,
                           rectDestinoTexto.y,
                           rectDestinoTexto.x + rectDestinoTexto.w + 4,
                           rectDestinoTexto.y + rectDestinoTexto.h
                           );
    }

    if (widget->texTexto) {

        graficos_dibujar_textura(widget->texTexto, widget->renderer, NULL, &rectDestinoTexto, NULL);
    }
}

static int _widget_texto_actualizar(tWidget *widget)
{
    SDL_Point tamTexto;
    tTexto *texto = (tTexto*)widget->datos;

    if (widget->texTexto != NULL) {

        SDL_DestroyTexture(widget->texTexto);
        widget->texTexto = NULL;
    }

    if (strlen(texto->buffer)) {

        widget->texTexto = texto_crear_textura(widget->renderer, widget->fuente, texto->buffer, widget->cTexto);
    }

    texto_obtener_tam(widget->fuente, texto->buffer, &tamTexto);
    widget->rectTexto.w = tamTexto.x;
    widget->rectTexto.h = tamTexto.y;

    return 0;
}

static void _widget_texto_dibujar(tWidget *widget)
{
    int anchoTexTexto, altoTexTexto;
    SDL_Rect rectDestinoTexto, rectDestinoFondo;
    tTexto *texto = (tTexto*)widget->datos;

    if (widget->texTexto) {
        SDL_QueryTexture(widget->texTexto, NULL, NULL, &anchoTexTexto, &altoTexTexto);
    } else {
        anchoTexTexto = 0;
        altoTexTexto = TTF_FontHeight(widget->fuente);
    }

    rectDestinoFondo.w = anchoTexTexto + (altoTexTexto * 2);
    rectDestinoFondo.h = (int)(altoTexTexto * 1.5);

    rectDestinoFondo.x = widget->coords.x - (rectDestinoFondo.w / 2);
    rectDestinoFondo.y = widget->coords.y - (rectDestinoFondo.h / 2);

    rectDestinoTexto.x = rectDestinoFondo.x + altoTexTexto;
    rectDestinoTexto.y = rectDestinoFondo.y + (rectDestinoFondo.h - altoTexTexto) / 2;
    rectDestinoTexto.w = anchoTexTexto;
    rectDestinoTexto.h = altoTexTexto;

    graficos_dibujar_relleno(widget->renderer, rectDestinoFondo, texto->cFondo);

    if (widget->texTexto) {

        graficos_dibujar_textura(widget->texTexto, widget->renderer, NULL, &rectDestinoTexto, NULL);
    }
}

static int _widget_boton_actualizar(tWidget *widget)
{
    return 0;
}

static void _widget_boton_dibujar(tWidget *widget)
{

}

static void _widget_boton_destruir(tWidget *widget)
{

}


static int _widget_contador_actualizar(tWidget *widget)
{
    SDL_Point tamTexto;
    char buffer[12];
    tContador *contador = (tContador*)widget->datos;

    SDL_DestroyTexture(widget->texTexto);
    snprintf(buffer, 12, "%d", contador->valor);
    widget->texTexto = texto_crear_textura(widget->renderer, widget->fuente, buffer, widget->cTexto);
    texto_obtener_tam(widget->fuente, buffer, &tamTexto);
    widget->rectTexto.w = tamTexto.x;
    widget->rectTexto.h = tamTexto.y;

    return WIDGET_ERR_TODO_OK;
}

static void _widget_contador_dibujar(tWidget *widget)
{
    int anchoTextura, altoTextura, paddingX = 0;
    SDL_Rect rectDestino;
    tContador *contador = (tContador*)widget->datos;

    if (contador->icono) {
        SDL_QueryTexture(contador->icono, NULL, NULL, &anchoTextura, &altoTextura);
        rectDestino.x = widget->coords.x;
        rectDestino.y = widget->coords.y;
        rectDestino.w = anchoTextura;
        rectDestino.h = altoTextura;
        graficos_dibujar_textura(contador->icono, widget->renderer, NULL, &rectDestino, NULL);
        paddingX = anchoTextura * 1.1;
    }

    rectDestino = widget->rectTexto;
    rectDestino.x += paddingX;
    graficos_dibujar_textura(widget->texTexto, widget->renderer, NULL, &rectDestino, NULL);
}



