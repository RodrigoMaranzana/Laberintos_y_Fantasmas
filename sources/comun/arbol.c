#include "..\..\include\comun\arbol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN(a,b)((a) < (b) ? (a) : (b))

static tNodoArbol** _arbol_buscar_nodo(const tArbol *arbol, const void *dato, tCmp cmp);
static int _arbol_altura(const tArbol *arbol);
static tNodoArbol** _arbol_menor_clave(const tArbol *arbol);
static tNodoArbol** _arbol_mayor_clave(const tArbol *arbol);
static int _arbol_eliminar_raiz(tArbol *arbol);
static int _arbol_cargar_desde_datos_ordenados(tArbol *arbol, void *datos, tLeer leer, int limiteInf, int limiteSup, void *extra);
static unsigned _arbol_leer_arch_bin(void **datoDest, void *datoFuente, int pos, void *extra);

/// REVISAR
static int _arbol_recorrer_inverso_lim(const tArbol *arbol, void *extra, tAccionLimite accionLimite);
/// REVISAR


void arbol_crear(tArbol *arbol)
{
    *arbol = NULL;
}

void arbol_vaciar(tArbol *arbol)
{
    if (!*arbol) {
        return;
    }

    arbol_vaciar(&(*arbol)->izq);
    arbol_vaciar(&(*arbol)->der);

    free((*arbol)->dato);
    free(*arbol);
    *arbol = NULL;
}

int arbol_insertar(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp)
{
    if (*arbol) {
        int comp;
        if ((comp = cmp(dato, (*arbol)->dato)) < 0) {
            return arbol_insertar(&(*arbol)->izq, dato, tamDato, cmp);
        } else if (comp > 0) {
            return arbol_insertar(&(*arbol)->der, dato, tamDato, cmp);
        }

        return ARBOL_DATO_DUP;
    }

    *arbol = malloc(sizeof(tNodoArbol));
    if (!*arbol) {
        return ARBOL_SIN_MEM;
    }

    (*arbol)->dato = malloc(tamDato);
    if (!(*arbol)->dato) {
        free(*arbol);
        return ARBOL_SIN_MEM;
    }

    memcpy((*arbol)->dato, dato, tamDato);

    (*arbol)->tamDato = tamDato;
    (*arbol)->izq = NULL;
    (*arbol)->der = NULL;

    return ARBOL_TODO_OK;
}

void arbol_recorrer_preorden(const tArbol *arbol, unsigned nivel, void *extra, tAccionArbol accion)
{
    if (!*arbol) {
        return;
    }

    accion((*arbol)->dato, (*arbol)->tamDato, nivel, extra);
    arbol_recorrer_preorden(&(*arbol)->izq, nivel + 1, extra, accion);
    arbol_recorrer_preorden(&(*arbol)->der, nivel + 1, extra, accion);
}

void arbol_recorrer_orden(const tArbol *arbol, unsigned nivel, void *extra, tAccionArbol accion)
{
    if (!*arbol) {
        return;
    }

    arbol_recorrer_orden(&(*arbol)->izq, nivel + 1, extra, accion);
    accion((*arbol)->dato, (*arbol)->tamDato, nivel, extra);
    arbol_recorrer_orden(&(*arbol)->der, nivel + 1, extra, accion);
}

void arbol_recorrer_orden_inverso(const tArbol *arbol, unsigned nivel, void *extra, tAccionArbol accion)
{
    if (!*arbol) {
        return;
    }

    arbol_recorrer_orden_inverso(&(*arbol)->der, nivel + 1, extra, accion);
    accion((*arbol)->dato, (*arbol)->tamDato, nivel, extra);
    arbol_recorrer_orden_inverso(&(*arbol)->izq, nivel + 1, extra, accion);
}

void arbol_recorrer_posorden(const tArbol *arbol, unsigned nivel, void *extra, tAccionArbol accion)
{
    if (!*arbol) {
        return;
    }

    arbol_recorrer_posorden(&(*arbol)->izq, nivel + 1, extra, accion);
    arbol_recorrer_posorden(&(*arbol)->der, nivel + 1, extra, accion);
    accion((*arbol)->dato, (*arbol)->tamDato, nivel, extra);
}

int arbol_buscar(const tArbol *arbol, void *dato, unsigned tamDato, tCmp cmp)
{
    tNodoArbol **nodo = _arbol_buscar_nodo(arbol, dato, cmp);

    if (!nodo) {
        return ARBOL_NO_ENCONTRADO;
    }

    memcpy(dato, (*nodo)->dato, MIN(tamDato, (*nodo)->tamDato));

    return ARBOL_TODO_OK;
}

int arbol_escribir_arch_bin_orden(const tArbol *arbol, FILE *arch)
{
    if (!*arbol) {
        return ARBOL_VACIO;
    }

    arbol_escribir_arch_bin_orden(&(*arbol)->izq, arch);
    if (fwrite((*arbol)->dato, (*arbol)->tamDato, 1, arch) != 1) {
        return ARBOL_ERR_ARCH;
    }
    arbol_escribir_arch_bin_orden(&(*arbol)->der, arch);

    return ARBOL_TODO_OK;
}

int arbol_escribir_arch_bin_orden_con_escritor(const tArbol *arbol, FILE *arch, tEscribir escribir, void *extra)
{
    if (!*arbol) {
        return ARBOL_VACIO;
    }

    arbol_escribir_arch_bin_orden_con_escritor(&(*arbol)->izq, arch, escribir, extra);
    if (escribir((*arbol)->dato, arch, extra) != 1) {
        return ARBOL_ERR_ARCH;
    }
    arbol_escribir_arch_bin_orden_con_escritor(&(*arbol)->der, arch, escribir, extra);

    return ARBOL_TODO_OK;
}

void arbol_vaciar_destructor(tArbol *arbol, tDestruir destruir)
{
    if (!*arbol) {
        return;
    }

    arbol_vaciar_destructor(&(*arbol)->izq, destruir);
    arbol_vaciar_destructor(&(*arbol)->der, destruir);

    if (destruir) {
        destruir((*arbol)->dato);
    }

    free((*arbol)->dato);
    free(*arbol);
    *arbol = NULL;
}

int arbol_eliminar(tArbol *arbol, void *dato, unsigned tamDato, tCmp cmp)
{
    tArbol *elim = _arbol_buscar_nodo(arbol, dato, cmp);

    if (!elim) {
        return ARBOL_NO_ENCONTRADO;
    }

    memcpy(dato, (*elim)->dato, MIN(tamDato, (*elim)->tamDato));

    return _arbol_eliminar_raiz(elim);
}

int arbol_cargar_arch_bin_ordenado(tArbol *arbol, FILE* arch, unsigned tamDato)
{
    int cantReg;
    if (*arbol) {
        return ARBOL_NO_INICIALIZADO;
    }

    fseek(arch, 0, SEEK_END);
    cantReg = ftell(arch) / tamDato;

    if (cantReg == 0) {
        return ARBOL_TODO_OK;
    }

    return _arbol_cargar_desde_datos_ordenados(arbol, arch, _arbol_leer_arch_bin, 0, cantReg - 1, &tamDato);
}

int arbol_cargar_datos_ordenados(tArbol *arbol, void *datos, int cantReg, void *extra, tLeer leer)
{
    if (*arbol) {
        return ARBOL_NO_INICIALIZADO;
    }

    if (cantReg == 0) {
        return ARBOL_TODO_OK;
    }

    return _arbol_cargar_desde_datos_ordenados(arbol, datos, leer, 0, cantReg - 1, extra);
}

/*************************
    FUNCIONES ESTATICAS
*************************/

static tNodoArbol** _arbol_buscar_nodo(const tArbol *arbol, const void *dato, tCmp cmp)
{
    int comp;
    tNodoArbol **nodoActual = (tNodoArbol**)arbol;

    while (*nodoActual && (comp = cmp(dato, (*nodoActual)->dato))) {
        nodoActual = comp < 0 ? &(*nodoActual)->izq : &(*nodoActual)->der;
    }

    return *nodoActual ? nodoActual : NULL;
}

static int _arbol_altura(const tArbol *arbol)
{
    int alturaIzq, alturaDer;

    if (!*arbol) {
        return 0;
    }

    alturaIzq = _arbol_altura(&(*arbol)->izq);
    alturaDer = _arbol_altura(&(*arbol)->der);

    return (alturaIzq > alturaDer ? alturaIzq : alturaDer) + 1;
}

static tNodoArbol** _arbol_mayor_clave(const tArbol *arbol)
{
    if (!*arbol) {
        return NULL;
    }

    while ((*arbol)->der) {
        arbol = &(*arbol)->der;
    }

    return (tNodoArbol**)arbol;
}

static tNodoArbol** _arbol_menor_clave(const tArbol *arbol)
{
    if (!*arbol) {
        return NULL;
    }

    while ((*arbol)->izq) {
        arbol = &(*arbol)->izq;
    }

    return (tNodoArbol**)arbol;
}

static int _arbol_eliminar_raiz(tArbol *arbol)
{
    tNodoArbol **reemp, *elim;

    if (!*arbol) {
        return ARBOL_VACIO;
    }

    free((*arbol)->dato);
    if (!(*arbol)->izq && !(*arbol)->der) {

        free(*arbol);
        *arbol = NULL;
        return ARBOL_TODO_OK;
    }

    reemp = _arbol_altura(&(*arbol)->izq) > _arbol_altura(&(*arbol)->der) ? _arbol_mayor_clave(&(*arbol)->izq) : _arbol_menor_clave(&(*arbol)->der);

    elim = *reemp;
    (*arbol)->dato = elim->dato;
    (*arbol)->tamDato = elim->tamDato;

    *reemp = elim->izq ? elim->izq : elim->der;

    free(elim);

    return ARBOL_TODO_OK;
}

static int _arbol_cargar_desde_datos_ordenados(tArbol *arbol, void *datos, tLeer leer, int limiteInf, int limiteSup, void *extra)
{
    int ret, mitad = (limiteInf + limiteSup) / 2;

    if (limiteInf > limiteSup) {
        return ARBOL_TODO_OK;
    }

    *arbol = (tNodoArbol*)malloc(sizeof(tNodoArbol));
    if (!*arbol) {
        return ARBOL_SIN_MEM;
    }

    (*arbol)->tamDato = leer(&(*arbol)->dato, datos, mitad, extra);
    if (!(*arbol)->tamDato) {
        free(*arbol);
        return ARBOL_SIN_MEM;
    }

    (*arbol)->izq = NULL;
    (*arbol)->der = NULL;

    if ((ret = _arbol_cargar_desde_datos_ordenados(&(*arbol)->izq, datos, leer, limiteInf, mitad - 1, extra)) != ARBOL_TODO_OK) {
        return ret;
    }

    return _arbol_cargar_desde_datos_ordenados(&(*arbol)->der, datos, leer, mitad + 1, limiteSup, extra);
}

static unsigned _arbol_leer_arch_bin(void **datoDest, void *datoFuente, int pos, void *extra)
{
    unsigned tamReg = *((unsigned*)extra);

    *datoDest = malloc(tamReg);
    if (!*datoDest) {
        return ARBOL_SIN_MEM;
    }

    fseek((FILE*)datoFuente, pos * tamReg, SEEK_SET);

    if (fread(*datoDest, tamReg, 1, (FILE*)datoFuente) == 1) {
        return tamReg;
    }

    free(*datoDest);
    *datoDest = NULL;
    return 0;
}



/// REVISAR
void arbol_recorrer_orden_inverso_con_limite(const tArbol *arbol, void *extra, tAccionLimite accionLimite)
{
    _arbol_recorrer_inverso_lim(arbol, extra, accionLimite);
}

static int _arbol_recorrer_inverso_lim(const tArbol *arbol, void *extra, tAccionLimite accionLimite)
{
    if (!*arbol) {
        return 1;
    }

    if (!_arbol_recorrer_inverso_lim(&(*arbol)->der, extra, accionLimite)) {
        return 0;
    }

    if (!accionLimite((*arbol)->dato, extra)) {
        return 0;
    }

    return _arbol_recorrer_inverso_lim(&(*arbol)->izq, extra, accionLimite);
}
/// REVISAR
