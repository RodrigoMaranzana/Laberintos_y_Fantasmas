#include <stdio.h>
#include <SDL2/SDL_image.h>
#include "../include/juego.h"
#include "../include/logica.h"
#include "../include/retorno.h"
#include "../include/graficos.h"
#include "../include/assets.h"

/// MACROS TEMPORALES
#define CANT_COLUMNAS 16
#define CANT_FILAS 16

typedef enum {
    MENU_PRINCIPAL_JUGAR,
    MENU_PRINCIPAL_SALIR,
    MENU_PRINCIPAL_ESTADISTICAS,
    MENU_PRINCIPAL_OPCIONES,
    MENU_PRINCIPAL_CANTIDAD,
}eMenuPrincipalAccion;

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana);
static void _juego_sonidos(tLogica *logica, Mix_Chunk **sonidos);
static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tMenu *menu);
static void _juego_iniciar_partida(void* datos);
static void _juego_salir_del_juego(void* datos);
static eMenuComando _juego_accion_a_menu(eAccion accion);

int juego_inicializar(tJuego *juego, const char *tituloVentana)
{
    int ret = TODO_OK;
    unsigned anchoRes, altoRes;

    SDL_Texture *texturaAux;
    juego->estado = JUEGO_NO_INICIADO;

    printf("Iniciando SDL\n");
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != TODO_OK){

        printf("SDL_Init() ERROR: %s\n", SDL_GetError());
        return ERR_SDL_INI;
    }

    // Inicializa SDL_mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){

        printf("Mix_OpenAudio() ERROR: %s\n", Mix_GetError());
        SDL_Quit();
        return ERR_SDL_INI;
    }

    if(TTF_Init() < 0){

        printf("TTF_Init() ERROR: %s\n", TTF_GetError());
        return ERR_SDL_INI;
    }

    logica_calc_resolucion(CANT_COLUMNAS, CANT_FILAS, &anchoRes, &altoRes);

    juego->anchoRes = anchoRes;
    juego->altoRes = altoRes;

    ret = _juego_crear_ventana(
        &juego->ventana,
        &juego->renderer,
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

    juego->sonidos = malloc(sizeof(Mix_Chunk*) * SONIDO_CANTIDAD);
    if(!juego->sonidos){

        return ERR_SIN_MEMORIA;
    }

    juego->imagenes = malloc(sizeof(SDL_Texture*) * IMAGEN_CANTIDAD);
    if(!juego->imagenes){

        return ERR_SIN_MEMORIA;
    }

    ret = assets_cargar_imagenes(juego->renderer, juego->imagenes);
    if(ret != TODO_OK){

        return ret;
    }

    ret = assets_cargar_sonidos(juego->sonidos);
    if(ret != TODO_OK){

        return ret;
    }

    ret = assets_cargar_fuente(&juego->fuente, 64);
    if(ret != TODO_OK){

        return ret;
    }

    juego->menu = menu_crear(2, (SDL_Point){32,32});
    if(!juego->menu){

        return ERR_MENU;
    }

    texturaAux = texto_crear_textura(juego->renderer, juego->fuente, "NUEVA PARTIDA", SDL_COLOR_BLANCO);
    if(menu_agregar_opcion(juego->menu, texturaAux, 64, (tMenuAccion){_juego_iniciar_partida, juego}) != TODO_OK){

        return ERR_MENU;
    }

    texturaAux = texto_crear_textura(juego->renderer, juego->fuente, "SALIR", SDL_COLOR_BLANCO);
    if(menu_agregar_opcion(juego->menu, texturaAux, 64, (tMenuAccion){_juego_salir_del_juego, juego}) != TODO_OK){

        return ERR_MENU;
    }

    if(menu_agregar_opcion(juego->menu, *(juego->imagenes + IMAGEN_PUERTA_SALIDA), 64, (tMenuAccion){_juego_salir_del_juego, juego}) != TODO_OK){

        return ERR_MENU;
    }

    juego->estado = JUEGO_CORRIENDO;
    printf("Juego iniciado con exito.\n");

    return ret;
}

int juego_ejecutar(tJuego *juego)
{
    eRetorno ret = TODO_OK;
    tMenuAccion accionProcesada;
    eAccion accion;
    SDL_Event evento;

    logica_inicializar(&juego->logica);

    while(juego->estado == JUEGO_CORRIENDO){

        accion = ACCION_NINGUNA;

        if(SDL_PollEvent(&evento)){

            if(evento.type == SDL_KEYDOWN){

                 accion = input_procesar_tecla(evento.key.keysym.sym, juego->logica.mapaTeclas, juego->logica.mapaCant);
            }
        }

        if(accion != ACCION_NINGUNA){

            switch(juego->logica.estado){

                case LOGICA_JUGANDO:
                    logica_actualizar(&juego->logica, accion);
                    _juego_sonidos(&juego->logica, juego->sonidos);
                    break;
                default:
                    accionProcesada = menu_procesar_comando(juego->menu, _juego_accion_a_menu(accion));
                    if(accionProcesada.funcion){
                        accionProcesada.funcion(accionProcesada.datos);
                    }
                    break;
            }
        }

        _juego_renderizar(juego->renderer, juego->imagenes, &juego->logica, juego->menu);

        SDL_RenderPresent(juego->renderer);
        SDL_Delay(16);
    }



    return ret;
}

void juego_destruir(tJuego *juego)
{
    SDL_DestroyRenderer(juego->renderer);
    SDL_DestroyWindow(juego->ventana);

    assets_destuir_imagenes(juego->imagenes);
    assets_destuir_sonidos(juego->sonidos);
    assets_destruir_fuente(juego->fuente);

    logica_destruir(&juego->logica);

    Mix_CloseAudio();
    SDL_Quit();

    juego->estado = JUEGO_CERRANDO;
}


/*************************
    FUNCIONES ESTATICAS
*************************/

static void _juego_sonidos(tLogica *logica, Mix_Chunk **sonidos)
{
    /// Test
    if(Mix_Playing(0) == 0) {

        //Mix_PlayChannel(0, *(sonidos + SONIDO_FANTASMA_01), 0);
    }
    ///
}

static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tMenu *menu)
{
    /// Test
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if(logica->estado == LOGICA_JUGANDO){

        escenario_dibujar(renderer, &logica->escenario, imagenes);
    }else{

        menu_dibujar(renderer, menu);
    }
    ///
}

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana)
{
    *ventana = SDL_CreateWindow(tituloVentana, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, anchoRes, altoRes, SDL_WINDOW_SHOWN);
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

    SDL_RenderSetLogicalSize(*renderer, anchoRes, altoRes);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    return TODO_OK;
}

static eMenuComando _juego_accion_a_menu(eAccion accion)
{
    switch(accion){

        case ACCION_ARRIBA:
            return MENU_ANTERIOR;
        case ACCION_ABAJO:
            return MENU_SIGUIENTE;
        case ACCION_CONFIRMAR:
            return MENU_CONFIRMAR;
        default:
            break;
    }

    return MENU_NINGUNO;
}

static void _juego_iniciar_partida(void* datos)
{
    tJuego *juego = (tJuego*)datos;
    puts("Iniciando partida\n");

    juego->logica.estado = LOGICA_JUGANDO;
}

static void _juego_salir_del_juego(void* datos)
{
    tJuego *juego = (tJuego*)datos;
    puts("Saliendo\n");

    juego->estado = JUEGO_CERRANDO;
}
