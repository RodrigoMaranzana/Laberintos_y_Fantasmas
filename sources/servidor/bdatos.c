#include <string.h>
#include "../../include/servidor/bdatos.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/arbol.h"

static int _bdatos_cmp_jugador(const void *jA, const void *jB);
static void _bdatos_accion_grabar_arbol(void *elem, void *extra);
static int _bdatos_actualizar_idx(tBDatos *bDatos);

// Funciones de prueba
static void __bdatos_insertar_lote_pruebas(tBDatos *bDatos);


int bdatos_iniciar(tBDatos *bDatos)
{
    int retorno = TODO_OK;

    bDatos->jugadorSesion.offset = -1;
    *bDatos->jugadorSesion.usuario = '\0';

    arbol_crear(&bDatos->arbol);

    bDatos->arch = fopen(ARCH_NOMBRE, "r+");
    if (!bDatos->arch) {

        puts("Creando base de datos...\n");
        retorno = bdatos_crear(bDatos);

        __bdatos_insertar_lote_pruebas(bDatos);
    }

    bDatos->archIdx = fopen(ARCH_IDX_NOMBRE, "r+");
    if (!bDatos->archIdx) {

        bDatos->archIdx = fopen(ARCH_IDX_NOMBRE, "w+");

    } else {

        retorno = bdatos_cargar_idx_en_arbol(bDatos);
    }

    return retorno;
}

int bdatos_crear(tBDatos *bDatos)
{
    bDatos->arch = fopen(ARCH_NOMBRE, "w+");
    if (!bDatos->arch) {

        bDatos->archIdx = NULL;
        return ERR_ARCHIVO;
    }

    bDatos->archIdx = fopen(ARCH_IDX_NOMBRE, "w+");
    if (!bDatos->archIdx) {

        fclose(bDatos->arch);
        bDatos->arch = NULL;
        return ERR_ARCHIVO;
    }

    return TODO_OK;
}

int bdatos_cerrar(tBDatos *bDatos)
{
    int retorno = TODO_OK;

    fclose(bDatos->arch);

    retorno = _bdatos_actualizar_idx(bDatos);
    fclose(bDatos->archIdx);

    return retorno;
}


int bdatos_insertar_jugador(tBDatos *bDatos, tJugador *jugador)
{
    arbol_insertar_rec(&bDatos->arbol, jugador, sizeof(tJugador), _bdatos_cmp_jugador);

    memcpy(&bDatos->jugadorSesion, jugador, sizeof(tJugador));

    return TODO_OK;
}


int bdatos_insertar_partida(tBDatos *bDatos, const tPartida *partida)
{
    tPartida partidaAux;
    long offsetNuevaPartida, offsetUltPartida, offsetAux = bDatos->jugadorSesion.offset;

    fseek(bDatos->arch, 0, SEEK_END);
    offsetNuevaPartida = ftell(bDatos->arch);

    // Si no hay partidas, guarda la primera
    if (offsetAux == -1) {

        bDatos->jugadorSesion.offset = offsetNuevaPartida;

        arbol_buscar_y_actualizar(&bDatos->arbol, &bDatos->jugadorSesion, sizeof(tJugador), _bdatos_cmp_jugador);

        fwrite(partida, sizeof(tPartida), 1, bDatos->arch);

        return TODO_OK;
    }

    // Busca la ultima partida
    while (offsetAux != -1) {

        fseek(bDatos->arch, offsetAux, SEEK_SET);
        offsetUltPartida = ftell(bDatos->arch);
        fread(&partidaAux, sizeof(tPartida), 1, bDatos->arch);
        offsetAux = partidaAux.offsetSig;
    }

    partidaAux.offsetSig = offsetNuevaPartida;
    fseek(bDatos->arch, offsetNuevaPartida, SEEK_SET);
    fwrite(partida, sizeof(tPartida), 1, bDatos->arch);

    fseek(bDatos->arch, offsetUltPartida, SEEK_SET);
    fwrite(&partidaAux, sizeof(tPartida), 1, bDatos->arch);

    return TODO_OK;
}

int bdatos_cargar_idx_en_arbol(tBDatos *bDatos)
{
    tJugador jugador;
    fseek(bDatos->archIdx, 0, SEEK_SET);

    while (fread(&jugador, sizeof(tJugador), 1, bDatos->archIdx) == 1) {

        arbol_insertar_rec(&bDatos->arbol, &jugador, sizeof(tJugador), _bdatos_cmp_jugador);
    }

    return TODO_OK;
}


int bdatos_buscar(const tBDatos *bDatos, tJugador *jugador)
{
    return arbol_buscar(&bDatos->arbol, jugador, sizeof(tJugador), _bdatos_cmp_jugador);;
}

static int _bdatos_actualizar_idx(tBDatos *bDatos)
{
    fclose(bDatos->archIdx);

    bDatos->archIdx = fopen(ARCH_IDX_NOMBRE, "w+");
    if (!bDatos->archIdx) {

        return ERR_ARCHIVO;
    }

    arbol_recorrer_preorden(&bDatos->arbol, bDatos->archIdx, _bdatos_accion_grabar_arbol);

    return TODO_OK;
}

static void _bdatos_accion_grabar_arbol(void *elem, void *extra)
{
    FILE *archIdx = (FILE*)extra;

    fwrite(elem, sizeof(tJugador), 1, archIdx);
}

static int _bdatos_cmp_jugador(const void *jA, const void *jB)
{
    return strcmp(((tJugador*)(jA))->usuario, ((tJugador*)(jB))->usuario);
}

static void __bdatos_insertar_lote_pruebas(tBDatos *bDatos)
{
    tJugador    jugadorA = {"MANUEL", -1},
                jugadorB = {"GABRIELA", -1},
                jugadorC = {"EZEQUIEL", -1},
                jugadorD = {"JAVIER", -1};

    tPartida    partidaA = {21, 4, 110, -1},
                partidaB = {11, 2, 67, -1},
                partidaC = {11, 2, 67, -1},
                partidaD = {6, 1, 33, -1},
                partidaE = {2, 1, 17, -1},
                partidaF = {19, 3, 46, -1};

    bdatos_insertar_jugador(bDatos, &jugadorA);
    bdatos_insertar_partida(bDatos, &partidaA);
    bdatos_insertar_partida(bDatos, &partidaB);

    bdatos_insertar_jugador(bDatos, &jugadorB);
    bdatos_insertar_partida(bDatos, &partidaC);

    bdatos_insertar_jugador(bDatos, &jugadorC);

    bdatos_insertar_jugador(bDatos, &jugadorD);

    bdatos_insertar_partida(bDatos, &partidaD);
    bdatos_insertar_partida(bDatos, &partidaE);
    bdatos_insertar_partida(bDatos, &partidaF);
}



