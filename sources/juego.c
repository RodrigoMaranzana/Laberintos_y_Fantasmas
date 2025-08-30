#include <stdio.h>
#include <SDL2/SDL_image.h>
#include "../include/juego.h"
#include "../include/logica.h"
#include "../include/retornos.h"
#include "../include/graficos.h"
#include "../include/assets.h"

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, SDL_Texture **framebuffer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana);
static void _juego_sonidos(tLogica *logica, Mix_Chunk **sndAssets);
static void _juego_renderizar(SDL_Renderer* renderer, SDL_Texture* framebuffer, SDL_Texture **imgAssets,  tLogica *logica);

int juego_inicializar(tJuego* juego, unsigned anchoRes, unsigned altoRes, const char *tituloVentana)
{
    int ret = TODO_OK;
    juego->estado = JUEGO_NO_INICIADO;

    printf("Iniciando SDL\n");
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != TODO_OK){

        printf("SDL_Init() Error: %s\n", SDL_GetError());
        return ERR_SDL_INI;
    }

    // Inicializa SDL_mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){

        printf("Mix_OpenAudio() Error: %s\n", Mix_GetError());
        SDL_Quit();
        return ERR_SDL_INI;
    }

    juego->anchoRes = anchoRes;
    juego->altoRes = altoRes;

    ret = _juego_crear_ventana(
        &juego->ventana,
        &juego->renderer,
        &juego->framebuffer,
        juego->anchoRes,
        juego->altoRes,
        tituloVentana
    );

    if(ret != TODO_OK){

        printf("Error: No se pudo crear la ventana.\n");
        Mix_CloseAudio();
        SDL_Quit();
        return ERR_VENTANA;
    }

    juego->sndAssets = malloc(sizeof(Mix_Chunk*) * SONIDO_CANTIDAD);
    if(!juego->sndAssets){

        return ERR_SIN_MEMORIA;
    }

    juego->imgAssets = malloc(sizeof(SDL_Texture*) * IMAGEN_CANTIDAD);
    if(!juego->imgAssets){

        return ERR_SIN_MEMORIA;
    }

    assets_cargar_imagenes(juego->renderer, juego->imgAssets);
    assets_cargar_sonidos(juego->sndAssets);

    juego->estado = JUEGO_CORRIENDO;
    printf("Motor iniciado con exito.\n");

    return ret;
}

int juego_ejecutar(tJuego* juego)
{
    eRetornos ret = TODO_OK;
    tLogica logica;
    eAccion accion;
    SDL_Event evento;

    logica_inicializar(&logica);

    while(juego->estado == JUEGO_CORRIENDO){

        while(SDL_PollEvent(&evento)){

            if(Mix_Playing(0) == 0 && evento.type == SDL_KEYDOWN && (accion = input_procesar_tecla(evento.key.keysym.sym, logica.mapaTeclas, logica.mapaCant)) != -1){

                logica_actualizar(&logica, accion);
                _juego_sonidos(&logica, juego->sndAssets);
            }
        }

        _juego_renderizar(juego->renderer, juego->framebuffer, juego->imgAssets, &logica);
    }

    logica_destruir(&logica);

    return ret;
}

void juego_destruir(tJuego* juego)
{
    SDL_DestroyRenderer(juego->renderer);
    SDL_DestroyWindow(juego->ventana);
    SDL_DestroyTexture(juego->framebuffer);
    Mix_CloseAudio();

    /// FREE TEXTURAS

    SDL_Quit();

    juego->estado = JUEGO_CERRANDO;
}


/*************************
    FUNCIONES ESTATICAS
*************************/

static void _juego_sonidos(tLogica *logica, Mix_Chunk **sndAssets)
{
    if(Mix_Playing(0) == 0) {

        Mix_PlayChannel(0, *(sndAssets + SONIDO_FANTASMA_01), 0);
    }
}

static void _juego_renderizar(SDL_Renderer* renderer, SDL_Texture* framebuffer, SDL_Texture **imgAssets, tLogica *logica)
{
    // Borra el renderer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    graficos_dibujar_textura(*imgAssets, renderer, &(SDL_Rect){.x = 0, .y = 0, .w = 16, .h = 16}, &(SDL_Rect){.x = logica->jugador.ubic.columna * 16, .y = logica->jugador.ubic.fila * 16, .w = 16, .h = 16});

    SDL_RenderPresent(renderer);
}

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, SDL_Texture **framebuffer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana)
{
    *ventana = SDL_CreateWindow(tituloVentana, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, anchoRes * 2, altoRes * 2, SDL_WINDOW_SHOWN);
    if(!*ventana){

        printf("Error: %s\n", SDL_GetError());
        return ERR_VENTANA;
    }

    *renderer = SDL_CreateRenderer(*ventana, -1, SDL_RENDERER_ACCELERATED);
    if(!*renderer){

        SDL_DestroyWindow(*ventana);
        printf("Error: %s\n", SDL_GetError());
        return ERR_RENDERER;
    }

//    *framebuffer = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, anchoRes, altoRes);
//    if(!*framebuffer){
//
//        SDL_DestroyRenderer(*renderer);
//        SDL_DestroyWindow(*ventana);
//        printf("Error: %s", SDL_GetError());
//        return ERR_TEXTURA;
//    }
//    SDL_SetTextureBlendMode(*framebuffer, SDL_BLENDMODE_BLEND);

    SDL_RenderSetLogicalSize(*renderer, anchoRes, altoRes);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    return TODO_OK;
}
