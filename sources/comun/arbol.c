#include "..\..\include\comun\arbol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN(a,b)((a) < (b) ? (a) : (b))

static tNodoArbol** _arbol_buscar_nodo(const tArbol *arbol, const void *dato, tCmp cmp);
static void _arbol_escribir_preorden(const tArbol *arbol, FILE *arch);
static void _arbol_escribir_preorden_con_escritor(const tArbol *arbol, FILE *arch, tEscribir escribir);
static int _arbol_recorrer_inverso_lim(const tArbol *arbol, void *extra, tAccionLimite accionLimite);
static int _arbol_eliminar_raiz(tArbol *arbol);
static tNodoArbol** _arbol_menor_nodo(const tArbol *arbol);
static tNodoArbol** _arbol_mayor_nodo(const tArbol *arbol);
static unsigned _arbol_altura(const tArbol *arbol);


void arbol_crear(tArbol *arbol)
{
    *arbol = NULL;
}

int arbol_insertar_rec(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp)
{
    tNodoArbol *nodoNuevo; // no es necesaria

    if (*arbol) {

        int comp;

        if ((comp = cmp(dato, (*arbol)->dato)) < 0) {

            return arbol_insertar_rec(&(*arbol)->izq, dato, tamDato, cmp);
        } else if (comp > 0) {

            return arbol_insertar_rec(&(*arbol)->der, dato, tamDato, cmp);
        }

        return ARBOL_DATO_DUP;
    }

    nodoNuevo = malloc(sizeof(tNodoArbol));
    if (!nodoNuevo) {
        return ARBOL_SIN_MEM;
    }

    nodoNuevo->dato = malloc(tamDato);
    if (!nodoNuevo->dato) {
        free(nodoNuevo);
        return ARBOL_SIN_MEM;
    }

    memcpy(nodoNuevo->dato, dato, tamDato);

    nodoNuevo->tamDato = tamDato;
    nodoNuevo->izq = NULL;
    nodoNuevo->der = NULL;
    *arbol = nodoNuevo;

    return ARBOL_TODO_OK;
}

int arbol_insertar_it(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp)
{
    while (*arbol) {

        int comp;

        if ((comp = cmp(dato, (*arbol)->dato)) < 0) {
            arbol = &(*arbol)->izq;
        } else if (comp > 0) {
            arbol = &(*arbol)->der;
        } else if (comp == 0){
            return ARBOL_DATO_DUP;
        }
    }

    *arbol = malloc(sizeof(tNodoArbol));
    if (!*arbol) {
        return ARBOL_SIN_MEM;
    }

    (*arbol)->dato = malloc(tamDato);
    if (!(*arbol)->dato) {
        free(*arbol);
        *arbol = NULL;
        return ARBOL_SIN_MEM;
    }

    memcpy((*arbol)->dato, dato, tamDato);

    (*arbol)->tamDato = tamDato;
    (*arbol)->izq = NULL;
    (*arbol)->der = NULL;

    return ARBOL_TODO_OK;
}

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

void arbol_recorrer_orden_inverso(const tArbol *arbol, void *extra, tAccion accion)
{
    if (!*arbol) {
        return;
    }

    arbol_recorrer_orden_inverso(&(*arbol)->der, extra, accion);
    accion((*arbol)->dato, extra);
    arbol_recorrer_orden_inverso(&(*arbol)->izq, extra, accion);
}

void arbol_recorrer_preorden(const tArbol *arbol, void *extra, tAccion accion)
{
    if (!*arbol) {
        return;
    }

    accion((*arbol)->dato, extra);
    arbol_recorrer_preorden(&(*arbol)->izq, extra, accion);
    arbol_recorrer_preorden(&(*arbol)->der, extra, accion);
}

void arbol_recorrer_posorden(const tArbol *arbol, void *extra, tAccion accion)
{
    if (!*arbol) {

        return;
    }

    arbol_recorrer_posorden(&(*arbol)->izq, extra, accion);
    arbol_recorrer_posorden(&(*arbol)->der, extra, accion);
    accion((*arbol)->dato, extra);
}

int arbol_escribir_en_arch(FILE *arch, tArbol *arbol)
{
    if (*arbol) {
        _arbol_escribir_preorden(arbol, arch);
    }

    return ARBOL_TODO_OK;
}

int arbol_escribir_en_arch_con_escritor(FILE *arch, tArbol *arbol, tEscribir escribir)
{
    if (*arbol) {
        _arbol_escribir_preorden_con_escritor(arbol, arch, escribir);
    }
    return ARBOL_TODO_OK;
}

static void _arbol_escribir_preorden(const tArbol *arbol, FILE *arch)
{
    if (!*arbol) {
        return;
    }

    fwrite((*arbol)->dato, (*arbol)->tamDato, 1, arch);
    _arbol_escribir_preorden(&(*arbol)->izq, arch);
    _arbol_escribir_preorden(&(*arbol)->der, arch);
}

static void _arbol_escribir_preorden_con_escritor(const tArbol *arbol, FILE *arch, tEscribir escribir)
{
    if (!*arbol) {
        return;
    }

    escribir((*arbol)->dato, arch);
    _arbol_escribir_preorden_con_escritor(&(*arbol)->izq, arch, escribir);
    _arbol_escribir_preorden_con_escritor(&(*arbol)->der, arch, escribir);
}

int arbol_cargar_de_arch(FILE *arch, tArbol *arbol, unsigned tamReg, tCmp cmp)
{
    int ret = ARBOL_TODO_OK;
    void *dato;

    if (*arbol) {
        return ARBOL_NO_INICIALIZADO;
    }

    dato = malloc(tamReg);
    if (!dato) {

        fclose(arch);
        return ARBOL_SIN_MEM;
    }

    while (ret == ARBOL_TODO_OK && fread(dato, tamReg, 1, arch)) {

       ret = arbol_insertar_rec(arbol, dato, tamReg, cmp);
       if (ret != ARBOL_TODO_OK) {
            arbol_vaciar(arbol);
       }
    }

    free(dato);
    return ret;
}

int arbol_cargar_de_arch_con_lector(FILE *arch, tArbol *arbol, tLeer leer, tCmp cmp)
{
    int ret = ARBOL_TODO_OK;
    void *dato;
    unsigned tamDato;

    if (*arbol) {
        return ARBOL_NO_INICIALIZADO;
    }

    while (ret == ARBOL_TODO_OK && leer(&dato, &tamDato, arch)) {

       ret = arbol_insertar_rec(arbol, dato, tamDato, cmp);
       free(dato);
       dato = NULL;
       if (ret != ARBOL_TODO_OK) {
            arbol_vaciar(arbol);
       }
    }

    return ret;
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

int arbol_buscar(const tArbol *arbol, void *dato, unsigned tamDato, tCmp cmp)
{
    tNodoArbol **nodo = _arbol_buscar_nodo(arbol, dato, cmp);

    if (!nodo) {
        return ARBOL_NO_ENCONTRADO;
    }

    memcpy(dato, (*nodo)->dato, MIN(tamDato, (*nodo)->tamDato));

    return ARBOL_TODO_OK;
}

static tNodoArbol** _arbol_buscar_nodo(const tArbol *arbol, const void *dato, tCmp cmp)
{
    int comp;
    tNodoArbol **nodoAux = (tNodoArbol**)arbol;

    while (*nodoAux && (comp = cmp(dato, (*nodoAux)->dato))) {
        nodoAux = comp < 0 ? &(*nodoAux)->izq : &(*nodoAux)->der;
    }

    if (!(*nodoAux)) {

        return NULL;
    }

    return nodoAux;
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

static int _arbol_eliminar_raiz(tArbol *arbol)
{
    tNodoArbol **remp, *aux;

    if (!*arbol) {
        return ARBOL_VACIO;
    }

    free((*arbol)->dato);
    if (!(*arbol)->izq && !(*arbol)->der) {

        free(*arbol);
        *arbol = NULL;
        return ARBOL_TODO_OK;
    }

    remp = _arbol_altura(&(*arbol)->izq) > _arbol_altura(&(*arbol)->der) ? _arbol_mayor_nodo(&(*arbol)->izq) : _arbol_menor_nodo(&(*arbol)->der);

    aux = *remp;
    (*arbol)->dato = aux->dato;
    (*arbol)->tamDato = aux->tamDato;

    *remp = aux->izq ? aux->izq : aux->der;

    free(aux);

    return ARBOL_TODO_OK;
}

static unsigned _arbol_altura(const tArbol *arbol)
{
    int alturaI, alturaD;

    if (!*arbol) {
        return ARBOL_VACIO;
    }

    alturaI = _arbol_altura(&(*arbol)->izq);
    alturaD = _arbol_altura(&(*arbol)->der);

    return (alturaI > alturaD ? alturaI : alturaD) + 1;
}

static tNodoArbol** _arbol_mayor_nodo(const tArbol *arbol)
{
    if (!*arbol) {
        return NULL;
    }

    while ((*arbol)->der) {
        arbol = &(*arbol)->der;
    }

    return (tNodoArbol**)arbol;
}


static tNodoArbol** _arbol_menor_nodo(const tArbol *arbol)
{
    if (!*arbol) {
        return NULL;
    }

    while ((*arbol)->izq) {
        arbol = &(*arbol)->izq;
    }

    return (tNodoArbol**)arbol;
}








