#include "../../include/cliente/logica.h"
#include "../../include/comun/retorno.h"
#include "../../include/comun/matriz.h"
#include "../../include/cliente/menu.h"
#include "../../include/cliente/temporizador.h"
#include "../../include/comun/pila.h"
#include "../../include/comun/cola.h"
#include "../../include/comun/comun.h"
#include "../../include/cliente/juego.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int logica_ubicacion_valida(const tEscenario *escenario, tUbicacion ubic);
static int _logica_mover_fantasma_bfs(tEscenario *escenario, tEntidad *fantasma);
static int _logica_mover_fantasma_dfs(tEscenario *escenario, tEntidad *fantasma);
static int _logica_es_ubicacion_valida(tEscenario *escenario, unsigned columna, unsigned fila);

void logica_calc_resolucion(unsigned cantColumnas, unsigned cantFilas, unsigned *anchoRes, unsigned *altoRes)
{
    /// DEBE OBTENERSE DESDE EL ARCHIVO DE TEXTO CONF
    *anchoRes =  cantColumnas * PIXELES_TILE;
    *altoRes =  cantFilas * PIXELES_TILE;
}

int logica_inicializar(tLogica *logica)
{
    escenario_crear(&logica->escenario, logica->escenario.confRonda.columnas, logica->escenario.confRonda.filas);
    escenario_generar(&logica->escenario);

    logica->fantasmaEnMov = NULL;

    ///DEBE LEERSE DEL ARCHIVO CONF
    logica->escenario.confRonda.cantFantasmas = 4;

    cola_crear(&logica->movimientosJugador);

    temporizador_inicializar(&logica->fantasmaMovTempor, 0.02f);

    logica->estado = LOGICA_EN_ESPERA;

    return TODO_OK;
}

void logica_destruir(tLogica *logica)
{
    escenario_destruir(&logica->escenario);
}

int logica_actualizar(tLogica *logica)
{
    tEntidad *pFantasmaUlt = logica->escenario.fantasmas + (logica->escenario.confRonda.cantFantasmas - 1);

    if(logica->fantasmaEnMov != NULL && logica->fantasmaEnMov <= pFantasmaUlt){

        /// TEST PARA MODERAR LA DIFICULTAD
        if ((logica->fantasmaEnMov->imagen == IMAGEN_FANTASMA_04 && rand() % 2 == 0)
            || (logica->fantasmaEnMov->imagen == IMAGEN_FANTASMA_03 && rand() % 4 == 0)
            || (logica->fantasmaEnMov->imagen == IMAGEN_FANTASMA_02 && rand() % 8 == 0)) {

            _logica_mover_fantasma_bfs(&logica->escenario, logica->fantasmaEnMov);
        }else{

            _logica_mover_fantasma_dfs(&logica->escenario, logica->fantasmaEnMov);
        }

        logica->fantasmaEnMov++;

    }else{

        logica->fantasmaEnMov = NULL;
    }

    return TODO_OK;
}

void logica_procesar_turno(tLogica *logica, SDL_Keycode tecla)
{
    tUbicacion nuevaUbic = logica->escenario.jugador.ubic;
    tMovimiento mov;

    switch (tecla) {

        case SDLK_UP:
            nuevaUbic.fila--;
            logica->escenario.jugador.orientacion = MIRANDO_ARRIBA;
            break;
        case SDLK_DOWN:
            nuevaUbic.fila++;
            logica->escenario.jugador.orientacion = MIRANDO_ABAJO;
            break;
        case SDLK_LEFT:
            nuevaUbic.columna--;
            logica->escenario.jugador.orientacion = MIRANDO_IZQUIERDA;
            break;
        case SDLK_RIGHT:
            nuevaUbic.columna++;
            logica->escenario.jugador.orientacion = MIRANDO_DERECHA;
            break;
        default:
            return;//este return evita de forma rapida que al tocar la tecla ENTER
            break;
    }

    if (logica_ubicacion_valida(&logica->escenario, nuevaUbic) && logica->escenario.tablero[nuevaUbic.fila][nuevaUbic.columna].transitable) {


        mov.ubic.fila = nuevaUbic.fila;
        mov.ubic.columna = nuevaUbic.columna;
        mov.direccion = logica->escenario.jugador.orientacion;

        cola_encolar(&logica->movimientosJugador, &mov, sizeof(tMovimiento));

        if (logica->escenario.tablero[nuevaUbic.fila][nuevaUbic.columna].entidad) {
                puts("PERDISTE");
                logica->estado = LOGICA_FIN_PARTIDA;
        }

        logica->escenario.tablero[nuevaUbic.fila][nuevaUbic.columna].entidad = logica->escenario.tablero[logica->escenario.jugador.ubic.fila][logica->escenario.jugador.ubic.columna].entidad;
        logica->escenario.tablero[logica->escenario.jugador.ubic.fila][logica->escenario.jugador.ubic.columna].entidad = NULL;

        logica->escenario.jugador.ubic.columna = nuevaUbic.columna;
        logica->escenario.jugador.ubic.fila = nuevaUbic.fila;


        if(logica->fantasmaEnMov == NULL){

            logica->fantasmaEnMov = logica->escenario.fantasmas;
            temporizador_iniciar(&logica->fantasmaMovTempor);
        }

        if (logica->escenario.tablero[nuevaUbic.fila][nuevaUbic.columna].tile->tileTipo == TILE_TIPO_PUERTA_SALIDA) {
            mov.direccion = 4; // ganaste
            cola_encolar(&logica->movimientosJugador, &mov, sizeof(tMovimiento));
            logica_siguiente_nivel(logica);
        }
    }

}

static int logica_ubicacion_valida(const tEscenario *escenario, tUbicacion ubic)
{
    if (ubic.columna < 0 || ubic.columna >= escenario->confRonda.columnas || ubic.fila < 0 || ubic.fila >= escenario->confRonda.filas) {

        return 0;
    }

    return 1;
}

static int _logica_es_ubicacion_valida(tEscenario *escenario, unsigned columna, unsigned fila)
{
    return (fila < escenario->confRonda.filas && columna < escenario->confRonda.columnas && escenario->tablero[fila][columna].transitable);
}

static int _logica_mover_fantasma_dfs(tEscenario *escenario, tEntidad *fantasma)
{
    tPila pila;
    tUbicacion ubicProcesada, ubicVecina, primerMovimiento;
    int dirFila[] = {-1, 0, 1, 0}, dirColumna[] = {0, 1, 0, -1}, jugadorEncontrado = 0, columna, fila;

    for(fila = 0; fila < escenario->confRonda.filas; fila++){

        for(columna = 0; columna < escenario->confRonda.columnas; columna++){

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

        if(escenario->tablero[ubicProcesada.fila][ubicProcesada.columna].entidad == &escenario->jugador) {

            jugadorEncontrado = 1;
        }

        for(i = 0; i < 4 && !nuevaUbicEncontrada && !jugadorEncontrado; i++) {

            char noVuelveAtras, esUbicacionValida, esNoVisitada, estaLibre;

            ubicVecina.fila = ubicProcesada.fila + dirFila[i];
            ubicVecina.columna = ubicProcesada.columna + dirColumna[i];

            noVuelveAtras = (ubicVecina.fila != fantasma->ubicAnterior.fila || ubicVecina.columna != fantasma->ubicAnterior.columna);
            esUbicacionValida = _logica_es_ubicacion_valida(escenario, ubicVecina.columna, ubicVecina.fila);

            if (esUbicacionValida) {

                esNoVisitada = !escenario->tablero[ubicVecina.fila][ubicVecina.columna].visitada;
                estaLibre = escenario->tablero[ubicVecina.fila][ubicVecina.columna].entidad == NULL || escenario->tablero[ubicVecina.fila][ubicVecina.columna].entidad == &escenario->jugador;
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

        escenario->tablero[fantasma->ubic.fila][fantasma->ubic.columna].entidad = NULL;

        if (primerMovimiento.columna > fantasma->ubic.columna) {

            fantasma->orientacion = MIRANDO_DERECHA;
        }
        else if (primerMovimiento.columna < fantasma->ubic.columna) {

            fantasma->orientacion = MIRANDO_IZQUIERDA;
        }
        else if (primerMovimiento.fila > fantasma->ubic.fila) {

            fantasma->orientacion = MIRANDO_ABAJO;
        }
        else if (primerMovimiento.fila < fantasma->ubic.fila) {

            fantasma->orientacion = MIRANDO_ARRIBA;
        }

        fantasma->ubicAnterior = fantasma->ubic;
        fantasma->ubic = primerMovimiento;
        escenario->tablero[fantasma->ubic.fila][fantasma->ubic.columna].entidad = fantasma;///
    }

    pila_vaciar(&pila);

    return TODO_OK;
}

static int _logica_mover_fantasma_bfs(tEscenario *escenario, tEntidad *fantasma)
{
    tCola cola;
    tUbicacion ubicProcesada, ubicVecina;
    int dirFila[] = {-1, 0, 1, 0}, dirColumna[] = {0, 1, 0, -1}, jugadorEncontrado = 0, columna, fila;

    tUbicacion **predecesores = (tUbicacion**)matriz_crear((size_t)escenario->confRonda.columnas, (size_t)escenario->confRonda.filas, sizeof(tUbicacion));
    if (!predecesores) {

        puts("ERROR: No se pudo mover al fantasma");
        return ERR_SIN_MEMORIA;
    }

    for (fila = 0; fila < escenario->confRonda.filas; fila++) {

        for (columna = 0; columna < escenario->confRonda.columnas; columna++) {

            predecesores[fila][columna].fila = -1;
            predecesores[fila][columna].columna = -1;

            escenario->tablero[fila][columna].visitada = 0;
        }
    }

    cola_crear(&cola);

    ubicProcesada = fantasma->ubic;
    predecesores[ubicProcesada.fila][ubicProcesada.columna].columna = -2;
    predecesores[ubicProcesada.fila][ubicProcesada.columna].fila = -2;

    cola_encolar(&cola, &ubicProcesada, sizeof(tUbicacion));

    while(cola_vacia(&cola) == TODO_OK && !jugadorEncontrado) {

        cola_desencolar(&cola, &ubicProcesada, sizeof(tUbicacion));

        if (ubicProcesada.fila == escenario->jugador.ubic.fila && ubicProcesada.columna == escenario->jugador.ubic.columna) {

            jugadorEncontrado = 1;
        }

        for(int i = 0; i < 4 && !jugadorEncontrado; i++) {

            ubicVecina.fila = ubicProcesada.fila + dirFila[i];
            ubicVecina.columna = ubicProcesada.columna + dirColumna[i];

            if(_logica_es_ubicacion_valida(escenario, ubicVecina.columna, ubicVecina.fila) && !escenario->tablero[ubicVecina.fila][ubicVecina.columna].visitada
               && (escenario->tablero[ubicVecina.fila][ubicVecina.columna].entidad == NULL || escenario->tablero[ubicVecina.fila][ubicVecina.columna].entidad == &escenario->jugador)) {

                predecesores[ubicVecina.fila][ubicVecina.columna] = ubicProcesada;
                escenario->tablero[ubicVecina.fila][ubicVecina.columna].visitada = 1;
                cola_encolar(&cola, &ubicVecina, sizeof(tUbicacion));
            }
        }
    }

    if (jugadorEncontrado) {

        char raizEncontrada = 0;
        tUbicacion primerMovimiento = escenario->jugador.ubic;

        while (!raizEncontrada && (predecesores[primerMovimiento.fila][primerMovimiento.columna].fila != fantasma->ubic.fila
               || predecesores[primerMovimiento.fila][primerMovimiento.columna].columna != fantasma->ubic.columna)) {

            if (predecesores[primerMovimiento.fila][primerMovimiento.columna].fila == -2) {

                raizEncontrada = 1;
            }

            primerMovimiento = predecesores[primerMovimiento.fila][primerMovimiento.columna];
        }

        escenario->tablero[fantasma->ubic.fila][fantasma->ubic.columna].entidad = NULL;

        if (primerMovimiento.columna > fantasma->ubic.columna) {

            fantasma->orientacion = MIRANDO_DERECHA;
        }
        else if (primerMovimiento.columna < fantasma->ubic.columna) {

            fantasma->orientacion = MIRANDO_IZQUIERDA;
        }
        else if (primerMovimiento.fila > fantasma->ubic.fila) {

            fantasma->orientacion = MIRANDO_ABAJO;
        }
        else if (primerMovimiento.fila < fantasma->ubic.fila) {

            fantasma->orientacion = MIRANDO_ARRIBA;
        }

        fantasma->ubic = primerMovimiento;
        escenario->tablero[fantasma->ubic.fila][fantasma->ubic.columna].entidad = fantasma;
    }

    cola_vaciar(&cola);

    matriz_destruir((void**)predecesores, escenario->confRonda.filas);

    return TODO_OK;
}

void logica_mostrar_historial_movimientos(tLogica *logica)
{
    unsigned paso = 1;
    tMovimiento mov;

    if(!cola_vacia(&logica->movimientosJugador))
        printf("Tus movimientos realizados:\n");

    // Vacia la cola original mostrando los movimientos
    while (cola_vacia(&logica->movimientosJugador) == TODO_OK)
    {
        cola_desencolar(&logica->movimientosJugador, &mov, sizeof(tMovimiento));
        switch (mov.direccion){
        case MIRANDO_ABAJO:
            printf("Paso %u: Fila=%d, Columna=%d, Direccion=ABAJO\n", paso++, mov.ubic.fila, mov.ubic.columna);
            break;
        case MIRANDO_IZQUIERDA:
            printf("Paso %u: Fila=%d, Columna=%d, Direccion=IZQUIERDA\n", paso++, mov.ubic.fila, mov.ubic.columna);
            break;
        case MIRANDO_DERECHA:
            printf("Paso %u: Fila=%d, Columna=%d, Direccion=DERECHA\n", paso++, mov.ubic.fila, mov.ubic.columna);
            break;
        case MIRANDO_ARRIBA:
            printf("Paso %u: Fila=%d, Columna=%d, Direccion=ARRIBA\n", paso++, mov.ubic.fila, mov.ubic.columna);
            break;
        default:
            puts("\n Nivel Completado:\n");
            break;
        }
    }
}

int logica_siguiente_nivel(tLogica *logica)
{
    logica->partida.ronda.numero++;

    escenario_destruir(&logica->escenario);

    escenario_crear(&logica->escenario,
                    logica->escenario.confRonda.columnas,
                    logica->escenario.confRonda.filas);

    escenario_generar(&logica->escenario);

    // Reiniciar ciclo de movimiento de fantasmas
    logica->fantasmaEnMov = NULL;
    temporizador_inicializar(&logica->fantasmaMovTempor, 0.02f);

    logica->estado = LOGICA_JUGANDO;

    return TODO_OK;
}
