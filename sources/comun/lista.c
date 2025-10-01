#include "../../include/comun/lista.h"
#include <stdlib.h>
#include <string.h>

#define MIN(a,b)((a) < (b) ? (a) : (b))

void lista_crear(tLista *lista)
{
    *lista = NULL;
}


void lista_recorrer(const tLista *lista, tAccion accion, void *extra)
{
    while (*lista) {

        accion((*lista)->dato, extra);
        lista = &(*lista)->sig;
    }
}


int lista_insertar_final(tLista *lista, const void *dato, unsigned tamDato)
{
    tNodo *nodoNuevo;

    while (*lista) {

        lista = &(*lista)->sig;
    }

    nodoNuevo = (tNodo*)malloc(sizeof(tNodo));
    if(!nodoNuevo) {

        return LISTA_SIN_MEM;
    }

    nodoNuevo->dato = malloc(tamDato);
    if(!nodoNuevo->dato) {

        free(nodoNuevo);
        return LISTA_SIN_MEM;
    }

    memcpy(nodoNuevo->dato, dato, tamDato);
    nodoNuevo->tamDato = tamDato;
    nodoNuevo->sig = NULL;
    *lista = nodoNuevo;

    return LISTA_TODO_OK;
}


int lista_insertar_comienzo(tLista *lista, const void *dato, unsigned tamDato)
{
    tNodo *nodoNuevo = (tNodo*)malloc(sizeof(tNodo));
    if(!nodoNuevo) {

        return LISTA_SIN_MEM;
    }

    nodoNuevo->dato = malloc(tamDato);
    if(!nodoNuevo->dato) {

        free(nodoNuevo);
        return LISTA_SIN_MEM;
    }

    memcpy(nodoNuevo->dato, dato, tamDato);
    nodoNuevo->tamDato = tamDato;
    nodoNuevo->sig = *lista;
    *lista = nodoNuevo;

    return LISTA_TODO_OK;
}


void lista_vaciar(tLista *lista)
{
    while (*lista) {

        tNodo *elim = *lista;
        *lista = elim->sig;
        free(elim->dato);
        free(elim);
    }
}


int lista_llena(const tLista *lista, unsigned tamDato)
{
    tNodo *nodoNuevo = (tNodo*)malloc(sizeof(tNodo));
    void *dato = malloc(tamDato);
    free(nodoNuevo);
    free(dato);

    return nodoNuevo == NULL || dato == NULL ? LISTA_LLENA : LISTA_TODO_OK;
}


int lista_vacia(const tLista *lista)
{
    return *lista == NULL ? LISTA_VACIA : LISTA_TODO_OK;
}


int lista_sacar_primero(tLista *lista, void *dato, unsigned tamDato)
{
    tNodo *elim;
    unsigned tamDatoEnLista;

    if (*lista == NULL) {

        return LISTA_VACIA;
    }

    memcpy(dato, (*lista)->dato, MIN(tamDato, (*lista)->tamDato));
    tamDatoEnLista = (*lista)->tamDato;

    elim = *lista;
    *lista = elim->sig;
    free(elim->dato);
    free(elim);

    return tamDato < tamDatoEnLista ? LISTA_DATO_PARCIAL : LISTA_TODO_OK;
}


int lista_sacar_ultimo(tLista *lista, void *dato, unsigned tamDato)
{
    unsigned tamDatoEnLista;

    if (!*lista) {

        return LISTA_VACIA;
    }

    while ((*lista)->sig) {

        lista = &(*lista)->sig;
    }

    memcpy(dato, (*lista)->dato, MIN(tamDato, (*lista)->tamDato));
    tamDatoEnLista = (*lista)->tamDato;

    free((*lista)->dato);
    free(*lista);
    *lista = NULL;

    return tamDato < tamDatoEnLista ? LISTA_DATO_PARCIAL : LISTA_TODO_OK;
}


int lista_ver_primero(const tLista *lista, void *dato, unsigned tamDato)
{
    if (!*lista) {

        return LISTA_VACIA;
    }

    memcpy(dato, (*lista)->dato, MIN(tamDato, (*lista)->tamDato));

    return tamDato < (*lista)->tamDato ? LISTA_DATO_PARCIAL : LISTA_TODO_OK;
}


int lista_ver_ultimo(const tLista *lista, void *dato, unsigned tamDato)
{
    if (!*lista) {

        return LISTA_VACIA;
    }

    while ((*lista)->sig) {

        lista = &(*lista)->sig;
    }

    memcpy(dato, (*lista)->dato, MIN(tamDato, (*lista)->tamDato));

    return tamDato < (*lista)->tamDato ? LISTA_DATO_PARCIAL : LISTA_TODO_OK;
}


int lista_insertar_en_orden(tLista *lista, const void *dato, unsigned tamDato, int modo, tCmp cmp)
{
    tNodo *nodoNuevo;
    int comp;

    while (*lista && (comp = cmp(dato, (*lista)->dato)) > 0) {

        lista = &(*lista)->sig;
    }

    if (*lista && comp == 0 && modo == INSERTAR_SIN_DUP) {

        return LISTA_DUPLICADO;
    }

    nodoNuevo = (tNodo*)malloc(sizeof(tNodo));
    if (!nodoNuevo) {

        return LISTA_SIN_MEM;
    }

    nodoNuevo->dato = malloc(tamDato);
    if (!nodoNuevo->dato) {

        free(nodoNuevo);
        return LISTA_SIN_MEM;
    }

    memcpy(nodoNuevo->dato, dato, tamDato);
    nodoNuevo->tamDato = tamDato;

    nodoNuevo->sig = *lista;
    *lista = nodoNuevo;

    return LISTA_TODO_OK;
}










