#include "../../include/cliente/juego.h"
#include "../../include/cliente/assets.h"
#include "../../include/comun/comun.h"
#include "../../include/cliente/dibujado.h"
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
#define VENTANA_USERNAME_ANCHO 340
#define VENTANA_USERNAME_ALTO 128
#define VENTANA_RANKING_ANCHO 340
#define VENTANA_RANKING_ALTO 128

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
    tJugador *jugador;
    tCola *colaContextos;
    tCola *colaSolicitudes;
    eLogicaEstado *estadoLogica;
} tDatosUsuario;

typedef struct {
    SDL_Renderer *renderer;
    char *username;
} tDatosRanking;

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

static int _juego_ventana_menu_crear(void *datos);
static void _juego_ventana_menu_actualizar(SDL_Event *evento, void *datos);
static void _juego_ventana_menu_dibujar(void *datos);
static void _juego_ventana_menu_destruir(void *datos);

static int _juego_ventana_usuario_crear(void *datos);
static void _juego_ventana_usuario_actualizar(SDL_Event *evento, void *datos);
static void _juego_ventana_usuario_dibujar(void *datos);
static void _juego_ventana_usuario_destruir(void *datos);

static int _juego_ventana_ranking_crear(void *datos);
static void _juego_ventana_ranking_actualizar(SDL_Event *evento, void *datos);
static void _juego_ventana_ranking_dibujar(void *datos);
static void _juego_ventana_ranking_destruir(void *datos);

int _juego_procesar_contingencia(tJuego *juego);
static void _juego_procesar_cola_respuestas(tJuego *juego);
static char* _juego_procesar_elem_cola_respuestas(tCola *colaRespuestas, tCola *colaContextos, int *codigoRet, int *cantReg, int *tamReg, eColaContexto *contexto);


int juego_inicializar(tJuego *juego, const char *tituloVentana, SOCKET sock, eEstadoSesion sesion)
{
    int ret = ERR_TODO_OK;

    memset(juego, 0, sizeof(tJuego));
    juego->estado = JUEGO_NO_INICIADO;
    printf("Iniciando SDL\n");
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


/// OPCION 1 - QUE EL SERVIDOR ACEPTE UNA ETIQUETA MAX LUEGO DE UNA VARIABLE EN ACTUALIZAR

/// PREFERIDA
/// OPCION 2 - AL ENCONTRAR UN ACTUALIZAR EN _juego_procesar_contingencia SOLICITAR UN SELECT CON EL NOMBRE DEL JUGADOR
///            Y SOLO ENCOLAR EL COMANDO SI EL VALOR ES MAYOR

/// ACTUALIZAR jugadores (record %d, cantPartidas %d) DONDE (username IGUAL %s)

int _juego_procesar_contingencia(tJuego *juego)
{
    char solicitud[TAM_BUFFER], solicitudAux[TAM_BUFFER];
    tJugador jugadorAux = {0};

    fseek(juego->archContingencia, 0, SEEK_SET);

    while (juego->sesion && fgets(solicitud, TAM_BUFFER, juego->archContingencia)) {

        if (strstr(solicitud, "ACTUALIZAR jugadores")) {
            char username[TAM_USUARIO], campos[256], *cursor;
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

                char *datos = NULL;
                int codigoRet, cantReg, tamReg;
                eColaContexto contexto;

                sprintf(solicitudAux, "SELECCIONAR jugadores (username IGUAL %s)", username);
                if (cliente_enviar_solicitud(juego->sock, solicitudAux) == CE_ERR_SOCKET || cliente_recibir_respuesta(juego->sock, &juego->colaRespuestas) == CE_ERR_SOCKET) {
                    juego->sesion = 0;
                    mensaje_advertencia("Socket dessesion, modo offline activo");
                }

                datos = _juego_procesar_elem_cola_respuestas(&juego->colaRespuestas,&juego->colaContextos, &codigoRet, &cantReg, &tamReg, &contexto);
                if (datos) {
                    memcpy(&jugadorAux, datos, sizeof(tJugador));
                    free(datos);
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
    }

    /// REVISAR CONDICIONES
    if (juego->sesion == SESION_OFFLINE && _juego_procesar_cola_solicitudes(juego->sock, juego->archContingencia, &juego->sesion, &juego->colaSolicitudes, SOLICITUD_PROCESAR_TODAS) == ERR_TODO_OK) {

        fclose(juego->archContingencia);
        juego->archContingencia = fopen("contingencia.txt", "w");
        if (!juego->archContingencia) {
            return ERR_ARCHIVO;
        }
    }

    return ERR_TODO_OK;
}

int juego_ejecutar(tJuego *juego)
{
    eRetorno ret = ERR_TODO_OK;
    if (juego->sesion == SESION_INICIADA) {
        _juego_inicializar_base_datos(juego);
        _juego_procesar_contingencia(juego);
    }

    while (juego->estado == JUEGO_CORRIENDO) {

        _juego_manejar_eventos(juego);

        if (juego->logica.estado == LOGICA_JUGANDO) {

            logica_actualizar(&juego->logica);
            _juego_actualizar_hud(&juego->hud, &juego->logica);
        } else {

            if (cola_vacia(&juego->colaSolicitudes) != COLA_VACIA) {

                if (_juego_procesar_cola_solicitudes(juego->sock, juego->archContingencia, &juego->sesion, &juego->colaSolicitudes, SOLICITUD_PROCESAR_UNA) == ERR_TODO_OK) {

                    cliente_recibir_respuesta(juego->sock, &juego->colaRespuestas); // revisar retorno

                    if (juego->sesion == SESION_INICIADA) {
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

        if (*sesion == SESION_INICIADA) {
            retorno = cliente_enviar_solicitud(sock, solicitud);
            if (retorno == CE_ERR_SOCKET) {
                *sesion = SESION_OFFLINE;
                mensaje_advertencia("Socket dessesion, modo offline activo");
            }
        }

        if (*sesion == SESION_INICIADA) {
            if (strncmp(solicitud, "SELECCIONAR", strlen("SELECCIONAR")) != 0) {
                fprintf(archContingencia, "%s\n", solicitud);
                mensaje_info("Guardado para futura conexion: ");
                printf("%s\n", solicitud);
            }
        }
        ++procesadas;
    }

    return *sesion == SESION_INICIADA ? ERR_TODO_OK : ERR_OFFLINE;
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

                    sprintf(buffer, "SELECCIONAR jugadores (username IGUAL %s)", juego->jugador.username);
                    _juego_encolar_solicitud(&juego->colaSolicitudes, &juego->colaContextos, buffer, CONTEXTO_DATOS_JUGADOR);

                    ventana_cerrar(juego->ventanaUsername);
                    widget_modificar_valor(juego->hud.widgets[HUD_WIDGETS_USERNAME], juego->jugador.username);
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
        char buffer[TAM_BUFFER];
        int cantMovs = 0;

        juego->logica.estado = LOGICA_EN_ESPERA;
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

static void _juego_renderizar(SDL_Renderer *renderer, SDL_Texture **imagenes, tLogica *logica, tVentana *ventanaMenu, tVentana *ventanaUsername, tVentana *ventanaRanking, tHud *hud)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (logica->estado == LOGICA_JUGANDO) {
        dibujado_escenario(renderer, &logica->escenario, imagenes);
        _juego_dibujar_hud(hud, renderer);
    } else {
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

static void _juego_mostrar_ranking(void *datos)
{
    //tLogica *logica = (tLogica*)datos;
    puts("RANKING\n");
    //logica->estado = LOGICA_EN_RANKING;
}

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
    if (menu_agregar_opcion(datosMenuPausa->menu, M_PRI_CAMBIAR_USUARIO, texturaAux, 64, (tMenuAccion){NULL, NULL}, OPCION_DESHABILITADA) != ERR_TODO_OK) {
        SDL_DestroyTexture(texturaAux);
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }

    // RANKING
    texturaAux = texto_crear_textura(datosMenuPausa->renderer, datosMenuPausa->fuentes[FUENTE_TAM_64], "RANKING", SDL_COLOR_BLANCO);
    if (!texturaAux) {
        menu_destruir(datosMenuPausa->menu);
        return -1;
    }
    if (menu_agregar_opcion(datosMenuPausa->menu, M_PRI_RANKING, texturaAux, 64, (tMenuAccion){_juego_mostrar_ranking, NULL}, OPCION_HABILITADA) != ERR_TODO_OK) {
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
    tDatosRanking *datosRanking;

    SDL_GetRendererOutputSize(juego->renderer, &rendererW, &rendererH);

    // Widgets HUD
    juego->hud.widgets[HUD_WIDGET_VIDAS] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_VIDAS_Y}, juego->imagenes[IMAGEN_ICO_VIDAS], juego->fuentes[FUENTE_TAM_48], SDL_COLOR_BLANCO, 0, WIDGET_VISIBLE);
    if (!juego->hud.widgets[HUD_WIDGET_VIDAS]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_PREMIOS] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_PREMIOS_Y}, juego->imagenes[IMAGEN_ICO_PREMIOS], juego->fuentes[FUENTE_TAM_48], SDL_COLOR_BLANCO, 0, WIDGET_VISIBLE);
    if (!juego->hud.widgets[HUD_WIDGETS_PREMIOS]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_RONDA] = widget_crear_contador(juego->renderer, (SDL_Point){HUD_X, HUD_RONDA_Y}, NULL, juego->fuentes[FUENTE_TAM_64], SDL_COLOR_BLANCO, 0, WIDGET_VISIBLE);
    if (!juego->hud.widgets[HUD_WIDGETS_RONDA]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_USERNAME] = widget_crear_texto(juego->renderer, NULL, (SDL_Point){rendererW / 2, rendererH - 32}, juego->fuentes[FUENTE_TAM_32], SDL_COLOR_BLANCO, (SDL_Color){0,0,0,0}, WIDGET_VISIBLE);
    if (!juego->hud.widgets[HUD_WIDGETS_USERNAME]) return ERR_SIN_MEMORIA;
    juego->hud.widgets[HUD_WIDGETS_TEXTO] = widget_crear_texto(juego->renderer, "SIGUIENTE RONDA!", (SDL_Point){rendererW / 2, 24}, juego->fuentes[FUENTE_TAM_32], SDL_COLOR_BLANCO, (SDL_Color){0,0,0,0}, WIDGET_OCULTO);
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
    *juego->jugador.username = '\0';
    datosUsername->renderer = juego->renderer;
    datosUsername->fuentes = juego->fuentes;
    datosUsername->campoTexto = NULL;
    datosUsername->jugador = &juego->jugador;
    datosUsername->colaContextos = &juego->colaContextos;
    datosUsername->colaSolicitudes = &juego->colaSolicitudes;
    datosUsername->estadoLogica = &juego->logica.estado;
    juego->ventanaUsername = ventana_crear(juego->renderer, (tVentanaAccion){_juego_ventana_usuario_crear, _juego_ventana_usuario_actualizar, _juego_ventana_usuario_dibujar, _juego_ventana_usuario_destruir, datosUsername}, dimsVentana, (SDL_Color){162, 200, 200, 255}, 1);
    ventana_abrir(juego->ventanaUsername);


    dimsVentana.w = VENTANA_RANKING_ANCHO;
    dimsVentana.h = VENTANA_RANKING_ALTO;
    dimsVentana.x = (rendererW - dimsVentana.w) / 2;
    dimsVentana.y = (rendererH - dimsVentana.h) / 2;

    datosRanking = malloc(sizeof(tDatosRanking));
    if (!datosRanking) return ERR_SIN_MEMORIA;
    datosRanking->renderer = juego->renderer;
    datosRanking->username = juego->jugador.username;
    juego->ventanaRanking = ventana_crear(juego->renderer, (tVentanaAccion){_juego_ventana_ranking_crear, _juego_ventana_ranking_actualizar, _juego_ventana_ranking_dibujar, _juego_ventana_ranking_destruir, datosRanking}, dimsVentana, (SDL_Color){162, 200, 200, 255}, 1);

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
    SDL_StartTextInput();
    return 0;
}


static void _juego_ventana_usuario_actualizar(SDL_Event *evento, void *datos)
{
    tDatosUsuario *datosUsuario = (tDatosUsuario*)datos;
    char buffer[TAM_BUFFER];

    if(evento->type == SDL_TEXTINPUT){

        char letra[2] = {*evento->text.text, '\0'};

        *letra &= ~0x20;

        strncat(datosUsuario->jugador->username, letra, TAM_USUARIO - strlen(datosUsuario->jugador->username) - 1);
        widget_modificar_valor(datosUsuario->campoTexto, datosUsuario->jugador->username);

    }else if(evento->type == SDL_KEYDOWN && evento->key.keysym.sym == SDLK_BACKSPACE){

        size_t longitud = strlen(datosUsuario->jugador->username);

        if (longitud > 0) {
            datosUsuario->jugador->username[longitud - 1] = '\0';
        }
        widget_modificar_valor(datosUsuario->campoTexto, datosUsuario->jugador->username);
    } else if (*datosUsuario->jugador->username && evento->type == SDL_KEYDOWN && evento->key.keysym.sym == SDLK_RETURN) {

        sprintf(buffer, "INSERTAR jugadores (username %s)", datosUsuario->jugador->username);
        _juego_encolar_solicitud(datosUsuario->colaSolicitudes, datosUsuario->colaContextos, buffer, CONTEXTO_INSERTAR_JUGADOR);

        *datosUsuario->estadoLogica = LOGICA_EN_ESPERA;
    }
}


static char* _juego_procesar_elem_cola_respuestas(tCola *colaRespuestas, tCola *colaContextos, int *codigoRet, int *cantReg, int *tamReg, eColaContexto *contexto)
{
    char mensaje[TAM_BUFFER], respuesta[TAM_BUFFER], *cursor, *datos = NULL;

    if (cola_desencolar(colaRespuestas, &respuesta, TAM_BUFFER) == COLA_VACIA
        || cola_desencolar(colaContextos, contexto, sizeof(eColaContexto)) == COLA_VACIA) {
        return NULL;
    }

    cursor = strchr(respuesta, '\n');
    if (!cursor) {
        return NULL;
    }
    *cursor = '\0';

    cursor = strrchr(respuesta, ';');
    sscanf(cursor + 1, "%d", tamReg);
    *cursor = '\0';

    cursor = strrchr(respuesta, ';');
    sscanf(cursor + 1, "%d", cantReg);
    *cursor = '\0';

    cursor = strrchr(respuesta, ';');
    strncpy(mensaje, cursor + 1, TAM_BUFFER);
    mensaje[TAM_BUFFER - 1] = '\0';

    *cursor = '\0';
    sscanf(respuesta, "%d", codigoRet);

    mensaje_color(*codigoRet > BD_ERROR_SIN_RESULTADOS ? TEXTO_ROJO : TEXTO_VERDE, "Respuesta: retorno = %d,  mensaje = %s", *codigoRet, mensaje);

    if (*codigoRet == BD_DATOS_OBTENIDOS && *cantReg > 0 && *tamReg > 0) {

        cola_desencolar(colaRespuestas, &datos, sizeof(void*));
    }

    return datos;
}


static void _juego_procesar_cola_respuestas(tJuego *juego)
{
    char *datos = NULL;
    int codigoRet, cantReg, tamReg;
    eColaContexto contexto;

    datos = _juego_procesar_elem_cola_respuestas(&juego->colaRespuestas, &juego->colaContextos, &codigoRet, &cantReg, &tamReg, &contexto);

    switch (contexto) {

        case CONTEXTO_INSERTAR_JUGADOR: {
            break;
        }
        case CONTEXTO_DATOS_RANKING: {
            if (codigoRet == BD_DATOS_OBTENIDOS) {
                free(datos);
            }
            break;
        }
        default:
            break;
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
    SDL_StopTextInput();
    widget_destruir(datosUsuario->campoTexto);
    free(datosUsuario);
}


static int _juego_ventana_ranking_crear(void *datos)
{
    //tDatosRanking *datosRanking = (tDatosRanking*)datos;

    return 0;
}

static void _juego_ventana_ranking_actualizar(SDL_Event *evento, void *datos)
{
    tDatosRanking *datosRanking = (tDatosRanking*)datos;

    /// REVISAR SI LA SESION ES OFFLINE

    if (*datosRanking->username && evento->type == SDL_KEYDOWN && evento->key.keysym.sym == SDLK_RETURN) {

        ///
    }
}

static void _juego_ventana_ranking_dibujar(void *datos)
{
    //tDatosRanking *datosRanking = (tDatosRanking*)datos;

    ///
}

static void _juego_ventana_ranking_destruir(void *datos)
{
    tDatosRanking *datosRanking = (tDatosRanking*)datos;
    if (!datosRanking) {
       return;
    }

    free(datosRanking);
}
