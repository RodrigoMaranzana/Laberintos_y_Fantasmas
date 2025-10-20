#include "../../include/juego/logica.h"
#include "../../include/juego/archivo.h"
#include "../../include/comun/matriz.h"
#include "../../include/juego/temporizador.h"
#include "../../include/comun/pila.h"
#include "../../include/comun/mensaje.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MIN_COLUMNAS 7
#define MIN_FILAS 7
#define DEF_COLUMNAS 11
#define DEF_FILAS 11
#define DEF_VIDAS_INICIO 3
#define DEF_MAX_FANTASMAS 3
#define DEF_MAX_PREMIOS 3
#define DEF_MAX_VIDAS_EXTRA 3

static int logica_ubicacion_valida(const tEscenario *escenario, tUbicacion ubic);
static tUbicacion _logica_mover_fantasma_bfs(tEscenario *escenario, tEntidad *jugador, tEntidad *fantasma);
static tUbicacion _logica_mover_fantasma_dfs(tEscenario *escenario, tEntidad *jugador, tEntidad *fantasma);
static void _logica_colocar_fantasmas(tLogica *logica);
static void _logica_inicializar_jugador(tEntidad *jugador);
static void _logica_colocar_jugador(const tEscenario *escenario, tEntidad *jugador);
static int _logica_fantasma_debe_usar_bfs(const tEntidad *fantasma);
static void _logica_actualizar_entidad(tEntidad *entidad);
static int _logica_encontrar_casilla_libre(tLogica *logica, tUbicacion *ubic);

void logica_calc_min_res(const tLogica *logica, unsigned *anchoRes, unsigned *altoRes)
{
    *anchoRes = logica->config.columnas * PIXELES_TILE;
    *altoRes = logica->config.filas * PIXELES_TILE;
}

int logica_inicializar(tLogica *logica)
{
    tConf confArch;
    FILE *archConf;
    int reescribir = 1;

    archConf = fopen("config.txt", "r+");
    if (archConf) {

        if (archivo_leer_conf(archConf, &confArch) == ERR_TODO_OK && confArch.columnas >= MIN_COLUMNAS && confArch.filas >= MIN_FILAS && confArch.max_num_fantasmas >= 1) {
            reescribir = 0;
        }

        fclose(archConf);
    }

    if (reescribir) {
        mensaje_info("Creando archivo de configuracion por defecto.");

        archConf = fopen("config.txt", "w");
        if (!archConf)return ERR_ARCHIVO;

        confArch.columnas = DEF_COLUMNAS;
        confArch.filas = DEF_FILAS;
        confArch.vidas_inicio = DEF_VIDAS_INICIO;
        confArch.max_num_fantasmas = DEF_MAX_FANTASMAS;
        confArch.max_num_premios = DEF_MAX_PREMIOS;
        confArch.max_vidas_extra = DEF_MAX_VIDAS_EXTRA;

        archivo_escribir_conf(archConf, &confArch);
        fclose(archConf);
    }

    logica->config.filas = confArch.filas;
    logica->config.columnas = confArch.columnas;
    logica->config.maxFantasmas = confArch.max_num_fantasmas;
    logica->config.maxVidasExtra = confArch.max_vidas_extra;
    logica->config.maxPremios = confArch.max_num_premios;
    logica->config.vidasInicio = confArch.vidas_inicio;

    logica->fantasmas = (tEntidad*)malloc(sizeof(tEntidad) * logica->config.maxFantasmas);
    if (!logica->fantasmas) return ERR_SIN_MEMORIA;

    _logica_inicializar_jugador(&logica->jugador);

    cola_crear(&logica->movimientos);
    cola_crear(&logica->movsJugador);

    escenario_crear(&logica->escenario, logica->config.columnas, logica->config.filas);

    temporizador_inicializar(&logica->temporCambioRonda, 3.0);

    logica->estado = LOGICA_EN_LOGIN;

    return ERR_TODO_OK;
}

void logica_destruir(tLogica *logica)
{
    escenario_destruir(&logica->escenario);

    if (logica->fantasmas) {
        free(logica->fantasmas);
        logica->fantasmas = NULL;
    }

    cola_vaciar(&logica->movimientos);
    cola_vaciar(&logica->movsJugador);
}

int logica_procesar_turno(tLogica *logica, SDL_Keycode tecla)
{
    tEntidad *fantasma, *fantasmaUlt = logica->fantasmas + (logica->ronda.cantFantasmas - 1);
    tUbicacion nuevaUbicJugador;
    tMovimiento mov;

    if (logica->jugador.estado == ENTIDAD_SIN_VIDA || logica->jugador.estado == ENTIDAD_ATURDIDA) return 0;

    nuevaUbicJugador = logica->jugador.ubic;

    switch (tecla) {

        case SDLK_UP:
            --nuevaUbicJugador.fila;
            logica->jugador.orientacion = MIRANDO_ARRIBA;
            break;
        case SDLK_DOWN:
            ++nuevaUbicJugador.fila;
            logica->jugador.orientacion = MIRANDO_ABAJO;
            break;
        case SDLK_LEFT:
            --nuevaUbicJugador.columna;
            logica->jugador.orientacion = MIRANDO_IZQUIERDA;
            break;
        case SDLK_RIGHT:
            ++nuevaUbicJugador.columna;
            logica->jugador.orientacion = MIRANDO_DERECHA;
            break;
        default:
            return 0;
    }

    if (logica_ubicacion_valida(&logica->escenario, nuevaUbicJugador)) {

        tUbicacion nuevaUbicFantasma;

        mov.ubic = nuevaUbicJugador;
        mov.direccion = logica->jugador.orientacion;
        mov.entidad = &logica->jugador;

        cola_encolar(&logica->movimientos, &mov, sizeof(tMovimiento));

        for (fantasma = logica->fantasmas; fantasma <= fantasmaUlt; fantasma++) {

            if (fantasma->estado == ENTIDAD_CON_VIDA) {

                nuevaUbicFantasma.columna = -1;
                nuevaUbicFantasma.fila = -1;

                if (_logica_fantasma_debe_usar_bfs(fantasma)) {
                    nuevaUbicFantasma = _logica_mover_fantasma_bfs(&logica->escenario, &logica->jugador, fantasma);
                } else {
                    nuevaUbicFantasma = _logica_mover_fantasma_dfs(&logica->escenario, &logica->jugador, fantasma);
                }

                if (nuevaUbicFantasma.fila != -1) {

                    if (nuevaUbicFantasma.columna > fantasma->ubic.columna) {

                        mov.direccion = MIRANDO_DERECHA;

                    } else if (nuevaUbicFantasma.columna < fantasma->ubic.columna) {

                        mov.direccion = MIRANDO_IZQUIERDA;

                    } else if (nuevaUbicFantasma.fila > fantasma->ubic.fila) {

                        mov.direccion = MIRANDO_ABAJO;

                    } else if (nuevaUbicFantasma.fila < fantasma->ubic.fila) {

                        mov.direccion = MIRANDO_ARRIBA;
                    }

                    mov.ubic = nuevaUbicFantasma;
                    mov.entidad = fantasma;

                    cola_encolar(&logica->movimientos, &mov, sizeof(tMovimiento));
                }
            }
        }

        return 1;

    } else {

        int jugadorEnPuerta = (logica->escenario.tablero[logica->jugador.ubic.fila][logica->jugador.ubic.columna].tile->tileTipo == TILE_TIPO_PUERTA_SALIDA);
        int saleDelMapa = (nuevaUbicJugador.columna < 0 || nuevaUbicJugador.columna >= logica->config.columnas || nuevaUbicJugador.fila < 0 || nuevaUbicJugador.fila >= logica->config.filas);

        if (jugadorEnPuerta && saleDelMapa) {

            logica_nueva_ronda(logica);
        }
    }

    return 0;
}

void logica_actualizar(tLogica *logica)
{
    tEntidad *fantasma, *fantasmaUlt;

    _logica_actualizar_entidad(&logica->jugador);

    fantasmaUlt = logica->fantasmas + (logica->ronda.cantFantasmas - 1);
    for (fantasma = logica->fantasmas; fantasma <= fantasmaUlt; fantasma++) {

        if (fantasma->estado != ENTIDAD_SIN_VIDA) {

           _logica_actualizar_entidad(fantasma);
        }
    }
}

void logica_procesar_movimientos(tLogica *logica)
{
    tMovimiento mov;
    tUbicacion ubicNueva;

    while (cola_desencolar(&logica->movimientos, &mov, sizeof(tMovimiento)) == COLA_TODO_OK) {

        int puedeMoverse = 1;
        tEntidad *entidadEnMov, *entidadEnCasilla;

        if (mov.entidad->tipo == ENTIDAD_JUGADOR) {

            cola_encolar(&logica->movsJugador, &mov, sizeof(tMovimiento));
        }

        ubicNueva = mov.ubic;
        entidadEnMov = mov.entidad;
        entidadEnCasilla = logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].entidad;

        if (entidadEnMov->estado == ENTIDAD_ATURDIDA || entidadEnMov->estado == ENTIDAD_SIN_VIDA) {

            puedeMoverse = 0;

        } else if (entidadEnCasilla != NULL) {

            if (entidadEnCasilla->tipo == ENTIDAD_JUGADOR || entidadEnMov->tipo == ENTIDAD_JUGADOR) {

                tEntidad *jugador = (entidadEnMov->tipo == ENTIDAD_JUGADOR) ? entidadEnMov : entidadEnCasilla;
                tEntidad *fantasma = (entidadEnMov->tipo == ENTIDAD_JUGADOR) ? entidadEnCasilla : entidadEnMov;

                fantasma->estado = ENTIDAD_SIN_VIDA;
                if (logica->ronda.cantVidasActual > 0) {

                    logica->jugador.estado = ENTIDAD_ATURDIDA;
                    --logica->ronda.cantVidasActual;
                    temporizador_iniciar(&logica->jugador.temporEstado);
                } else {

                    logica->jugador.estado = ENTIDAD_SIN_VIDA;
                    logica_fin_juego(logica);
                }

                logica->escenario.tablero[jugador->ubic.fila][jugador->ubic.columna].entidad = NULL;
                logica->escenario.tablero[fantasma->ubic.fila][fantasma->ubic.columna].entidad = NULL;
                _logica_colocar_jugador(&logica->escenario, &logica->jugador);

                puedeMoverse = 0;

            } else {

                puedeMoverse = 0;
            }
        }

        if (puedeMoverse) {

            logica->escenario.tablero[entidadEnMov->ubic.fila][entidadEnMov->ubic.columna].entidad = NULL;
            logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].entidad = entidadEnMov;

            entidadEnMov->ubicAnterior = entidadEnMov->ubic;
            entidadEnMov->ubic = ubicNueva;
            entidadEnMov->orientacion = mov.direccion;

            if (entidadEnMov->tipo == ENTIDAD_JUGADOR) {

                switch (logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].extra) {
                    case EXTRA_PREMIO: {

                        tEntidad *fantasma, *fantasmaUlt = logica->fantasmas + (logica->ronda.cantFantasmas - 1);

                        ++logica->ronda.cantPremios;
                        logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].extra = EXTRA_NINGUNO;

                        for (fantasma = logica->fantasmas; fantasma <= fantasmaUlt; ++fantasma) {

                            if (fantasma->estado != ENTIDAD_SIN_VIDA) {

                                fantasma->estado = ENTIDAD_ATURDIDA;
                                temporizador_iniciar(&fantasma->temporEstado);
                            }
                        }
                        break;
                    }
                    case EXTRA_VIDA: {
                        ++logica->ronda.cantVidasActual;
                        logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].extra = EXTRA_NINGUNO;
                        logica->jugador.estado = ENTIDAD_POTENCIADA;
                        temporizador_iniciar(&logica->jugador.temporEstado);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
}

int logica_mostrar_historial_movs(tCola *movsJugador)
{
    int movNro = 0;
    tMovimiento mov;
    const char *direccion[] = {
        "ARRIBA",
        "ABAJO",
        "IZQUIERDA",
        "DERECHA",
    };

    mensaje_info("Tus movimientos realizados:");

    while (cola_desencolar(movsJugador, &mov, sizeof(tMovimiento)) != COLA_VACIA) {

        printf("Movimiento numero %-4u | Fila: %3d | Columna: %3d | Direccion: %s\n", ++movNro, mov.ubic.fila, mov.ubic.columna, direccion[(int)mov.direccion]);
    }

    return movNro;
}

int logica_iniciar_juego(tLogica *logica)
{
    logica->semillaMaestra = time(NULL);
    mensaje_color(TEXTO_MAGENTA, "[DEBUG] La semilla maestra de esta partida es: %ld", logica->semillaMaestra);

    logica->ronda.numRonda = 1;
    logica->ronda.cantPremios = 0;
    logica->ronda.cantVidasActual = logica->config.vidasInicio;

    logica->jugador.estado = ENTIDAD_CON_VIDA;

    logica->ronda.semillaRonda = logica->semillaMaestra + logica->ronda.numRonda;
    srand(logica->ronda.semillaRonda);

    logica->ronda.cantFantasmas = 1 + (rand() % logica->config.maxFantasmas);
    logica->ronda.cantPremiosExtra = 1 + (rand() % logica->config.maxPremios);
    logica->ronda.cantVidasExtra = (rand() % logica->config.maxVidasExtra + 1);
    escenario_generar(&logica->escenario, logica->ronda.cantPremiosExtra, logica->ronda.cantVidasExtra);

    _logica_colocar_jugador(&logica->escenario, &logica->jugador);
    _logica_colocar_fantasmas(logica);

    archivo_escribir_escenario(&logica->escenario, logica->ronda.numRonda, logica->ronda.semillaRonda);

    logica->estado = LOGICA_JUGANDO;

    return ERR_TODO_OK;
}

int logica_nueva_ronda(tLogica *logica)
{
    mensaje_info("Siguiente ronda");

    cola_vaciar(&logica->movimientos);

    logica->ronda.numRonda++;
    logica->ronda.semillaRonda = logica->semillaMaestra + logica->ronda.numRonda;
    srand(logica->ronda.semillaRonda);

    logica->ronda.cantFantasmas = 1 + (rand()  % logica->config.maxFantasmas);
    logica->ronda.cantPremiosExtra = 1 + (rand() % logica->config.maxPremios);
    logica->ronda.cantVidasExtra = (rand() % logica->config.maxVidasExtra + 1);
    escenario_generar(&logica->escenario, logica->ronda.cantPremiosExtra, logica->ronda.cantVidasExtra);

    _logica_colocar_jugador(&logica->escenario, &logica->jugador);
    _logica_colocar_fantasmas(logica);

    archivo_escribir_escenario(&logica->escenario, logica->ronda.numRonda, logica->ronda.semillaRonda);
    temporizador_iniciar(&logica->temporCambioRonda);
    logica->mostrarSigRonda = 1;

    temporizador_iniciar(&logica->temporCambioRonda);
    logica->mostrarSigRonda = 1;

    return ERR_TODO_OK;
}

void logica_fin_juego(tLogica *logica)
{
    mensaje_info("FIN DEL JUEGO");
    mensaje_color(TEXTO_CIAN, "Ronda maxima alcanzada: %d "
           "Puntaje obtenido: %d\n"
           ,logica->ronda.numRonda
           ,logica->ronda.cantPremios
           );
    logica->estado = LOGICA_FIN_PARTIDA;
}


/*************************
    FUNCIONES ESTATICAS
*************************/

static void _logica_colocar_jugador(const tEscenario *escenario, tEntidad *jugador)
{
    int pared = escenario_ubic_es_pared_limite(escenario, escenario->ubicPEntrada);

    jugador->ubic = escenario->ubicPEntrada;

    switch (pared) {
        case PARED_SUPERIOR:
            jugador->orientacion = MIRANDO_ABAJO;
            ++jugador->ubic.fila;
            break;
        case PARED_INFERIOR:
            jugador->orientacion = MIRANDO_ARRIBA;
            --jugador->ubic.fila;
            break;
        case PARED_IZQUIERDA:
            jugador->orientacion = MIRANDO_DERECHA;
            ++jugador->ubic.columna;
            break;
        case PARED_DERECHA:
            jugador->orientacion = MIRANDO_IZQUIERDA;
            --jugador->ubic.columna;
            break;
        default:
            return;
    }

    escenario->tablero[jugador->ubic.fila][jugador->ubic.columna].entidad = jugador;
}

static void _logica_inicializar_jugador(tEntidad *jugador)
{
    jugador->tipo = ENTIDAD_JUGADOR;
    jugador->imagen = IMAGEN_JUGADOR;
    jugador->frame = 0;
    temporizador_inicializar(&jugador->temporEstado, 1.5f);
    temporizador_inicializar(&jugador->temporFrame, 0.25f);
    temporizador_iniciar(&jugador->temporFrame);
}

static int _logica_encontrar_casilla_libre(tLogica *logica, tUbicacion *ubicEncontrada)
{
    int columna, fila, intento = 0;

    do {
        columna = rand() % logica->config.columnas;
        fila = rand() % logica->config.filas;
        ++intento;

        if (intento > logica->config.filas * logica->config.columnas) {

            return -1;
        }

    } while (logica->escenario.tablero[fila][columna].tile->tileTipo != TILE_TIPO_PISO || logica->escenario.tablero[fila][columna].entidad != NULL);

    ubicEncontrada->fila = fila;
    ubicEncontrada->columna = columna;

    return ERR_TODO_OK;
}

static void _logica_colocar_fantasmas(tLogica *logica)
{
    tUbicacion ubic;
    tEntidad *fantasma, *fantasmaUlt = logica->fantasmas + (logica->ronda.cantFantasmas - 1);

    for (fantasma = logica->fantasmas; fantasma <= fantasmaUlt; fantasma++) {

        int iFantasma = (fantasmaUlt - fantasma) % 4;

        if (_logica_encontrar_casilla_libre(logica, &ubic) == ERR_TODO_OK) {

            fantasma->orientacion = rand() % 4;
            fantasma->frame = rand() % 4;
            fantasma->ubic = ubic;
            fantasma->ubicAnterior = fantasma->ubic;
            fantasma->imagen = IMAGEN_FANTASMA_01 + iFantasma;
            fantasma->tipo = ENTIDAD_FANTASMA_AMARILLO + iFantasma;
            fantasma->estado = ENTIDAD_CON_VIDA;

            temporizador_inicializar(&fantasma->temporEstado, 4.0f);
            temporizador_inicializar(&fantasma->temporFrame, 0.25f + ((float)(rand() % 101)) / 1000.0f);
            temporizador_iniciar(&fantasma->temporFrame);

            logica->escenario.tablero[ubic.fila][ubic.columna].entidad = fantasma;

        } else {

            mensaje_advertencia("No hay espacio para colocar otro fantasma.");
            fantasma->estado = ENTIDAD_SIN_VIDA;
        }
    }
}


static int logica_ubicacion_valida(const tEscenario *escenario, tUbicacion ubic)
{
    int dentroLimites = (ubic.columna >= 0 && ubic.columna < escenario->cantColumnas && ubic.fila >= 0 && ubic.fila < escenario->cantFilas);

    return dentroLimites && escenario->tablero[ubic.fila][ubic.columna].transitable;
}

static tUbicacion _logica_mover_fantasma_dfs(tEscenario *escenario, tEntidad *jugador, tEntidad *fantasma)
{
    tPila pila;
    tUbicacion ubicProcesada, ubicVecina, primerMovimiento = {-1, -1};
    int dirFila[] = {-1, 0, 1, 0}, dirColumna[] = {0, 1, 0, -1}, jugadorEncontrado = 0, columna, fila;

    for(fila = 0; fila < escenario->cantFilas; fila++){

        for(columna = 0; columna < escenario->cantColumnas; columna++){

            escenario->tablero[fila][columna].visitada = 0;
        }
    }

    pila_crear(&pila);

    ubicProcesada = fantasma->ubic;
    pila_apilar(&pila, &ubicProcesada, sizeof(tUbicacion));

    escenario->tablero[ubicProcesada.fila][ubicProcesada.columna].visitada = 1;

    while(pila_vacia(&pila) == ERR_TODO_OK && !jugadorEncontrado) {

        int i, nuevaUbicEncontrada = 0;

        pila_tope(&pila, &ubicProcesada, sizeof(tUbicacion));

        if(escenario->tablero[ubicProcesada.fila][ubicProcesada.columna].entidad == jugador) {

            jugadorEncontrado = 1;
        }

        for(i = 0; i < 4 && !nuevaUbicEncontrada && !jugadorEncontrado; i++) {

            char noVuelveAtras, esUbicacionValida, esNoVisitada, estaLibre;

            ubicVecina.fila = ubicProcesada.fila + dirFila[i];
            ubicVecina.columna = ubicProcesada.columna + dirColumna[i];

            noVuelveAtras = (ubicVecina.fila != fantasma->ubicAnterior.fila || ubicVecina.columna != fantasma->ubicAnterior.columna);
            esUbicacionValida = logica_ubicacion_valida(escenario, ubicVecina);

            if (esUbicacionValida) {

                esNoVisitada = !escenario->tablero[ubicVecina.fila][ubicVecina.columna].visitada;
                estaLibre = escenario->tablero[ubicVecina.fila][ubicVecina.columna].entidad == NULL || escenario->tablero[ubicVecina.fila][ubicVecina.columna].entidad == jugador;
            }

            if(noVuelveAtras && esUbicacionValida && esNoVisitada && estaLibre) {

                pila_apilar(&pila, &ubicVecina, sizeof(tUbicacion));
                escenario->tablero[ubicVecina.fila][ubicVecina.columna].visitada = 1;

                nuevaUbicEncontrada = 1;
            }
        }

        if(!nuevaUbicEncontrada && !jugadorEncontrado) {

            pila_desapilar(&pila, &ubicProcesada, sizeof(tUbicacion));
        }
    }

    if(jugadorEncontrado){

        while(pila_vacia(&pila) == ERR_TODO_OK) {

            pila_desapilar(&pila, &ubicProcesada, sizeof(tUbicacion));

            if(pila_vacia(&pila) == ERR_TODO_OK) {

                primerMovimiento = ubicProcesada;
            }
        }
    }

    pila_vaciar(&pila);

    return primerMovimiento;
}

static tUbicacion _logica_mover_fantasma_bfs(tEscenario *escenario, tEntidad *jugador, tEntidad *fantasma)
{
    tCola cola;
    tUbicacion ubicProcesada, ubicVecina;
    tUbicacion primerMovimiento = {-1,-1};
    int i, dirFila[] = {-1, 0, 1, 0}, dirColumna[] = {0, 1, 0, -1}, jugadorEncontrado = 0, columna, fila;

    for (fila = 0; fila < escenario->cantFilas; fila++) {

        for (columna = 0; columna < escenario->cantColumnas; columna++) {

            escenario->tablero[fila][columna].anteriorBFS.fila = -1;
            escenario->tablero[fila][columna].anteriorBFS.columna = -1;

            escenario->tablero[fila][columna].visitada = 0;
        }
    }

    cola_crear(&cola);

    ubicProcesada = fantasma->ubic;
    escenario->tablero[ubicProcesada.fila][ubicProcesada.columna].anteriorBFS.columna = -2;
    escenario->tablero[ubicProcesada.fila][ubicProcesada.columna].anteriorBFS.fila = -2;
    escenario->tablero[ubicProcesada.fila][ubicProcesada.columna].visitada = 1;

    cola_encolar(&cola, &ubicProcesada, sizeof(tUbicacion));

    while(cola_vacia(&cola) == ERR_TODO_OK && !jugadorEncontrado) {

        cola_desencolar(&cola, &ubicProcesada, sizeof(tUbicacion));

        if (ubicProcesada.fila == jugador->ubic.fila && ubicProcesada.columna == jugador->ubic.columna) {
            jugadorEncontrado = 1;
        }

        for(i = 0; i < 4 && !jugadorEncontrado; i++) {

            ubicVecina.fila = ubicProcesada.fila + dirFila[i];
            ubicVecina.columna = ubicProcesada.columna + dirColumna[i];

            if(logica_ubicacion_valida(escenario, ubicVecina) && !escenario->tablero[ubicVecina.fila][ubicVecina.columna].visitada
               && (escenario->tablero[ubicVecina.fila][ubicVecina.columna].entidad == NULL || escenario->tablero[ubicVecina.fila][ubicVecina.columna].entidad == jugador)){

                escenario->tablero[ubicVecina.fila][ubicVecina.columna].anteriorBFS = ubicProcesada;
                escenario->tablero[ubicVecina.fila][ubicVecina.columna].visitada = 1;
                cola_encolar(&cola, &ubicVecina, sizeof(tUbicacion));
            }
        }
    }

    if (jugadorEncontrado) {

        char raizEncontrada = 0;
        primerMovimiento = jugador->ubic;

        while (!raizEncontrada && (escenario->tablero[primerMovimiento.fila][primerMovimiento.columna].anteriorBFS.fila != fantasma->ubic.fila
               || escenario->tablero[primerMovimiento.fila][primerMovimiento.columna].anteriorBFS.columna != fantasma->ubic.columna)) {

            if (escenario->tablero[primerMovimiento.fila][primerMovimiento.columna].anteriorBFS.fila == -2) {

                raizEncontrada = 1;
            }

            if (!raizEncontrada) {
                primerMovimiento = escenario->tablero[primerMovimiento.fila][primerMovimiento.columna].anteriorBFS;
            }
        }
    }

    cola_vaciar(&cola);

    return primerMovimiento;
}

static int _logica_fantasma_debe_usar_bfs(const tEntidad *fantasma)
{
    if ((fantasma->tipo == ENTIDAD_FANTASMA_ROJO && rand() % 2 == 0)
        || (fantasma->tipo == ENTIDAD_FANTASMA_ROSA && rand() % 4 == 0)
        || (fantasma->tipo == ENTIDAD_FANTASMA_AZUL && rand() % 8 == 0)) {

        return 1;
    }

    return 0;
}

static void _logica_actualizar_entidad(tEntidad *entidad)
{
    if (entidad->estado == ENTIDAD_ATURDIDA || entidad->estado == ENTIDAD_POTENCIADA) {

        entidad->frame = 0;
        temporizador_actualizar(&entidad->temporEstado);
        if (temporizador_estado(&entidad->temporEstado) == TEMPOR_FINALIZADO) {
            entidad->estado = ENTIDAD_CON_VIDA;
        }

    }else if (entidad->estado == ENTIDAD_CON_VIDA) {

        temporizador_actualizar(&entidad->temporFrame);
        if (temporizador_estado(&entidad->temporFrame) == TEMPOR_FINALIZADO) {
            entidad->frame = (entidad->frame + 1) % 4;
            temporizador_iniciar(&entidad->temporFrame);
        }
    }
}
