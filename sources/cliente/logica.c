#include "../../include/cliente/logica.h"
#include "../../include/cliente/archivo.h"
#include "../../include/comun/matriz.h"
#include "../../include/cliente/temporizador.h"
#include "../../include/comun/pila.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int logica_ubicacion_valida(const tEscenario *escenario, tUbicacion ubic);
static tUbicacion _logica_mover_fantasma_bfs(tEscenario *escenario, tEntidad *jugador, tEntidad *fantasma);
static tUbicacion _logica_mover_fantasma_dfs(tEscenario *escenario, tEntidad *jugador, tEntidad *fantasma);
static void _logica_colocar_fantasmas(tLogica *logica);
static void _logica_colocar_jugador(tLogica *logica);

void logica_calc_min_res(tLogica *logica, unsigned *anchoRes, unsigned *altoRes)
{
    *anchoRes = logica->config.columnas * PIXELES_TILE;
    *altoRes = logica->config.filas * PIXELES_TILE;
}

int logica_inicializar(tLogica *logica)
{
    tConf confArch;
    FILE *archConf;
    int reescribir = 0;

    archConf = fopen("config.txt", "r+");
    if (!archConf) {
        reescribir = 1;
    } else {

        if (archivo_leer_conf(archConf, &confArch) == ERR_CONF || confArch.columnas < 8 || confArch.filas < 8 || confArch.max_num_fantasmas < 1) {

            reescribir = 1;
        }
        fclose(archConf);
    }

    if (reescribir) {
        puts("Creando archivo de configuracion por defecto...");

        archConf = fopen("config.txt", "w");
        if (!archConf) {
            return ERR_ARCHIVO;
        }

        confArch.columnas = 11;
        confArch.filas = 11;
        confArch.vidas_inicio = 3;
        confArch.max_num_fantasmas = 4;
        confArch.max_num_premios = 2;
        confArch.max_vidas_extra = 1;

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
    if (!logica->fantasmas) {
        return ERR_SIN_MEMORIA;
    }

    cola_crear(&logica->movimientos);
    cola_crear(&logica->movsJugador);

    escenario_crear(&logica->escenario, logica->config.columnas, logica->config.filas);

    logica->estado = LOGICA_EN_LOGIN;

    return TODO_OK;
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

void logica_procesar_turno(tLogica *logica, SDL_Keycode tecla)
{
    tEntidad *fantasma, *fantasmaUlt = logica->fantasmas + (logica->ronda.cantFantasmas - 1);
    tUbicacion nuevaUbicJugador;
    tMovimiento mov;

    if (logica->jugador.estado != ENTIDAD_CON_VIDA) {
        return;
    }

    nuevaUbicJugador = logica->jugador.ubic;

    switch (tecla) {

        case SDLK_UP:
            nuevaUbicJugador.fila--;
            logica->jugador.orientacion = MIRANDO_ARRIBA;
            break;
        case SDLK_DOWN:
            nuevaUbicJugador.fila++;
            logica->jugador.orientacion = MIRANDO_ABAJO;
            break;
        case SDLK_LEFT:
            nuevaUbicJugador.columna--;
            logica->jugador.orientacion = MIRANDO_IZQUIERDA;
            break;
        case SDLK_RIGHT:
            nuevaUbicJugador.columna++;
            logica->jugador.orientacion = MIRANDO_DERECHA;
            break;
        default:
            return; // Tecla invalida
    }

    if (logica_ubicacion_valida(&logica->escenario, nuevaUbicJugador)) {

        tUbicacion nuevaUbicFantasma;

        mov.ubic = nuevaUbicJugador;
        mov.direccion = logica->jugador.orientacion;
        mov.entidad = &logica->jugador;

        cola_encolar(&logica->movimientos, &mov, sizeof(tMovimiento));
        cola_encolar(&logica->movsJugador, &mov, sizeof(tMovimiento));

        for (fantasma = logica->fantasmas; fantasma <= fantasmaUlt; fantasma++) {

            if (fantasma->estado == ENTIDAD_CON_VIDA) {

                nuevaUbicFantasma.columna = -1;
                nuevaUbicFantasma.fila = -1;

                if ((fantasma->imagen == IMAGEN_FANTASMA_04 && rand() % 2 == 0)
                    || (fantasma->imagen == IMAGEN_FANTASMA_03 && rand() % 4 == 0)
                    || (fantasma->imagen == IMAGEN_FANTASMA_02 && rand() % 8 == 0)) {

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
    }
}

void logica_actualizar(tLogica *logica)
{
    tEntidad *jugador = &logica->jugador;
    tEntidad *fantasma, *fantasmaUlt;

    if (jugador->estado == ENTIDAD_ATURDIDA || jugador->estado == ENTIDAD_POTENCIADA) {
        jugador->frame = 0;
        temporizador_actualizar(&jugador->temporEstado);
        if (jugador->estado != ENTIDAD_SIN_VIDA && temporizador_estado(&jugador->temporEstado) == TEMPOR_FINALIZADO) {
            jugador->estado = ENTIDAD_CON_VIDA;
        }
    }else {

        temporizador_actualizar(&jugador->temporFrame);
        if (temporizador_estado(&jugador->temporFrame) == TEMPOR_FINALIZADO) {
            jugador->frame = (jugador->frame + 1) % 4;
            temporizador_iniciar(&jugador->temporFrame);
        }
    }

    fantasmaUlt = logica->fantasmas + (logica->ronda.cantFantasmas - 1);
    for (fantasma = logica->fantasmas; fantasma <= fantasmaUlt; fantasma++) {

        if (fantasma->estado != ENTIDAD_SIN_VIDA) {
            temporizador_actualizar(&fantasma->temporFrame);
            if (temporizador_estado(&fantasma->temporFrame) == TEMPOR_FINALIZADO) {
                fantasma->frame = (fantasma->frame + 1) % 4;
                temporizador_iniciar(&fantasma->temporFrame);
            }
        }

//        if (fantasma->estado == ENTIDAD_ATURDIDA) {
//            temporizador_actualizar(&fantasma->temporEstado);
//            if (temporizador_estado(&fantasma->temporEstado) == TEMPOR_FINALIZADO) {
//                fantasma->estado = ENTIDAD_CON_VIDA;
//            }
//        }
    }
}

int logica_procesar_movimientos(tLogica *logica)
{
    tMovimiento mov;
    tUbicacion ubicNueva;

    while (cola_vacia(&logica->movimientos) == COLA_TODO_OK) {

        int puedeMoverse = 1;
        tEntidad *entidadEnMov, *entidadEnCasilla;

        cola_desencolar(&logica->movimientos, &mov, sizeof(tMovimiento));

        ubicNueva = mov.ubic;
        entidadEnMov = mov.entidad;
        entidadEnCasilla = logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].entidad;

        if(entidadEnMov->estado == ENTIDAD_ATURDIDA || entidadEnMov->estado == ENTIDAD_SIN_VIDA){

            puedeMoverse = 0;
        }

        if (puedeMoverse && entidadEnCasilla != NULL) {

            if (entidadEnMov->tipo != ENTIDAD_JUGADOR && entidadEnCasilla->tipo != ENTIDAD_JUGADOR) { /// DOS FANTASMAS COLISIONAN

                puedeMoverse = 0;
            }

            if (entidadEnMov->tipo == ENTIDAD_JUGADOR || entidadEnCasilla->tipo == ENTIDAD_JUGADOR) {

                if (logica->ronda.cantVidasActual > 0) {

                    logica->jugador.estado = ENTIDAD_ATURDIDA;
                    logica->ronda.cantVidasActual--;
                    temporizador_iniciar(&logica->jugador.temporEstado);

                } else {

                    logica->jugador.estado = ENTIDAD_SIN_VIDA;
                    logica_fin_juego(logica);
                    cola_vaciar(&logica->movimientos);
                    return TODO_OK;
                }

                if (entidadEnMov->tipo != ENTIDAD_JUGADOR) {
                    entidadEnMov->estado = ENTIDAD_SIN_VIDA;
                    puedeMoverse = 0;
                    logica->escenario.tablero[entidadEnMov->ubic.fila][entidadEnMov->ubic.columna].entidad = NULL;
                }

                if (entidadEnCasilla->tipo != ENTIDAD_JUGADOR) {
                    entidadEnCasilla->estado = ENTIDAD_SIN_VIDA;
                    logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].entidad = NULL;
                }
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
                        logica->ronda.cantPremios++;
                        logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].extra = EXTRA_NINGUNO;
                        break;
                    }
                    case EXTRA_VIDA: {
                        logica->ronda.cantVidasActual++;
                        logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].extra = EXTRA_NINGUNO;
                        logica->jugador.estado = ENTIDAD_POTENCIADA;
                        temporizador_iniciar(&logica->jugador.temporEstado);
                        break;
                    }
                    default:
                        break;
                }
            }

            if (entidadEnMov->tipo == ENTIDAD_JUGADOR && logica->escenario.tablero[ubicNueva.fila][ubicNueva.columna].tile->tileTipo == TILE_TIPO_PUERTA_SALIDA) {

                mov.direccion = 4;
                cola_encolar(&logica->movsJugador, &mov, sizeof(tMovimiento));

                logica_nueva_ronda(logica);

                cola_vaciar(&logica->movimientos);
                return TODO_OK;
            }
        }
    }

    return TODO_OK;
}



void logica_mostrar_historial_movs(tLogica *logica)
{
    unsigned movNro = 1;
    tMovimiento mov;

    if(!cola_vacia(&logica->movsJugador)){

        printf("Tus movimientos realizados:\n");
    }

    while (cola_vacia(&logica->movsJugador) == TODO_OK) {

        cola_desencolar(&logica->movsJugador, &mov, sizeof(tMovimiento));
        switch (mov.direccion){
            case MIRANDO_ABAJO:
                printf("Movimiento numero %-4u | Fila: %3d | Columna: %3d | Direccion: ABAJO\n", movNro++, mov.ubic.fila, mov.ubic.columna);
                break;
            case MIRANDO_IZQUIERDA:
                printf("Movimiento numero %-4u | Fila: %3d | Columna: %3d | Direccion: IZQUIERDA\n", movNro++, mov.ubic.fila, mov.ubic.columna);
                break;
            case MIRANDO_DERECHA:
                printf("Movimiento numero %-4u | Fila: %3d | Columna: %3d | Direccion: DERECHA\n", movNro++, mov.ubic.fila, mov.ubic.columna);
                break;
            case MIRANDO_ARRIBA:
                printf("Movimiento numero %-4u | Fila: %3d | Columna: %3d | Direccion: ARRIBA\n", movNro++, mov.ubic.fila, mov.ubic.columna);
                break;
            default:
                puts("\n Nivel Completado\n");
                break;
        }
    }
}

int logica_iniciar_juego(tLogica *logica)
{
    /// TEST SEMILLA MAESTRA
    ///logica->semillaMaestra = 1758654106;
    ///

    logica->semillaMaestra = time(NULL);
    printf("La semilla maestra de esta partida es: %ld\n", logica->semillaMaestra);

    logica->ronda.numRonda = 1;
    logica->ronda.cantPremios = 0;
    logica->ronda.cantVidasActual = logica->config.vidasInicio;
    logica->puntajeTotal = 0;

    logica->ronda.semillaRonda = logica->semillaMaestra + logica->ronda.numRonda;
    srand(logica->ronda.semillaRonda);

    logica->ronda.cantFantasmas = 1 + (rand() % logica->config.maxFantasmas);
    escenario_generar(&logica->escenario);
    _logica_colocar_jugador(logica);
    _logica_colocar_fantasmas(logica);

    archivo_escribir_escenario(&logica->escenario, logica->ronda.numRonda, logica->ronda.semillaRonda);

    logica->estado = LOGICA_JUGANDO;

    return TODO_OK;
}

int logica_nueva_ronda(tLogica *logica)
{
    puts("Siguiente ronda");

    cola_vaciar(&logica->movimientos);

    logica->ronda.numRonda++;
    logica->ronda.semillaRonda = logica->semillaMaestra + logica->ronda.numRonda;
    srand(logica->ronda.semillaRonda);

    logica->ronda.cantFantasmas = 1 + (rand()  % logica->config.maxFantasmas);
    escenario_generar(&logica->escenario);
    _logica_colocar_jugador(logica);
    _logica_colocar_fantasmas(logica);

    archivo_escribir_escenario(&logica->escenario, logica->ronda.numRonda, logica->ronda.semillaRonda);

    return TODO_OK;
}

void logica_fin_juego(tLogica *logica)
{
    puts("FIN DEL JUEGO");
    printf("Ronda maxima alcanzada: %d "
           "Puntaje obtenido: %d\n"
           ,logica->ronda.numRonda
           ,logica->puntajeTotal
           );
    logica->estado = LOGICA_FIN_PARTIDA;
}


/*************************
    FUNCIONES ESTATICAS
*************************/

static void _logica_colocar_jugador(tLogica *logica)
{
    int pared, columna = logica->escenario.ubicPEntrada.columna, fila = logica->escenario.ubicPEntrada.fila;

    pared = escenario_ubic_es_pared_limite(&logica->escenario, logica->escenario.ubicPEntrada);

    switch (pared) {
        case PARED_SUPERIOR:
            logica->jugador.orientacion = MIRANDO_ABAJO;
            fila++;
            break;
        case PARED_INFERIOR:
            logica->jugador.orientacion = MIRANDO_ARRIBA;
            fila--;
            break;
        case PARED_IZQUIERDA:
            logica->jugador.orientacion = MIRANDO_DERECHA;
            columna++;
            break;
        case PARED_DERECHA:
            logica->jugador.orientacion = MIRANDO_IZQUIERDA;
            columna--;
            break;
        default:
            return;
    }

    logica->jugador.ubic.columna = columna;
    logica->jugador.ubic.fila = fila;
    logica->jugador.tipo = ENTIDAD_JUGADOR;
    logica->jugador.imagen = IMAGEN_JUGADOR;
    logica->jugador.frame = 0;
    logica->jugador.estado = ENTIDAD_CON_VIDA;

    temporizador_inicializar(&logica->jugador.temporFrame, 0.25f);
    temporizador_iniciar(&logica->jugador.temporFrame);

    temporizador_inicializar(&logica->jugador.temporEstado, 1.5f);

    logica->escenario.tablero[fila][columna].tile = &logica->escenario.tiles[TILE_PISO_0];
    logica->escenario.tablero[fila][columna].entidad = &logica->jugador;
    logica->escenario.tablero[fila][columna].transitable = 1;
}

static void _logica_colocar_fantasmas(tLogica *logica)
{
    int columna, fila;
    tEntidad *fantasma, *fantasmaUlt = logica->fantasmas + (logica->ronda.cantFantasmas - 1);

    for (fantasma = logica->fantasmas; fantasma <= fantasmaUlt; fantasma++) {

        int iFantasma = (fantasmaUlt - fantasma) % 4;

        do {
            columna = rand() % logica->config.columnas;
            fila = rand() % logica->config.filas;
        } while(logica->escenario.tablero[fila][columna].tile->tileTipo != TILE_TIPO_PISO
                 || logica->escenario.tablero[fila][columna].entidad);

        fantasma->orientacion = rand() % 4;
        fantasma->frame = rand() % 4;
        fantasma->ubic.columna = columna;
        fantasma->ubic.fila = fila;
        fantasma->ubicAnterior = fantasma->ubic;
        fantasma->imagen = IMAGEN_FANTASMA_01 + iFantasma;
        fantasma->tipo = ENTIDAD_FANTASMA_AMARILLO + iFantasma;
        fantasma->estado = ENTIDAD_CON_VIDA;

        temporizador_inicializar(&fantasma->temporFrame, 0.25f + ((float)(rand() % 101)) / 1000.0f);
        temporizador_iniciar(&fantasma->temporFrame);

        logica->escenario.tablero[fila][columna].entidad = fantasma;
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

    while(pila_vacia(&pila) == TODO_OK && !jugadorEncontrado) {

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

        while(pila_vacia(&pila) == TODO_OK) {

            pila_desapilar(&pila, &ubicProcesada, sizeof(tUbicacion));

            if(pila_vacia(&pila) == TODO_OK) {

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
    int dirFila[] = {-1, 0, 1, 0}, dirColumna[] = {0, 1, 0, -1}, jugadorEncontrado = 0, columna, fila;

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

    while(cola_vacia(&cola) == TODO_OK && !jugadorEncontrado) {

        cola_desencolar(&cola, &ubicProcesada, sizeof(tUbicacion));

        if (ubicProcesada.fila == jugador->ubic.fila && ubicProcesada.columna == jugador->ubic.columna) {
            jugadorEncontrado = 1;
        }

        for(int i = 0; i < 4 && !jugadorEncontrado; i++) {

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
