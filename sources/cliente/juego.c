#include <stdio.h>
#include <SDL_image.h>
#include "../../include/cliente/juego.h"
#include "../../include/cliente/logica.h"
#include "../../include/comun/retorno.h"
#include "../../include/cliente/graficos.h"
#include "../../include/cliente/assets.h"
#include "../../include/comun/comun.h"
#include "../../include/cliente/archivo.h"

#define PADDING_MARGEN 64

typedef enum {
    M_PRI_NUEVA_PARTIDA,
    M_PRI_SALIR,
    M_PRI_ESTADISTICAS,
    M_PRI_TEST,
    M_PRI_CANTIDAD,
} eMenuOpcionID;

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana);
static void _juego_sonidos(tLogica *logica, Mix_Chunk **sonidos);
static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tMenu *menu);
static void _juego_iniciar_partida(void* datos);
static void _juego_salir_del_juego(void* datos);

int juego_inicializar(tJuego *juego, const char *tituloVentana)
{
    int ret = TODO_OK;
    unsigned anchoRes, altoRes;
    FILE *archConf;
    tConf conf;

    SDL_Texture *texturaAux;
    juego->estado = JUEGO_NO_INICIADO;

    printf("Iniciando SDL\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != TODO_OK) {

        printf("SDL_Init() ERROR: %s\n", SDL_GetError());
        return ERR_SDL_INI;
    }

    // Inicializa SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {

        printf("Mix_OpenAudio() ERROR: %s\n", Mix_GetError());
        SDL_Quit();
        return ERR_SDL_INI;
    }

    if (TTF_Init() < 0) {

        printf("TTF_Init() ERROR: %s\n", TTF_GetError());
        return ERR_SDL_INI;
    }

    archConf = fopen("config.txt", "r+");
    if (!archConf) {

        archConf = fopen("config.txt", "w+");
        if (!archConf) {

            return ERR_ARCHIVO;
        }
    }

    if (archivo_leer_conf(archConf, &conf) == ERR_CONF) {

        puts("Creando archivo de configuracion por defecto...");

        conf.filas = 17;
        conf.columnas = 17;
        conf.vidas_inicio = 3;
        conf.max_num_fantasmas = 4;
        conf.max_num_premios = 2;
        conf.max_vidas_extra = 1;

        archivo_escribir_conf(archConf, &conf);
    }

    fclose(archConf);

    juego->logica.escenario.confRonda.filas = conf.filas;
    juego->logica.escenario.confRonda.columnas = conf.columnas;

    logica_calc_resolucion(conf.columnas, conf.filas, &anchoRes, &altoRes);
    juego->anchoRes = anchoRes + PADDING_MARGEN;
    juego->altoRes = altoRes + PADDING_MARGEN;

    ret = _juego_crear_ventana(
              &juego->ventana,
              &juego->renderer,
              juego->anchoRes,
              juego->altoRes,
              tituloVentana
          );

    if (ret != TODO_OK) {

        printf("Error: No se pudo crear la ventana.\n");
        Mix_CloseAudio();
        SDL_Quit();
        return ERR_VENTANA;
    }

    juego->sonidos = malloc(sizeof(Mix_Chunk*) * SONIDO_CANTIDAD);
    if (!juego->sonidos) {

        return ERR_SIN_MEMORIA;
    }

    juego->imagenes = malloc(sizeof(SDL_Texture*) * IMAGEN_CANTIDAD);
    if (!juego->imagenes) {

        return ERR_SIN_MEMORIA;
    }

    ret = assets_cargar_imagenes(juego->renderer, juego->imagenes);
    if (ret != TODO_OK) {

        return ret;
    }

    ret = assets_cargar_sonidos(juego->sonidos);
    if (ret != TODO_OK) {

        return ret;
    }

    ret = assets_cargar_fuente(&juego->fuente, 64);
    if (ret != TODO_OK) {

        return ret;
    }

    juego->menu = menu_crear(2, (SDL_Point){32, 32}, MENU_VERTICAL);
    if (!juego->menu) {

        return ERR_MENU;
    }

    texturaAux = texto_crear_textura(juego->renderer, juego->fuente, "NUEVA PARTIDA", SDL_COLOR_BLANCO);
    if (menu_agregar_opcion(juego->menu, M_PRI_NUEVA_PARTIDA, texturaAux, 64, (tMenuAccion) {_juego_iniciar_partida, juego}, OPCION_HABILITADA) != TODO_OK) {

        return ERR_MENU;
    }

    texturaAux = texto_crear_textura(juego->renderer, juego->fuente, "ESTADISTICAS", SDL_COLOR_BLANCO);
    if (menu_agregar_opcion(juego->menu, M_PRI_ESTADISTICAS, texturaAux, 64, (tMenuAccion) {_juego_iniciar_partida, juego}, OPCION_DESHABILITADA) != TODO_OK) {

        return ERR_MENU;
    }

    texturaAux = texto_crear_textura(juego->renderer, juego->fuente, "SALIR", SDL_COLOR_BLANCO);
    if (menu_agregar_opcion(juego->menu, M_PRI_SALIR, texturaAux, 64, (tMenuAccion) {_juego_salir_del_juego, juego}, OPCION_HABILITADA) != TODO_OK) {

        return ERR_MENU;
    }

    juego->estado = JUEGO_CORRIENDO;
    printf("Juego iniciado con exito.\n");

    return ret;
}

int juego_ejecutar(tJuego *juego)
{
    eRetorno ret = TODO_OK;
    tMenuAccion accionProcesada = {NULL, NULL};
    SDL_Keycode tecla;
    SDL_Event evento;

    logica_inicializar(&juego->logica);

    while (juego->estado == JUEGO_CORRIENDO) {

        if (SDL_PollEvent(&evento)) {

            if (evento.type == SDL_QUIT) {

                juego->estado = JUEGO_CERRANDO;
            }

            if (evento.type == SDL_KEYDOWN && input_tecla_valida(evento.key.keysym.sym)) {

                tecla = evento.key.keysym.sym;

                switch (juego->logica.estado) {

                    case LOGICA_JUGANDO:

                        if(juego->logica.fantasmaEnMov == NULL){

                            logica_procesar_turno(&juego->logica, tecla);
                        }

                        _juego_sonidos(&juego->logica, juego->sonidos);
                        break;
                    case LOGICA_EN_ESPERA: {

                        if (tecla == SDLK_UP) {

                            menu_anterior_opcion(juego->menu);
                            Mix_PlayChannel(0, * (juego->sonidos + SONIDO_MENU_MOVIMIENTO), 0);

                        } else if (tecla == SDLK_DOWN) {

                            menu_siguiente_opcion(juego->menu);
                            Mix_PlayChannel(0, * (juego->sonidos + SONIDO_MENU_MOVIMIENTO), 0);

                        } else if (tecla == SDLK_RETURN) {

                            accionProcesada = menu_confirmar_opcion(juego->menu);
                            Mix_PlayChannel(0, * (juego->sonidos + SONIDO_MENU_CONFIRMAR), 0);
                        }

                        if (accionProcesada.funcion) {

                            accionProcesada.funcion(accionProcesada.datos);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        temporizador_actualizar(&juego->logica.fantasmaMovTempor);
        if (juego->logica.estado == LOGICA_JUGANDO && temporizador_estado(&juego->logica.fantasmaMovTempor) == TEMPOR_FINALIZADO && juego->logica.fantasmaEnMov != NULL) {

            logica_actualizar(&juego->logica);
            temporizador_iniciar(&juego->logica.fantasmaMovTempor);

            if (!Mix_Playing(0)) {

                Mix_FadeInChannel(0, *(juego->sonidos + SONIDO_JUGADOR_MOV), 0, 300);

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
    if (Mix_Playing(0) == 0) {

        //Mix_PlayChannel(0, *(sonidos + SONIDO_FANTASMA_01), 0);
    }
    ///
}

static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tMenu *menu)
{
    int i;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (logica->estado == LOGICA_JUGANDO) {

        for (i = 0; i < logica->escenario.confRonda.cantFantasmas; i++) {

            temporizador_actualizar(&logica->escenario.fantasmas[i].temporFrame);
            if (temporizador_estado(&logica->escenario.fantasmas[i].temporFrame) == TEMPOR_FINALIZADO) {

                logica->escenario.fantasmas[i].frame = (logica->escenario.fantasmas[i].frame + 1) % 4;
                temporizador_iniciar(&logica->escenario.fantasmas[i].temporFrame);
            }
        }

        temporizador_actualizar(&logica->escenario.jugador.temporFrame);
        if (temporizador_estado(&logica->escenario.jugador.temporFrame) == TEMPOR_FINALIZADO) {

            logica->escenario.jugador.frame = (logica->escenario.jugador.frame + 1) % 4;
            temporizador_iniciar(&logica->escenario.jugador.temporFrame);
        }

        temporizador_actualizar(&logica->escenario.temporFrame);
        if (temporizador_estado(&logica->escenario.temporFrame) == TEMPOR_FINALIZADO) {

            logica->escenario.frame = (logica->escenario.frame + 1) % 4;
            temporizador_iniciar(&logica->escenario.temporFrame);
        }

        escenario_dibujar(renderer, &logica->escenario, imagenes);
    } else {

        menu_dibujar(renderer, menu);
    }
}

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana)
{
    *ventana = SDL_CreateWindow(tituloVentana, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, anchoRes, altoRes, SDL_WINDOW_SHOWN);
    if (!*ventana) {

        printf("Error: %s\n", SDL_GetError());
        return ERR_VENTANA;
    }

    *renderer = SDL_CreateRenderer(*ventana, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {

        SDL_DestroyWindow(*ventana);
        printf("Error: %s\n", SDL_GetError());
        return ERR_RENDERER;
    }

    SDL_RenderSetLogicalSize(*renderer, anchoRes, altoRes);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    return TODO_OK;
}

static void _juego_iniciar_partida(void* datos)
{
    tJuego *juego = (tJuego*) datos;
    puts("Iniciando partida\n");

    juego->logica.estado = LOGICA_JUGANDO;
}

static void _juego_salir_del_juego(void* datos)
{
    tJuego *juego = (tJuego*) datos;
    puts("Saliendo\n");

    juego->estado = JUEGO_CERRANDO;
}
