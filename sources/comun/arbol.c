#include "..\..\include\comun\arbol.h"
#include <stdlib.h>
#include <string.h>

#define MIN(a,b)((a) < (b) ? (a) : (b))

static tNodoArbol** _arbol_buscar_nodo(const tArbol *arbol, const void *dato, tCmp cmp);

void arbol_crear(tArbol *arbol)
{
    *arbol = NULL;
}

int arbol_insertar_rec(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp)
{
    tNodoArbol *nodoNuevo;

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

void arbol_recorrer_preorden(const tArbol *arbol, void *extra, tAccion accion)
{
    if (!*arbol) {

        return;
    }

    accion((*arbol)->dato, extra);
    arbol_recorrer_preorden(&(*arbol)->izq, extra, accion);
    arbol_recorrer_preorden(&(*arbol)->der, extra, accion);
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

int arbol_buscar_y_actualizar(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp)
{
    tNodoArbol **nodo = _arbol_buscar_nodo(arbol, dato, cmp);

    if (!nodo) {

        return ARBOL_NO_ENCONTRADO;
    }

    memcpy((*nodo)->dato, dato, MIN(tamDato, (*nodo)->tamDato));

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




