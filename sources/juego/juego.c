#include "../../include/juego/juego.h"
#include "../../include/juego/assets.h"
#include "../../include/comun/comun.h"
#include "../../include/juego/dibujado.h"
#include "../../include/juego/graficos.h"
#include "../../include/comun/protocolo.h"
#include "../../include/comun/mensaje.h"

#define FUENTE_TAM_MIN 32
#define FUENTE_INCREMENTO 16
#define HUD_X 16
#define HUD_VIDAS_Y 16
#define HUD_PREMIOS_Y 96
#define HUD_RONDA_Y 160
#define WIDGET_VISIBLE 1
#define WIDGET_OCULTO 0
#define VENTANA_MENU_PAUSA_ANCHO 384
#define VENTANA_MENU_PAUSA_ALTO 512
#define VENTANA_USERNAME_ANCHO 454
#define VENTANA_USERNAME_ALTO 256
#define VENTANA_RANKING_ANCHO 710
#define VENTANA_RANKING_ALTO 474

typedef enum {
    SOLICITUD_PROCESAR_TODAS = -1,
    SOLICITUD_PROCESAR_NINGUNA = 0,
    SOLICITUD_PROCESAR_UNA = 1
} eModoSolicitud;

typedef enum {
    M_PRI_NUEVA_PARTIDA,
    M_PRI_CONTINUAR,
    M_PRI_CAMBIAR_USUARIO,
    M_PRI_RANKING,
    M_PRI_SALIR,
    M_PRI_CANTIDAD
} eMenuOpcionID;

static int _juego_crear_ventana(SDL_Window **ventana, SDL_Renderer **renderer, unsigned anchoRes, unsigned altoRes, const char *tituloVentana);
static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tVentana *ventanaMenu, tVentana *ventanaUsername, tVentana *ventanaRanking, tHud *hud);
static void _juego_iniciar_partida(void* datos);
static void _juego_salir_del_juego(void* datos);
static void _juego_continuar_partida(void* datos);
static void _juego_mostrar_ranking(void *datos);
static int _juego_actualizar_hud(tHud *hud, tLogica *logica);
static int _juego_crear_hud(tJuego *juego);
static int _juego_cargar_assets(tJuego *juego);
static void _juego_manejar_input(tJuego *juego, SDL_Keycode tecla);
static void _juego_manejar_eventos(tJuego *juego);
static int _juego_encolar_solicitud(tCola *colaSolicitudes, tCola *colaContexto, const char *solicitud, eColaContexto contexto);
static int _juego_procesar_cola_solicitudes(SOCKET sock, FILE *archContingencia, eEstadoSesion *sesion, tCola *colaSolicitudes, eModoSolicitud modo);
void _juego_inicializar_base_datos(tJuego *juego);
int _juego_buscar_respuesta_datos(tJuego *juego, int *codigoRetorno, int *cantRegistros, int *tamRegistro);
int _juego_procesar_contingencia(tJuego *juego);
static void _juego_procesar_cola_respuestas(tJuego *juego);
static void _juego_configurar_menu(tJuego *juego);

static int _juego_ventana_menu_crear(void *datos);
static void _juego_ventana_menu_actualizar(SDL_Event *evento, void *datos);
static void _juego_ventana_menu_dibujar(void *datos);
static void _juego_ventana_menu_destruir(void *datos);

static int _juego_ventana_usuario_crear(void *datos);
static void _juego_ventana_usuario_actualizar(SDL_Event *evento, void *datos);
static void _juego_ventana_usuario_dibujar(void *datos);
static void _juego_ventana_usuario_destruir(void *datos);

static void _juego_ventana_ranking_actualizar(SDL_Event *evento, void *datos);
static void _juego_ventana_ranking_dibujar(void *datos);


int juego_inicializar(tJuego *juego, const char *tituloVentana, SOCKET sock, eEstadoSesion sesion)
{
    int ret = ERR_TODO_OK;

    mensaje_subtitulo("Inicializando juego");

    memset(juego, 0, sizeof(tJuego));
    juego->estado = JUEGO_NO_INICIADO;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != ERR_TODO_OK) {
        mensaje_error("No se pudo inicializar SDL");
        mensaje_color(TEXTO_ROJO_B, "%s", SDL_GetError());
        return ERR_SDL_INI;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        mensaje_error("No se pudo inicializar SDL_mixer");
        mensaje_color(TEXTO_ROJO_B, "%s", Mix_GetError());
        SDL_Quit();
        return ERR_SDL_INI;
    }

    if (TTF_Init() < 0) {
        mensaje_error("No se pudo inicializar SDL_ttf");
        mensaje_color(TEXTO_ROJO_B, "%s", TTF_GetError());
        return ERR_SDL_INI;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        mensaje_error("No se pudo inicializar SDL_image");
        mensaje_color(TEXTO_ROJO_B, "%s", IMG_GetError());
        ret = ERR_SDL_INI;
    }

    if (logica_inicializar(&juego->logica) != ERR_TODO_OK) {
        mensaje_error("No se pudo inicializar la logica del juego");
        return ERR_LOGICA;
    }
    logica_calc_min_res(&juego->logica, &juego->anchoRes, &juego->altoRes);
    juego->anchoRes += PADDING_MARGEN * 2;
    juego->altoRes += PADDING_MARGEN * 2;

    if (_juego_crear_ventana(&juego->ventana, &juego->renderer, juego->anchoRes, juego->altoRes, tituloVentana) != ERR_TODO_OK) {
        mensaje_error("No se pudo crear la ventana");
        return ERR_VENTANA;
    }

    if ((ret = _juego_cargar_assets(juego)) != ERR_TODO_OK) return ret;
    if ((ret = _juego_crear_hud(juego)) != ERR_TODO_OK) return ret;

    cola_crear(&juego->colaSolicitudes);
    cola_crear(&juego->colaContextos);
    cola_crear(&juego->colaRespuestas);
    juego->sock = sock;
    juego->sesion = sesion;
    juego->estado = JUEGO_CORRIENDO;

    if (comun_crear_directorio("rondas") != ERR_TODO_OK) {
        mensaje_advertencia("No se pudo crear el directorio /rondas. Los .txt de la ronda no se guardaran.");
    }

    juego->archContingencia = fopen("contingencia.txt", "a+");
    if (!juego->archContingencia) {
        return ERR_ARCHIVO;
    }

    mensaje_todo_ok("Juego iniciado con exito.");

    return ret;
}

void _juego_inicializar_base_datos(tJuego *juego)
{
    char solicitud[TAM_BUFFER];

    sprintf(solicitud, "CREAR partidas (idPartida ENTERO PK AI, username TEXTO(16), cantPremios ENTERO, cantMovs ENTERO, semilla ENTERO)");
    _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, solicitud, CONTEXTO_CREAR_BASE);

    sprintf(solicitud, "CREAR jugadores (username TEXTO(16) PK, record ENTERO IS, cantPartidas ENTERO)");
    _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, solicitud, CONTEXTO_CREAR_BASE);
}

int _juego_procesar_contingencia(tJuego *juego)
{
    char solicitud[TAM_BUFFER], solicitudAux[TAM_BUFFER];
    tJugador jugadorAux = {0};
    FILE *archContingenciaTmp = NULL;
    fseek(juego->archContingencia, 0, SEEK_SET);

    mensaje_advertencia("Sincronizando progreso Offline de partidas anteriores. Espere" PARPADEO "..." TEXTO_RESET "\n");

    while (fgets(solicitud, TAM_BUFFER, juego->archContingencia)) {

        if (juego->sesion == SESION_ONLINE) {

            if (strstr(solicitud, "ACTUALIZAR jugadores")) {

                char username[TAM_USUARIO], campos[256] = {0}, *cursor;
                int record = -1, cantPartidas = -1;

                cursor = strstr(solicitud, "username IGUAL ");
                if (cursor && sscanf(cursor, "username IGUAL %[^)]", username) != 1) {
                    return ERR_ARCHIVO;
                }

                cursor = strstr(solicitud, "record ");
                if (cursor) {
                     sscanf(cursor, "record %d", &record);
                }

                cursor = strstr(solicitud, "cantPartidas ");
                if (cursor) {
                     sscanf(cursor, "cantPartidas %d", &cantPartidas);
                }

                if (*jugadorAux.username == '\0' || strcmp(jugadorAux.username, username) != 0) {

                    cola_vaciar(&juego->colaRespuestas);

                    sprintf(solicitudAux, "SELECCIONAR jugadores DONDE (username IGUAL %s)", username);
                    if (cliente_enviar_solicitud(juego->sock, solicitudAux) == CE_ERR_SOCKET || cliente_recibir_respuesta(juego->sock, &juego->colaRespuestas) == CE_ERR_SOCKET) {
                        juego->sesion = SESION_OFFLINE;
                        mensaje_advertencia("Socket desconectado, modo offline activo");
                    } else {

                        char cabecera[TAM_BUFFER];
                        int codigoRet, cantReg;

                        if (cola_desencolar(&juego->colaRespuestas, cabecera, TAM_BUFFER) != COLA_VACIA) {
                            sscanf(cabecera, "%d;%*[^;];%d", &codigoRet, &cantReg);

                            if (codigoRet == BD_DATOS_OBTENIDOS && cantReg == 1) {
                                char registroStr[TAM_BUFFER];
                                if (cola_desencolar(&juego->colaRespuestas, registroStr, TAM_BUFFER) != COLA_VACIA) {
                                    sscanf(registroStr, "%[^;];%d;%d", jugadorAux.username, &jugadorAux.record, &jugadorAux.cantPartidas);
                                }
                            }
                        }
                    }
                }

                if (record > jugadorAux.record) {
                    sprintf(campos, "record %d", record);
                }

                if (cantPartidas > jugadorAux.cantPartidas) {
                    char aux[64];
                    if (*campos != '\0') {
                        strcat(campos, ", ");
                    }
                    sprintf(aux, "cantPartidas %d", cantPartidas);
                    strcat(campos, aux);
                }

                if (*campos != '\0') {
                    snprintf(solicitud, TAM_BUFFER, "ACTUALIZAR jugadores (%s) DONDE (username IGUAL %s)", campos, username);
                    _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, solicitud, CONTEXTO_IRRELEVANTE);
                }

            } else {
                _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, solicitud, CONTEXTO_IRRELEVANTE);
            }
        } else {

            if (!archContingenciaTmp) {
                archContingenciaTmp = fopen("contingenciaTmp.txt", "w");
                if (!archContingenciaTmp) {
                    return ERR_ARCHIVO;
                }
            }

            fprintf(archContingenciaTmp, "%s", solicitud);
            mensaje_info("Guardando datos futura conexion al servidor.");
            mensaje_debug(solicitud);
        }
    }

    fclose(juego->archContingencia);

    if (archContingenciaTmp) {
        fclose(archContingenciaTmp);
        if (remove("contingencia.txt") != 0) return ERR_ARCHIVO;
        if (rename("contingencia.tmp", "contingencia.txt") != 0) return ERR_ARCHIVO;
        juego->archContingencia = fopen("contingencia.txt", "a+");
        if (!juego->archContingencia) return ERR_ARCHIVO;
    } else {
        juego->archContingencia = fopen("contingencia.txt", "w");
        if (!juego->archContingencia) return ERR_ARCHIVO;
    }

    return ERR_TODO_OK;
}

int juego_ejecutar(tJuego *juego)
{
    eRetorno ret = ERR_TODO_OK;
    if (juego->sesion == SESION_ONLINE) {
        _juego_inicializar_base_datos(juego);
        //_juego_procesar_contingencia(juego);
       // _juego_procesar_cola_solicitudes(juego->sock, juego->archContingencia, &juego->sesion, &juego->colaSolicitudes, SOLICITUD_PROCESAR_TODAS);
        menu_estado_opcion(juego->hud.menu, M_PRI_RANKING, OPCION_HABILITADA);
    }

    while (juego->estado == JUEGO_CORRIENDO) {

        _juego_manejar_eventos(juego);

        if (juego->logica.estado == LOGICA_JUGANDO) {

            logica_actualizar(&juego->logica);
            _juego_actualizar_hud(&juego->hud, &juego->logica);
        } else if (juego->logica.estado == LOGICA_EN_ESPERA || juego->logica.estado == LOGICA_EN_RANKING){

            if (cola_vacia(&juego->colaSolicitudes) != COLA_VACIA) {

                if (_juego_procesar_cola_solicitudes(juego->sock, juego->archContingencia, &juego->sesion, &juego->colaSolicitudes, SOLICITUD_PROCESAR_UNA) == ERR_TODO_OK) {

                    cliente_recibir_respuesta(juego->sock, &juego->colaRespuestas); // revisar retorno

                    if (juego->sesion == SESION_ONLINE) {
                        _juego_procesar_cola_respuestas(juego);
                    }
                }
            }
        }

        _juego_renderizar(juego->renderer, juego->imagenes, &juego->logica, juego->ventanaMenuPausa, juego->ventanaUsername, juego->ventanaRanking, &juego->hud);

        SDL_Delay(16);
    }

    _juego_procesar_cola_solicitudes(juego->sock, juego->archContingencia, &juego->sesion, &juego->colaSolicitudes, SOLICITUD_PROCESAR_TODAS);

    return ret;
}

void juego_destruir(tJuego *juego)
{
    int i;

    if (juego->imagenes) assets_destuir_imagenes(juego->imagenes);
    if (juego->sonidos) assets_destuir_sonidos(juego->sonidos);

    for (i = 0; i < FUENTE_CANTIDAD; i++){
        if (juego->fuentes[i]) {
            assets_destruir_fuente(juego->fuentes[i]);
        }
    }

    for (i = 0; i < HUD_WIDGETS_CANTIDAD; i++) {
        if (juego->hud.widgets[i]) {
            widget_destruir(juego->hud.widgets[i]);
        }
    }

    if (juego->ventanaMenuPausa) ventana_destruir(juego->ventanaMenuPausa);
    if (juego->ventanaUsername) ventana_destruir(juego->ventanaUsername);
    if (juego->ventanaRanking) ventana_destruir(juego->ventanaRanking);

    logica_destruir(&juego->logica);

    if (juego->renderer) SDL_DestroyRenderer(juego->renderer);
    if (juego->ventana) SDL_DestroyWindow(juego->ventana);

    IMG_Quit();
    TTF_Quit();
    Mix_CloseAudio();
    SDL_Quit();

    cola_vaciar(&juego->colaContextos);
    cola_vaciar(&juego->colaSolicitudes);
    cola_vaciar(&juego->colaRespuestas);

    juego->estado = JUEGO_CERRANDO;
}


/*************************
    FUNCIONES ESTATICAS
*************************/

static int _juego_procesar_cola_solicitudes(SOCKET sock, FILE *archContingencia, eEstadoSesion *sesion, tCola *colaSolicitudes, eModoSolicitud modo)
{
    char solicitud[TAM_BUFFER] = {0};
    int retorno, procesadas = 0;

    if (modo == SOLICITUD_PROCESAR_NINGUNA) {
        return ERR_EN_ESPERA;
    }

    while ((modo == SOLICITUD_PROCESAR_TODAS || procesadas < modo) && cola_desencolar(colaSolicitudes, solicitud, TAM_BUFFER) != COLA_VACIA) {

        if (strlen(solicitud) + 1 < TAM_BUFFER) {
            strcat(solicitud, "\n");
        } else {
            mensaje_advertencia("La solicitud excede el tamaño del buffer y no puede enviarse");
        }

        if (*sesion == SESION_ONLINE) {
            retorno = cliente_enviar_solicitud(sock, solicitud);
            if (retorno == CE_ERR_SOCKET) {
                *sesion = SESION_OFFLINE;
                mensaje_advertencia("Socket desconectado, modo offline activo");
            }
        }

        if (*sesion == SESION_OFFLINE && (strncmp(solicitud, "SELECCIONAR", strlen("SELECCIONAR")) != 0 && strncmp(solicitud, "CREAR", strlen("CREAR")) != 0)) {
            fprintf(archContingencia, "%s", solicitud);
            mensaje_info("Guardando datos futura conexion al servidor.");
            mensaje_debug(solicitud);
        }
        ++procesadas;
    }

    return *sesion == SESION_ONLINE ? ERR_TODO_OK : ERR_OFFLINE;
}

static int _juego_encolar_solicitud(tCola *colaSolicitudes, tCola *colaContexto, const char *solicitud, eColaContexto contexto)
{
    cola_encolar(colaSolicitudes, solicitud, strlen(solicitud));
    cola_encolar(colaContexto, &contexto, sizeof(eColaContexto));

    return ERR_TODO_OK;
}

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
                    char buffer[TAM_BUFFER];

                    sprintf(buffer, "SELECCIONAR jugadores DONDE (username IGUAL %s)", juego->jugador.username);
                    _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, buffer, CONTEXTO_DATOS_JUGADOR);

                    ventana_cerrar(juego->ventanaUsername);
                    widget_modificar_valor(juego->hud.widgets[HUD_WIDGETS_USERNAME], juego->jugador.username);
                }
                break;
            case LOGICA_RESULTADO:
                /* CAIDA */
            case LOGICA_JUGANDO:
                if (evento.type == SDL_KEYDOWN && TECLA_VALIDA(evento.key.keysym.sym)) {
                    tecla = evento.key.keysym.sym;
                    _juego_manejar_input(juego, tecla);
                }
                break;
            case LOGICA_EN_RANKING:
                ventana_actualizar(juego->ventanaRanking, &evento);
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
        menu_estado_opcion(juego->hud.menu, M_PRI_CONTINUAR, OPCION_HABILITADA);
        juego->logica.estado = LOGICA_EN_PAUSA;
        _juego_configurar_menu(juego);
        Mix_PlayChannel(-1, juego->sonidos[SONIDO_MENU_MOVIMIENTO], 0);
        return;
    }

    if (logica_procesar_turno(&juego->logica, tecla)) {

        Mix_PlayChannel(-1, juego->sonidos[SONIDO_JUGADOR_MOV], 0);
        logica_procesar_movimientos(&juego->logica);
    }

    if (juego->logica.estado == LOGICA_FIN_PARTIDA) {
        char buffer[TAM_BUFFER];
        int cantMovs = 0;

        Mix_PlayChannel(-1, juego->sonidos[SONIDO_JUGADOR_GAMEOVER], 0);

        widget_modificar_visibilidad(juego->hud.widgets[HUD_WIDGETS_PERDISTE], 1);

        juego->logica.estado = LOGICA_RESULTADO;
        cantMovs = logica_mostrar_historial_movs(&juego->logica.movsJugador);

        ++juego->jugador.cantPartidas;

        sprintf(buffer, "INSERTAR partidas (username %s, cantPremios %d, cantMovs %d, semilla %d)", juego->jugador.username, juego->logica.ronda.cantPremios, cantMovs, (int)juego->logica.semillaMaestra);
        _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, buffer, CONTEXTO_IRRELEVANTE);
        if (juego->logica.ronda.cantPremios > juego->jugador.record) {
            juego->jugador.record = juego->logica.ronda.cantPremios;
            sprintf(buffer, "ACTUALIZAR jugadores (record %d, cantPartidas %d) DONDE (username IGUAL %s)", juego->jugador.record, juego->jugador.cantPartidas, juego->jugador.username);
        } else {
            sprintf(buffer, "ACTUALIZAR jugadores (cantPartidas %d) DONDE (username IGUAL %s)", juego->jugador.cantPartidas, juego->jugador.username);
        }

         _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, buffer, CONTEXTO_IRRELEVANTE);
    }else if (juego->logica.estado == LOGICA_RESULTADO && tecla == SDLK_RETURN) {
        juego->logica.estado = LOGICA_EN_ESPERA;
        _juego_configurar_menu(juego);
        widget_modificar_visibilidad(juego->hud.widgets[HUD_WIDGETS_PERDISTE], 0);
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
    for (int i = 0; i < HUD_WIDGETS_CANTIDAD; i++) {

        widget_dibujar(hud->widgets[i]);
    }
}

static void _juego_configurar_menu(tJuego *juego)
{
    menu_estado_opcion(juego->hud.menu, M_PRI_CONTINUAR, OPCION_OCULTA);
    menu_estado_opcion(juego->hud.menu, M_PRI_NUEVA_PARTIDA, OPCION_HABILITADA);

    if (juego->sesion == SESION_ONLINE) {
        menu_estado_opcion(juego->hud.menu, M_PRI_RANKING, OPCION_HABILITADA);
    } else {
        menu_estado_opcion(juego->hud.menu, M_PRI_RANKING, OPCION_DESHABILITADA);
    }

    switch (juego->logica.estado) {
        case LOGICA_EN_PAUSA:
            menu_estado_opcion(juego->hud.menu, M_PRI_CONTINUAR, OPCION_HABILITADA);
            menu_estado_opcion(juego->hud.menu, M_PRI_RANKING, OPCION_OCULTA);
            break;
        case LOGICA_EN_ESPERA:
        case LOGICA_FIN_PARTIDA:
        case LOGICA_RESULTADO:
            menu_estado_opcion(juego->hud.menu, M_PRI_CONTINUAR, OPCION_OCULTA);
            break;

        default:
            break;
    }
}

static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tVentana *ventanaMenu, tVentana *ventanaUsername, tVentana *ventanaRanking, tHud *hud)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (logica->estado == LOGICA_JUGANDO || logica->estado == LOGICA_RESULTADO) {
        dibujado_escenario(renderer, &logica->escenario, imagenes);
        _juego_dibujar_hud(hud, renderer);
    } else {
        graficos_dibujar_mosaico(renderer, imagenes[IMAGEN_TSET_CASTILLO], &(SDL_Rect){0,192,96,96}, 2.0);
        ventana_dibujar(ventanaMenu);
        ventana_dibujar(ventanaUsername);
        ventana_dibujar(ventanaRanking);
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
        mensaje_color(TEXTO_ROJO, "[ERROR] %s\n", SDL_GetError());
        return ERR_VENTANA;
    }

    *renderer = SDL_CreateRenderer(*ventana, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        SDL_DestroyWindow(*ventana);
        mensaje_color(TEXTO_ROJO, "[ERROR] %s\n", SDL_GetError());
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
    mensaje_info("Iniciando partida");
    logica_iniciar_juego(logica);
}

static void _juego_salir_del_juego(void *datos)
{
    eJuegoEstado *estado = (eJuegoEstado*)datos;
    mensaje_info("Saliendo");
    *estado = JUEGO_CERRANDO;
}

static void _juego_continuar_partida(void *datos)
{
    tLogica *logica = (tLogica*)datos;
    mensaje_info("Continuar partida");
    logica->estado = LOGICA_JUGANDO;
}

static void _juego_mostrar_ranking(void *datos)
{
    tJuego *juego = (tJuego*)(datos);
    _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, "SELECCIONAR jugadores DONDE (record TOP 6)", CONTEXTO_RANKING);
    ventana_abrir(juego->ventanaRanking);
    juego->logica.estado = LOGICA_EN_RANKING;
}

static int _juego_ventana_menu_crear(void *datos)
{
    tJuego *juego = (tJuego*)(datos);
    SDL_Texture *texturaAux = NULL;

    juego->hud.menu = menu_crear(juego->renderer, 2, (SDL_Point) {32, 32}, MENU_VERTICAL);
    if (!juego->hud.menu) {
        return -1;
    }

    // NUEVA PARTIDA
    texturaAux = texto_crear_textura(juego->renderer, juego->fuentes[FUENTE_TAM_64], "NUEVA PARTIDA", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(juego->hud.menu);
        return -1;
    }
    if (menu_agregar_opcion(juego->hud.menu, M_PRI_NUEVA_PARTIDA, texturaAux, 64, (tMenuAccion){_juego_iniciar_partida, &juego->logica}, OPCION_HABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(juego->hud.menu);
        return -1;
    }

    // CONTINUAR
    texturaAux = texto_crear_textura(juego->renderer, juego->fuentes[FUENTE_TAM_64], "CONTINUAR", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(juego->hud.menu);
        return -1;
    }
    if (menu_agregar_opcion(juego->hud.menu, M_PRI_CONTINUAR, texturaAux, 64, (tMenuAccion){_juego_continuar_partida, &juego->logica}, OPCION_OCULTA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(juego->hud.menu);
        return -1;
    }

    // LOGIN USUARIO
    texturaAux = texto_crear_textura(juego->renderer, juego->fuentes[FUENTE_TAM_64], "LOGIN USUARIO", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(juego->hud.menu);
        return -1;
    }
    if (menu_agregar_opcion(juego->hud.menu, M_PRI_CAMBIAR_USUARIO, texturaAux, 64, (tMenuAccion){NULL, NULL}, OPCION_DESHABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(juego->hud.menu);
        return -1;
    }

    // RANKING
    texturaAux = texto_crear_textura(juego->renderer, juego->fuentes[FUENTE_TAM_64], "RANKING", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(juego->hud.menu);
        return -1;
    }
    if (menu_agregar_opcion(juego->hud.menu, M_PRI_RANKING, texturaAux, 64, (tMenuAccion){_juego_mostrar_ranking, juego}, OPCION_DESHABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(juego->hud.menu);
        return -1;
    }

    // SALIR
    texturaAux = texto_crear_textura(juego->renderer, juego->fuentes[FUENTE_TAM_64], "SALIR", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(juego->hud.menu);
        return -1;
    }
    if (menu_agregar_opcion(juego->hud.menu, M_PRI_SALIR, texturaAux, 64, (tMenuAccion){_juego_salir_del_juego, &juego->estado}, OPCION_HABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(juego->hud.menu);
        return -1;
    }

    return 0;
}

static void _juego_ventana_menu_actualizar(SDL_Event *evento, void *datos)
{
    tJuego *juego = (tJuego*)datos;
    SDL_Keycode tecla;
    tMenuAccion accionProcesada = {NULL, NULL};

    if (evento->type != SDL_KEYDOWN) {
        return;
    }

    tecla = evento->key.keysym.sym;

    if (tecla == SDLK_ESCAPE && juego->logica.estado == LOGICA_JUGANDO) {

        menu_estado_opcion(juego->hud.menu, M_PRI_CONTINUAR, OPCION_HABILITADA);
        Mix_PlayChannel(0, * (juego->sonidos + SONIDO_MENU_MOVIMIENTO), 0);

    } else if (juego->logica.estado == LOGICA_EN_ESPERA) {

        menu_estado_opcion(juego->hud.menu, M_PRI_CONTINUAR, OPCION_OCULTA);
    }

    if (tecla == SDLK_UP) {

        menu_anterior_opcion(juego->hud.menu);
        Mix_PlayChannel(0, * (juego->sonidos + SONIDO_MENU_MOVIMIENTO), 0);
    } else if (tecla == SDLK_DOWN) {

        menu_siguiente_opcion(juego->hud.menu);
        Mix_PlayChannel(0, * (juego->sonidos + SONIDO_MENU_MOVIMIENTO), 0);
    } else if (tecla == SDLK_RETURN) {

        accionProcesada = menu_confirmar_opcion(juego->hud.menu);
        Mix_PlayChannel(0, * (juego->sonidos + SONIDO_MENU_CONFIRMAR), 0);

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
    int rendererW = juego->anchoRes, rendererH = juego->altoRes;

    // Widgets HUD
    juego->hud.widgets[HUD_WIDGET_VIDAS] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_VIDAS_Y}, juego->imagenes[IMAGEN_ICO_VIDAS], juego->fuentes[FUENTE_TAM_48], SDL_COLOR_BLANCO, 0, WIDGET_VISIBLE);
    if (!juego->hud.widgets[HUD_WIDGET_VIDAS]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_PREMIOS] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_PREMIOS_Y}, juego->imagenes[IMAGEN_ICO_PREMIOS], juego->fuentes[FUENTE_TAM_48], SDL_COLOR_BLANCO, 0, WIDGET_VISIBLE);
    if (!juego->hud.widgets[HUD_WIDGETS_PREMIOS]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_RONDA] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_RONDA_Y}, NULL, juego->fuentes[FUENTE_TAM_64], SDL_COLOR_BLANCO, 0, WIDGET_VISIBLE);
    if (!juego->hud.widgets[HUD_WIDGETS_RONDA]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_USERNAME] = widget_crear_texto(juego->renderer, NULL, (SDL_Point){rendererW / 2, rendererH - 32}, juego->fuentes[FUENTE_TAM_32], SDL_COLOR_BLANCO, (SDL_Color){0,0,0,0}, WIDGET_VISIBLE);
    if (!juego->hud.widgets[HUD_WIDGETS_USERNAME]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_TEXTO] = widget_crear_texto(juego->renderer, "¡SIGUIENTE RONDA!", (SDL_Point){rendererW / 2, 24}, juego->fuentes[FUENTE_TAM_32], SDL_COLOR_BLANCO, (SDL_Color){0,0,0,0}, WIDGET_OCULTO);
    if (!juego->hud.widgets[HUD_WIDGETS_TEXTO]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_PERDISTE] = widget_crear_texto(juego->renderer, "¡GAME OVER!", (SDL_Point){rendererW / 2, rendererH / 2}, juego->fuentes[FUENTE_TAM_64], (SDL_Color){255, 30, 30, 255}, (SDL_Color){18, 5, 35, 196}, WIDGET_OCULTO);
    if (!juego->hud.widgets[HUD_WIDGETS_PERDISTE]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_CAMPO_USERNAME] = widget_crear_campo_texto(juego->renderer, (SDL_Point){VENTANA_USERNAME_ANCHO/2,128}, juego->fuentes[FUENTE_TAM_32], SDL_COLOR_BLANCO, (SDL_Color){100, 100, 100, 255}, 1);
    if (!juego->hud.widgets[HUD_WIDGETS_CAMPO_USERNAME]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGET_LINEA] = widget_crear_texto(juego->renderer, " ", (SDL_Point){0, 0}, juego->fuentes[FUENTE_TAM_32], (SDL_Color){253, 253, 0, 255}, (SDL_Color){16, 8, 32, 255}, WIDGET_OCULTO);
    if (!juego->hud.widgets[HUD_WIDGET_LINEA]) return ERR_SIN_MEMORIA;

    // Ventana Pausa
    dimsVentana.w = VENTANA_MENU_PAUSA_ANCHO;
    dimsVentana.h = VENTANA_MENU_PAUSA_ALTO;
    dimsVentana.x = (rendererW - dimsVentana.w) / 2;
    dimsVentana.y = (rendererH - dimsVentana.h) / 2;

    juego->ventanaMenuPausa = ventana_crear(juego->renderer, (tVentanaAccion){_juego_ventana_menu_crear, _juego_ventana_menu_actualizar, _juego_ventana_menu_dibujar, _juego_ventana_menu_destruir, juego}, dimsVentana, (SDL_Color){200, 162, 200, 255}, 1);
    ventana_abrir(juego->ventanaMenuPausa);

    // Ventana Username
    dimsVentana.w = VENTANA_USERNAME_ANCHO;
    dimsVentana.h = VENTANA_USERNAME_ALTO;
    dimsVentana.x = (rendererW - dimsVentana.w) / 2;
    dimsVentana.y = (rendererH - dimsVentana.h) / 2;

    juego->ventanaUsername = ventana_crear(juego->renderer, (tVentanaAccion){_juego_ventana_usuario_crear, _juego_ventana_usuario_actualizar, _juego_ventana_usuario_dibujar, _juego_ventana_usuario_destruir, juego}, dimsVentana, (SDL_Color){30, 16, 63, 196}, 1);
    ventana_abrir(juego->ventanaUsername);

    dimsVentana.w = VENTANA_RANKING_ANCHO;
    dimsVentana.h = VENTANA_RANKING_ALTO;
    dimsVentana.x = (rendererW - dimsVentana.w) / 2;
    dimsVentana.y = (rendererH - dimsVentana.h) / 2;

    juego->ventanaRanking = ventana_crear(juego->renderer, (tVentanaAccion){NULL, _juego_ventana_ranking_actualizar, _juego_ventana_ranking_dibujar, NULL, juego}, dimsVentana, (SDL_Color){30, 16, 63, 196}, 1);

    return ERR_TODO_OK;
}


static void _juego_ventana_menu_dibujar(void *datos)
{
    tJuego *juego = (tJuego*)datos;

    menu_dibujar(juego->hud.menu);
}

static void _juego_ventana_menu_destruir(void *datos)
{
    tJuego *juego = (tJuego*)datos;

    menu_destruir(juego->hud.menu);

}

static int _juego_ventana_usuario_crear(void *datos)
{
    SDL_StartTextInput();
    return 0;
}


static void _juego_ventana_usuario_actualizar(SDL_Event *evento, void *datos)
{
    tJuego *juego = (tJuego*)datos;
    char buffer[TAM_BUFFER];

    if(evento->type == SDL_TEXTINPUT){

        char letra[2] = {*evento->text.text, '\0'};

        *letra &= ~0x20;

        strncat(juego->jugador.username, letra, TAM_USUARIO - strlen(juego->jugador.username) - 1);
        widget_modificar_valor(juego->hud.widgets[HUD_WIDGETS_CAMPO_USERNAME], juego->jugador.username);

    }else if(evento->type == SDL_KEYDOWN && evento->key.keysym.sym == SDLK_BACKSPACE){

        size_t longitud = strlen(juego->jugador.username);

        if (longitud > 0) {
            juego->jugador.username[longitud - 1] = '\0';
        }
        widget_modificar_valor(juego->hud.widgets[HUD_WIDGETS_CAMPO_USERNAME], juego->jugador.username);
    } else if (*juego->jugador.username && evento->type == SDL_KEYDOWN && evento->key.keysym.sym == SDLK_RETURN) {

        sprintf(buffer, "INSERTAR jugadores (username %s)", juego->jugador.username);
        _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, buffer, CONTEXTO_INSERTAR_JUGADOR);
        widget_modificar_visibilidad(juego->hud.widgets[HUD_WIDGETS_CAMPO_USERNAME], WIDGET_OCULTO);
        juego->logica.estado = LOGICA_EN_ESPERA;
    }
}

static void _juego_procesar_cola_respuestas(tJuego *juego)
{
    char cabecera[TAM_BUFFER];
    char registroStr[TAM_BUFFER];
    int codigoRet, cantReg;
    char mensaje[TAM_BUFFER];
    eColaContexto contexto;

    if (cola_desencolar(&juego->colaContextos, &contexto, sizeof(eColaContexto)) == COLA_VACIA) {
        return;
    }

    if (cola_desencolar(&juego->colaRespuestas, cabecera, TAM_BUFFER) == COLA_VACIA) {
        return;
    }

    sscanf(cabecera, "%d;%[^;];%d", &codigoRet, mensaje, &cantReg);
    mensaje_color(codigoRet > BD_ERROR_SIN_RESULTADOS ? TEXTO_ROJO : TEXTO_VERDE, "Respuesta: %d, %s, %d", codigoRet, mensaje, cantReg);

    if (codigoRet < BD_TODO_OK || cantReg == 0) {

        if (contexto == CONTEXTO_RANKING) {

            juego->rankingCant = 0;
        }
        return;
    }

    switch (contexto) {
        case CONTEXTO_DATOS_JUGADOR:
            if (codigoRet == BD_DATOS_OBTENIDOS && cantReg == 1) {

                cola_desencolar(&juego->colaRespuestas, registroStr, TAM_BUFFER);
                sscanf(registroStr, "%[^;];%d;%d", juego->jugador.username, &juego->jugador.record, &juego->jugador.cantPartidas);
            }
            break;
        case CONTEXTO_RANKING: {
            juego->rankingCant = MIN(cantReg, TOP_CANT);
            for (int i = 0; i < juego->rankingCant; i++) {

                cola_desencolar(&juego->colaRespuestas, registroStr, TAM_BUFFER);
                sscanf(registroStr, "%[^;];%d", juego->ranking[i].username, &juego->ranking[i].record);
            }
            break;
        }
        default:
            cola_vaciar(&juego->colaRespuestas);
            break;
    }
}

static void _juego_ventana_usuario_dibujar(void *datos)
{
    tJuego *juego = (tJuego*)datos;

    char buffer[128];
    SDL_Point nuevaPos = {(VENTANA_USERNAME_ANCHO / 2) - 4, 32};

    tWidget* widget = juego->hud.widgets[HUD_WIDGET_LINEA];
    widget_modificar_visibilidad(widget, WIDGET_VISIBLE);

    sprintf(buffer, " Ingrese su nombre ");
    widget_modificar_valor(widget, buffer);
    widget_modificar_posicion(widget, nuevaPos);
    widget_dibujar(juego->hud.widgets[HUD_WIDGET_LINEA]);

    sprintf(buffer, "ENTER para continuar");
    widget_modificar_valor(widget, buffer);
    nuevaPos.y = 216;
    widget_modificar_posicion(widget, nuevaPos);
    widget_dibujar(juego->hud.widgets[HUD_WIDGET_LINEA]);

    widget_dibujar(juego->hud.widgets[HUD_WIDGETS_CAMPO_USERNAME]);

    widget_modificar_visibilidad(widget, WIDGET_OCULTO);
}

static void _juego_ventana_usuario_destruir(void *datos)
{
    SDL_StopTextInput();
}

static void _juego_ventana_ranking_actualizar(SDL_Event *evento, void *datos)
{
    tJuego *juego = (tJuego*)datos;

    if (evento->type == SDL_KEYDOWN) {

        if (evento->key.keysym.sym == SDLK_ESCAPE || evento->key.keysym.sym == SDLK_RETURN) {

            ventana_cerrar(juego->ventanaRanking);
            juego->rankingCant = 0;
            juego->logica.estado = LOGICA_EN_ESPERA;
        }
    }
}

static void _juego_ventana_ranking_dibujar(void *datos)
{
    tJuego *juego = (tJuego*)datos;
    SDL_Point nuevaPos = {(VENTANA_RANKING_ANCHO / 2) - 4, 36};
    char buffer[128];

    tWidget* widget = juego->hud.widgets[HUD_WIDGET_LINEA];
    widget_modificar_visibilidad(widget, (juego->rankingCant > 0));

    sprintf(buffer, "%6s %-16s %7s pts", "Puesto", "Username", "Premios");
    widget_modificar_valor(widget, buffer);
    widget_modificar_posicion(widget, nuevaPos);
    widget_dibujar(widget);

    for (int i = 0; i < TOP_CANT; i++) {

        SDL_Point nuevaPos = {(VENTANA_RANKING_ANCHO / 2) - 4, 102 + (i * 66)};

        if (i < juego->rankingCant) {

            sprintf(buffer, "%-6d %-16s %7d pts", i + 1, juego->ranking[juego->rankingCant - 1 - i].username, juego->ranking[juego->rankingCant - 1 - i].record);
        } else {
            sprintf(buffer, "%-6d %-16s %7d pts", i + 1, "...", 0);
        }

        widget_modificar_valor(widget, buffer);
        widget_modificar_posicion(widget, nuevaPos);
        widget_dibujar(widget);
    }
    widget_modificar_visibilidad(widget, WIDGET_OCULTO);
}
