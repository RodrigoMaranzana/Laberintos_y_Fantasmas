#include "..\..\include\comun\arbol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN(a,b)((a) < (b) ? (a) : (b))

static tNodoArbol** _arbol_buscar_nodo(const tArbol *arbol, const void *dato, tCmp cmp);
static void _arbol_escribir_preorden(const tArbol *arbol, FILE *arch);

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

void arbol_recorrer_posorden(const tArbol *arbol, void *extra, tAccion accion)
{
    if (!*arbol) {

        return;
    }

    arbol_recorrer_posorden(&(*arbol)->izq, extra, accion);
    arbol_recorrer_posorden(&(*arbol)->der, extra, accion);
    accion((*arbol)->dato, extra);
}

int arbol_escribir_en_arch(tArbol *arbol, const char *nombreArch)
{
    FILE *arch = fopen(nombreArch, "wb");
    if (!arch) {
        return ARBOL_ERR_ARCH;
    }

    if (*arbol) {
        _arbol_escribir_preorden(arbol, arch);
    }

    fclose(arch);
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

int arbol_cargar_de_archivo(tArbol *arbol, const char *nombreArch, unsigned tamReg, tCmp cmp)
{
    int ret = ARBOL_TODO_OK;
    FILE *arch;
    void *dato;

    if (*arbol) {

        return ARBOL_NO_INICIALIZADO;
    }

    arch = fopen(nombreArch, "rb");
    if (!arch) {

        return ARBOL_ERR_ARCH;
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

    fclose(arch);
    free(dato);

    return ret;
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

