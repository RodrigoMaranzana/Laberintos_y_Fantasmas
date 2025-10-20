#ifndef MOTOR_H_INCLUDED
#define MOTOR_H_INCLUDED

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include "../../include/cliente/logica.h"
#include "../../include/cliente/texto.h"
#include "../../include/cliente/menu.h"
#include "../../include/comun/cliente.h"
#include "../../include/cliente/widget.h"
#include "../../include/cliente/ventana.h"

#define TECLA_VALIDA(k)( k == SDLK_ESCAPE || k == SDLK_RETURN || k == SDLK_BACKSPACE || k == SDLK_UP || k == SDLK_DOWN || k == SDLK_LEFT || k == SDLK_RIGHT ? 1 : 0)

#define SDL_COLOR_BLANCO ((SDL_Color){255, 255, 255, 255})
#define TOP_CANT 6

typedef enum {
    JUEGO_NO_INICIADO,
    JUEGO_CORRIENDO,
    JUEGO_CERRANDO
} eJuegoEstado;

typedef enum {
    CONTEXTO_IRRELEVANTE,
    CONTEXTO_CREAR_BASE,
    CONTEXTO_INSERTAR_JUGADOR,
    CONTEXTO_DATOS_JUGADOR,
    CONTEXTO_RANKING,
    CONTEXTO_DATOS_RANKING,
}eColaContexto;

typedef enum {
    FUENTE_TAM_32,
    FUENTE_TAM_48,
    FUENTE_TAM_64,
    FUENTE_CANTIDAD,
}eFuenteTam;

typedef enum {
    HUD_WIDGET_VIDAS,
    HUD_WIDGETS_PREMIOS,
    HUD_WIDGETS_RONDA,
    HUD_WIDGETS_USERNAME,
    HUD_WIDGETS_TEXTO,
    HUD_WIDGETS_PERDISTE,
    HUD_WIDGETS_CAMPO_USERNAME,
    HUD_WIDGET_LINEA,
    HUD_WIDGETS_CANTIDAD
} eWidgetsHud;

typedef struct {
    tWidget *widgets[HUD_WIDGETS_CANTIDAD];
    tMenu *menu;
}tHud;

typedef struct {
    char username[TAM_USUARIO];
    int record;
    int cantPartidas;
} tJugador;

typedef struct {
    int idPartida;
    char username[TAM_USUARIO];
    int puntaje;
    int cantMovs;
    int semilla;
} tPartida;

typedef struct {
    SDL_Window *ventana;
    SDL_Renderer *renderer;
    unsigned anchoRes;
    unsigned altoRes;

    SDL_Texture **imagenes;
    Mix_Chunk **sonidos;
    TTF_Font *fuentes[FUENTE_CANTIDAD];

    tHud hud;
    tVentana *ventanaMenuPausa;
    tVentana *ventanaUsername;
    tVentana *ventanaRanking;

    tJugador jugador;
    tLogica logica;
    eJuegoEstado estado;

    SOCKET sock;
    eEstadoSesion sesion;
    tCola colaRespuestas;
    tCola colaContextos;
    tCola colaSolicitudes;
    FILE *archContingencia;

    tJugador ranking[TOP_CANT];
    int rankingCant;
} tJuego;

int juego_inicializar(tJuego *juego, const char *tituloVentana, SOCKET sock, eEstadoSesion sesion);
int juego_ejecutar(tJuego *juego);
void juego_destruir(tJuego *juego);

#endif // MOTOR_H_INCLUDED
