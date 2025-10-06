#include "../../include/cliente/juego.h"
#include "../../include/cliente/assets.h"
#include "../../include/comun/comun.h"
#include "../../include/cliente/dibujado.h"

#include <stdio.h>

#define FUENTE_TAM_MIN 32
#define FUENTE_INCREMENTO 16
#define HUD_X 16
#define HUD_VIDAS_Y 16
#define HUD_PREMIOS_Y 96
#define HUD_RONDA_Y 160
#define VENTANA_MENU_PAUSA_ANCHO 384
#define VENTANA_MENU_PAUSA_ALTO 512
#define VENTANA_USERNAME_ANCHO 340
#define VENTANA_USERNAME_ALTO 128

typedef enum {
    M_PRI_NUEVA_PARTIDA,
    M_PRI_CONTINUAR,
    M_PRI_CAMBIAR_USUARIO,
    M_PRI_ESTADISTICAS,
    M_PRI_SALIR,
    M_PRI_CANTIDAD
} eMenuOpcionID;

typedef struct {
    tLogica *logica;
    eJuegoEstado *estadoJuego;
    SDL_Renderer *renderer;
    Mix_Chunk **sonidos;
    TTF_Font **fuentes;
    tMenu* menu;
} tDatosMenuPausa;

typedef struct {
    SDL_Renderer *renderer;
    TTF_Font **fuentes;
    tWidget *campoTexto;
    char *usuario;
    eLogicaEstado *estadoLogica;
} tDatosUsuario;

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana);
static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tVentana *ventanaMenu, tVentana *ventanaUsername, tHud *hud);
static void _juego_iniciar_partida(void* datos);
static void _juego_salir_del_juego(void* datos);
static void _juego_continuar_partida(void* datos);
static int _juego_actualizar_hud(tHud *hud, tLogica *logica);
static int _juego_crear_hud(tJuego *juego);
static int _juego_cargar_assets(tJuego *juego);
static void _juego_manejar_input(tJuego *juego, SDL_Keycode tecla);
static void _juego_manejar_eventos(tJuego *juego);

static int _juego_ventana_menu_crear(void *datos);
static void _juego_ventana_menu_actualizar(SDL_Event *evento, void *datos);
static void _juego_ventana_menu_dibujar(void *datos);
static void _juego_ventana_menu_destruir(void *datos);

static int _juego_ventana_usuario_crear(void *datos);
static void _juego_ventana_usuario_actualizar(SDL_Event *evento, void *datos);
static void _juego_ventana_usuario_dibujar(void *datos);
static void _juego_ventana_usuario_destruir(void *datos);

int juego_inicializar(tJuego *juego, const char *tituloVentana)
{
    int ret = ERR_TODO_OK;

    memset(juego, 0, sizeof(tJuego));
    juego->estado = JUEGO_NO_INICIADO;
    printf("Iniciando SDL\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != ERR_TODO_OK) {
        printf("SDL_Init() ERROR: %s\n", SDL_GetError());
        return ERR_SDL_INI;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio() ERROR: %s\n", Mix_GetError());
        SDL_Quit();
        return ERR_SDL_INI;
    }

    if (TTF_Init() < 0) {
        printf("TTF_Init() ERROR: %s\n", TTF_GetError());
        return ERR_SDL_INI;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("IMG_Init() ERROR: %s\n", IMG_GetError());
        ret = ERR_SDL_INI;
    }

    logica_inicializar(&juego->logica);
    logica_calc_min_res(&juego->logica, &juego->anchoRes, &juego->altoRes);
    juego->anchoRes += PADDING_MARGEN * 2;
    juego->altoRes += PADDING_MARGEN * 2;

    if (_juego_crear_ventana(&juego->ventana, &juego->renderer, juego->anchoRes, juego->altoRes, tituloVentana) != ERR_TODO_OK) {
        printf("Error: No se pudo crear la ventana.\n");
        Mix_CloseAudio();
        SDL_Quit();
        return ERR_VENTANA;
    }

    if ((ret = _juego_cargar_assets(juego)) != ERR_TODO_OK) return ret;
    if ((ret = _juego_crear_hud(juego)) != ERR_TODO_OK) return ret;

    juego->estado = JUEGO_CORRIENDO;
    printf("Juego iniciado con exito.\n");

    return ret;
}

int juego_ejecutar(tJuego *juego)
{
    eRetorno ret = ERR_TODO_OK;

    while (juego->estado == JUEGO_CORRIENDO) {

        _juego_manejar_eventos(juego);

        if (juego->logica.estado == LOGICA_JUGANDO) {

            logica_actualizar(&juego->logica);
            _juego_actualizar_hud(&juego->hud, &juego->logica);
        }

        _juego_renderizar(juego->renderer, juego->imagenes, &juego->logica, juego->ventanaMenuPausa, juego->ventanaUsername, &juego->hud);

        SDL_Delay(16);
    }

    return ret;
}

void juego_destruir(tJuego *juego)
{
    int i;

    assets_destuir_imagenes(juego->imagenes);
    assets_destuir_sonidos(juego->sonidos);

    for (i = 0; i < FUENTE_CANTIDAD; i++){
        assets_destruir_fuente(juego->fuentes[i]);
    }

    for (i = 0; i < HUD_WIDGETS_CANTIDAD; i++) {
        widget_destruir(juego->hud.widgets[i]);
    }

    if (juego->ventanaMenuPausa) ventana_destruir(juego->ventanaMenuPausa);

    if (juego->ventanaUsername) ventana_destruir(juego->ventanaUsername);

    logica_destruir(&juego->logica);

    if (juego->renderer) {
        SDL_DestroyRenderer(juego->renderer);
    }
    if (juego->ventana) {
        SDL_DestroyWindow(juego->ventana);
    }

    IMG_Quit();
    TTF_Quit();
    Mix_CloseAudio();
    SDL_Quit();

    juego->estado = JUEGO_CERRANDO;
}


/*************************
    FUNCIONES ESTATICAS
*************************/

static void _juego_manejar_eventos(tJuego *juego)
{
    SDL_Event evento;
    SDL_Keycode tecla;

    while (SDL_PollEvent(&evento)) {

        if (evento.type == SDL_QUIT) {

            juego->estado = JUEGO_CERRANDO;
            return;
        }
        switch (juego->logica.estado) {
            case LOGICA_EN_LOGIN:
                ventana_actualizar(juego->ventanaUsername, &evento);
                if (juego->logica.estado == LOGICA_EN_ESPERA) {

                    ventana_cerrar(juego->ventanaUsername);
                }
                break;

            case LOGICA_JUGANDO:
                if (evento.type == SDL_KEYDOWN && TECLA_VALIDA(evento.key.keysym.sym)) {

                    tecla = evento.key.keysym.sym;
                    _juego_manejar_input(juego, tecla);
                }
                break;

            case LOGICA_FIN_PARTIDA:
                /* CAIDA */
            case LOGICA_EN_ESPERA:
                /* CAIDA */
            case LOGICA_EN_PAUSA:
                ventana_actualizar(juego->ventanaMenuPausa, &evento);
                break;
            default:
                break;
        }
    }
}

static void _juego_manejar_input(tJuego *juego, SDL_Keycode tecla)
{
    if (tecla == SDLK_ESCAPE) {

        ventana_abrir(juego->ventanaMenuPausa);
        juego->logica.estado = LOGICA_EN_PAUSA;
        Mix_PlayChannel(-1, juego->sonidos[SONIDO_MENU_MOVIMIENTO], 0);
        return;
    }

    if (logica_procesar_turno(&juego->logica, tecla)) {

        logica_procesar_movimientos(&juego->logica);
    }

    if (juego->logica.estado == LOGICA_FIN_PARTIDA) {

        juego->logica.estado = LOGICA_EN_ESPERA;
        logica_mostrar_historial_movs(&juego->logica.movimientos);
    }

}

static int _juego_actualizar_hud(tHud *hud, tLogica *logica)
{
    widget_modificar_valor(hud->widgets[HUD_WIDGET_VIDAS], &logica->ronda.cantVidasActual);
    widget_modificar_valor(hud->widgets[HUD_WIDGETS_PREMIOS], &logica->ronda.cantPremios);
    widget_modificar_valor(hud->widgets[HUD_WIDGETS_RONDA], &logica->ronda.numRonda);

    if (logica->mostrarSigRonda) {

        widget_modificar_visibilidad(hud->widgets[HUD_WIDGETS_TEXTO], 1);
        logica->mostrarSigRonda = 0;
    }

    temporizador_actualizar(&logica->temporCambioRonda);
    if (temporizador_estado(&logica->temporCambioRonda) == TEMPOR_FINALIZADO) {

        widget_modificar_visibilidad(hud->widgets[HUD_WIDGETS_TEXTO], 0);
    }

    return ERR_TODO_OK;
}

static void _juego_dibujar_hud(tHud *hud, SDL_Renderer *renderer)
{
    int i;

    for (i = 0; i < HUD_WIDGETS_CANTIDAD; i++) {

        widget_dibujar(hud->widgets[i]);
    }
}

static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tVentana *ventanaMenu, tVentana *ventanaUsername, tHud *hud)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (logica->estado == LOGICA_JUGANDO) {
        dibujado_escenario(renderer, &logica->escenario, imagenes);
        _juego_dibujar_hud(hud, renderer);
    } else {
        ventana_dibujar(ventanaMenu);
        ventana_dibujar(ventanaUsername);
    }

    SDL_RenderPresent(renderer);
}

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana)
{
    float maxAltoVentana, factorEscala;

    SDL_DisplayMode display;
    if (SDL_GetDesktopDisplayMode(0, &display) != 0) {
        SDL_Quit();
        return ERR_VENTANA;
    }

    maxAltoVentana = display.h * 0.90f;
    factorEscala = 1.0f;
    if (altoRes > maxAltoVentana) {

        factorEscala = maxAltoVentana / altoRes;
    }

    *ventana = SDL_CreateWindow(tituloVentana, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, anchoRes * factorEscala, altoRes * factorEscala, SDL_WINDOW_SHOWN);
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

    SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);

    SDL_RenderSetLogicalSize(*renderer, anchoRes, altoRes);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    return ERR_TODO_OK;
}

static void _juego_iniciar_partida(void *datos)
{
    tLogica *logica = (tLogica*)datos;
    puts("\nIniciando partida\n");
    logica_iniciar_juego(logica);
}

static void _juego_salir_del_juego(void *datos)
{
    eJuegoEstado *estado = (eJuegoEstado*)datos;
    puts("Saliendo\n");
    *estado = JUEGO_CERRANDO;
}

static void _juego_continuar_partida(void *datos)
{
    tLogica *logica = (tLogica*)datos;
    puts("Continuar partida\n");
    logica->estado = LOGICA_JUGANDO;
}

//static int _juego_solicitar_apertura_tabla(const char *nombreTabla)
//{
//    char bufferEnvio[TAM_BUFFER] = {0};
//    char bufferRespuesta[TAM_BUFFER] = {0};
//
//    sprintf(bufferEnvio, "%s %s", "ABRIR TABLA", nombreTabla);
//    printf("Solicitud: %s\n", bufferEnvio);
//
//    cliente_enviar_solicitud(juego.sock, bufferEnvio, bufferRespuesta);
//    printf("Respuesta del servidor: %s\n", bufferRespuesta);
//
//
//
//    return ERR_TODO_OK;
//}

///// LONGITUD|RETORNO|MENSAJE|
//
//static int _juego_parsear_respuesta(const char *respuesta)
//{
//
//}


static int _juego_ventana_menu_crear(void *datos)
{
    tDatosMenuPausa *datosMenuPausa = (tDatosMenuPausa*)(datos);
    SDL_Texture *texturaAux = NULL;

    datosMenuPausa->menu = menu_crear(datosMenuPausa->renderer, 2, (SDL_Point) {32, 32}, MENU_VERTICAL);
    if (!datosMenuPausa->menu) {
        return -1;
    }

    // NUEVA PARTIDA
    texturaAux = texto_crear_textura(datosMenuPausa->renderer, datosMenuPausa->fuentes[FUENTE_TAM_64], "NUEVA PARTIDA", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }
    if (menu_agregar_opcion(datosMenuPausa->menu, M_PRI_NUEVA_PARTIDA, texturaAux, 64, (tMenuAccion){_juego_iniciar_partida, datosMenuPausa->logica}, OPCION_HABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }

    // CONTINUAR
    texturaAux = texto_crear_textura(datosMenuPausa->renderer, datosMenuPausa->fuentes[FUENTE_TAM_64], "CONTINUAR", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }
    if (menu_agregar_opcion(datosMenuPausa->menu, M_PRI_CONTINUAR, texturaAux, 64, (tMenuAccion){_juego_continuar_partida, datosMenuPausa->logica}, OPCION_OCULTA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }

    // LOGIN USUARIO
    texturaAux = texto_crear_textura(datosMenuPausa->renderer, datosMenuPausa->fuentes[FUENTE_TAM_64], "LOGIN USUARIO", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }
    if (menu_agregar_opcion(datosMenuPausa->menu, M_PRI_CAMBIAR_USUARIO, texturaAux, 64, (tMenuAccion){NULL, datosMenuPausa->logica}, OPCION_HABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }

    // ESTADISTICAS
    texturaAux = texto_crear_textura(datosMenuPausa->renderer, datosMenuPausa->fuentes[FUENTE_TAM_64], "ESTADISTICAS", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }
    if (menu_agregar_opcion(datosMenuPausa->menu, M_PRI_ESTADISTICAS, texturaAux, 64, (tMenuAccion){NULL, NULL}, OPCION_DESHABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }

    // SALIR
    texturaAux = texto_crear_textura(datosMenuPausa->renderer, datosMenuPausa->fuentes[FUENTE_TAM_64], "SALIR", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }
    if (menu_agregar_opcion(datosMenuPausa->menu, M_PRI_SALIR, texturaAux, 64, (tMenuAccion){_juego_salir_del_juego, datosMenuPausa->estadoJuego}, OPCION_HABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }

    return 0;
}

static void _juego_ventana_menu_actualizar(SDL_Event *evento, void *datos)
{
    tDatosMenuPausa *datosMenuPausa = (tDatosMenuPausa*)datos;
    SDL_Keycode tecla;
    tMenuAccion accionProcesada = {NULL, NULL};

    if (evento->type != SDL_KEYDOWN) {

        return;
    }

    tecla = evento->key.keysym.sym;

    if (datosMenuPausa->logica->estado == LOGICA_FIN_PARTIDA) {

        menu_estado_opcion(datosMenuPausa->menu, M_PRI_CONTINUAR, OPCION_OCULTA);
    } else {

        menu_estado_opcion(datosMenuPausa->menu, M_PRI_CONTINUAR, OPCION_HABILITADA);
    }

    if (tecla == SDLK_ESCAPE && datosMenuPausa->logica->estado == LOGICA_JUGANDO) {

        menu_estado_opcion(datosMenuPausa->menu, M_PRI_CONTINUAR, OPCION_HABILITADA);
        Mix_PlayChannel(0, * (datosMenuPausa->sonidos + SONIDO_MENU_MOVIMIENTO), 0);

    } else if (datosMenuPausa->logica->estado == LOGICA_EN_ESPERA) {

        menu_estado_opcion(datosMenuPausa->menu, M_PRI_CONTINUAR, OPCION_OCULTA);
    }

    if (tecla == SDLK_UP) {

        menu_anterior_opcion(datosMenuPausa->menu);
        Mix_PlayChannel(0, * (datosMenuPausa->sonidos + SONIDO_MENU_MOVIMIENTO), 0);
    } else if (tecla == SDLK_DOWN) {

        menu_siguiente_opcion(datosMenuPausa->menu);
        Mix_PlayChannel(0, * (datosMenuPausa->sonidos + SONIDO_MENU_MOVIMIENTO), 0);
    } else if (tecla == SDLK_RETURN) {

        accionProcesada = menu_confirmar_opcion(datosMenuPausa->menu);
        Mix_PlayChannel(0, * (datosMenuPausa->sonidos + SONIDO_MENU_CONFIRMAR), 0);

        if (accionProcesada.funcion) {

            accionProcesada.funcion(accionProcesada.datos);
        }
    }
}

static int _juego_cargar_assets(tJuego *juego)
{
    int i, ret = ERR_TODO_OK;

    juego->sonidos = malloc(sizeof(Mix_Chunk*) * SONIDO_CANTIDAD);
    if (!juego->sonidos) return ERR_SIN_MEMORIA;

    juego->imagenes = malloc(sizeof(SDL_Texture*) * IMAGEN_CANTIDAD);
    if (!juego->imagenes) {
        free(juego->sonidos);
        return ERR_SIN_MEMORIA;
    }

    if ((ret = assets_cargar_imagenes(juego->renderer, juego->imagenes)) != ERR_TODO_OK) return ret;

    if ((ret = assets_cargar_sonidos(juego->sonidos)) != ERR_TODO_OK) return ret;

    for (i = 0; i < FUENTE_CANTIDAD; i++) {

        if ((ret = assets_cargar_fuente(&juego->fuentes[i], FUENTE_TAM_MIN + (FUENTE_INCREMENTO * i))) != ERR_TODO_OK) {
            return ret;
        }
    }

    return ERR_TODO_OK;
}

static int _juego_crear_hud(tJuego *juego)
{
    SDL_Rect dimsVentana;
    int rendererW = 0, rendererH = 0;
    tDatosMenuPausa *datosMenuPausa;
    tDatosUsuario *datosUsername;

    SDL_GetRendererOutputSize(juego->renderer, &rendererW, &rendererH);

    // Widgets HUD
    juego->hud.widgets[HUD_WIDGET_VIDAS] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_VIDAS_Y}, juego->imagenes[IMAGEN_ICO_VIDAS], juego->fuentes[FUENTE_TAM_48], SDL_COLOR_BLANCO, 0, 1);
    if (!juego->hud.widgets[HUD_WIDGET_VIDAS]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_PREMIOS] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_PREMIOS_Y}, juego->imagenes[IMAGEN_ICO_PREMIOS], juego->fuentes[FUENTE_TAM_48], SDL_COLOR_BLANCO, 0, 1);
    if (!juego->hud.widgets[HUD_WIDGETS_PREMIOS]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_RONDA] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_RONDA_Y}, NULL, juego->fuentes[FUENTE_TAM_64], SDL_COLOR_BLANCO, 0, 1);
    if (!juego->hud.widgets[HUD_WIDGETS_RONDA]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_TEXTO] = widget_crear_texto(juego->renderer, "SIGUIENTE RONDA!", (SDL_Point){rendererW / 2, 24}, juego->fuentes[FUENTE_TAM_32], SDL_COLOR_BLANCO, (SDL_Color){0,0,0,0}, 0);
    if (!juego->hud.widgets[HUD_WIDGETS_TEXTO]) return ERR_SIN_MEMORIA;

    // Ventana Pausa
    dimsVentana.w = VENTANA_MENU_PAUSA_ANCHO;
    dimsVentana.h = VENTANA_MENU_PAUSA_ALTO;
    dimsVentana.x = (rendererW - dimsVentana.w) / 2;
    dimsVentana.y = (rendererH - dimsVentana.h) / 2;

    datosMenuPausa = malloc(sizeof(tDatosMenuPausa));
    if (!datosMenuPausa) return ERR_SIN_MEMORIA;
    datosMenuPausa->fuentes = juego->fuentes;
    datosMenuPausa->sonidos = juego->sonidos;
    datosMenuPausa->logica = &juego->logica;
    datosMenuPausa->estadoJuego = &juego->estado;
    datosMenuPausa->menu = NULL;
    datosMenuPausa->renderer = juego->renderer;
    juego->ventanaMenuPausa = ventana_crear(juego->renderer, (tVentanaAccion){_juego_ventana_menu_crear, _juego_ventana_menu_actualizar, _juego_ventana_menu_dibujar, _juego_ventana_menu_destruir, datosMenuPausa}, dimsVentana, (SDL_Color){200, 162, 200, 255}, 1);
    ventana_abrir(juego->ventanaMenuPausa);

    // Ventana Username
    dimsVentana.w = VENTANA_USERNAME_ANCHO;
    dimsVentana.h = VENTANA_USERNAME_ALTO;
    dimsVentana.x = (rendererW - dimsVentana.w) / 2;
    dimsVentana.y = (rendererH - dimsVentana.h) / 2;

    datosUsername = malloc(sizeof(tDatosUsuario));
    if (!datosUsername) return ERR_SIN_MEMORIA;
    *juego->usuario = '\0';
    datosUsername->renderer = juego->renderer;
    datosUsername->fuentes = juego->fuentes;
    datosUsername->campoTexto = NULL;
    datosUsername->usuario = juego->usuario;
    datosUsername->estadoLogica = &juego->logica.estado;
    juego->ventanaUsername = ventana_crear(juego->renderer, (tVentanaAccion){_juego_ventana_usuario_crear, _juego_ventana_usuario_actualizar, _juego_ventana_usuario_dibujar, _juego_ventana_usuario_destruir, datosUsername}, dimsVentana, (SDL_Color){162, 200, 200, 255}, 1);
    ventana_abrir(juego->ventanaUsername);

    return ERR_TODO_OK;
}


static void _juego_ventana_menu_dibujar(void *datos)
{
    tDatosMenuPausa* datosMenuPausa = (tDatosMenuPausa*)datos;

    menu_dibujar(datosMenuPausa->menu);
}

static void _juego_ventana_menu_destruir(void *datos)
{
    tDatosMenuPausa* datosMenuPausa = (tDatosMenuPausa*)datos;
    if (!datosMenuPausa) {
       return;
    }

    menu_destruir(datosMenuPausa->menu);

    free(datosMenuPausa);
}


static int _juego_ventana_usuario_crear(void *datos)
{
    tDatosUsuario *datosUsuario = (tDatosUsuario*)(datos);

    datosUsuario->campoTexto = widget_crear_campo_texto(datosUsuario->renderer, (SDL_Point){166,60}, datosUsuario->fuentes[FUENTE_TAM_32], SDL_COLOR_BLANCO, (SDL_Color){100, 100, 100, 255}, 1);

    return 0;
}

static void _juego_ventana_usuario_actualizar(SDL_Event *evento, void *datos)
{
    tDatosUsuario *datosUsuario = (tDatosUsuario*)datos;

    if(evento->type == SDL_TEXTINPUT){

        strncat(datosUsuario->usuario, evento->text.text, TAM_USUARIO - strlen(datosUsuario->usuario) - 1);
        widget_modificar_valor(datosUsuario->campoTexto, datosUsuario->usuario);

    }else if(evento->type == SDL_KEYDOWN && evento->key.keysym.sym == SDLK_BACKSPACE){

        size_t longitud = strlen(datosUsuario->usuario);

        if (longitud > 0) {
            datosUsuario->usuario[longitud - 1] = '\0';
        }
        widget_modificar_valor(datosUsuario->campoTexto, datosUsuario->usuario);
    } else if (evento->type == SDL_KEYDOWN && evento->key.keysym.sym == SDLK_RETURN) {

        *datosUsuario->estadoLogica = LOGICA_EN_ESPERA;
    }
}

static void _juego_ventana_usuario_dibujar(void *datos)
{
    tDatosUsuario *datosUsuario = (tDatosUsuario*)datos;

    widget_dibujar(datosUsuario->campoTexto);
}

static void _juego_ventana_usuario_destruir(void *datos)
{
    tDatosUsuario *datosUsuario = (tDatosUsuario*)datos;
    if (!datosUsuario) {
       return;
    }

    widget_destruir(datosUsuario->campoTexto);

    free(datosUsuario);
}
