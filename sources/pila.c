#include <string.h>
#include "../include/pila.h"
#include "../include/retorno.h"

#define MIN(a,b)((a) < (b) ? (a) : (b))

void pila_crear(tPila *pila)
{
    *pila = NULL;
}

int pila_llena(const tPila *pila, unsigned tamDato)
{
    tNodo *nodoAux = malloc(sizeof(tNodo));
    void *dato = malloc(tamDato);

    free(nodoAux);
    free(dato);

    return nodoAux == NULL || dato == NULL;
}

int pila_apilar(tPila *pila, const void *dato, unsigned tamDato)
{
    tNodo *nodoNuevo = malloc(sizeof(tNodo));
    if (!nodoNuevo) {

        return ERR_SIN_MEMORIA;
    }

    nodoNuevo->dato = malloc(tamDato);
    if (!nodoNuevo->dato) {

        free(nodoNuevo);
        return ERR_SIN_MEMORIA;
    }

    memcpy(nodoNuevo->dato, dato, tamDato);

    nodoNuevo->tamDato = tamDato;
    nodoNuevo->sig = *pila;
    *pila = nodoNuevo;

    return TODO_OK;
}

int pila_tope(const tPila *pila, void *dato, unsigned tamDato)
{
    if (*pila == NULL) {

        return ERR_PILA_VACIA;
    }

    memcpy(dato, (*pila)->dato, MIN(tamDato, (*pila)->tamDato));

    return TODO_OK;
}

int pila_vacia(const tPila *pila)
{
    return *pila == NULL ? ERR_PILA_VACIA : TODO_OK;
}

int pila_desapilar(tPila *pila, void *dato, unsigned tamDato)
{
    tNodo *nodoAux = *pila;

    if (nodoAux == NULL) {

        return ERR_PILA_VACIA;
    }

    *pila = nodoAux->sig;

    memcpy(dato, nodoAux->dato, MIN(tamDato, nodoAux->tamDato));

    free(nodoAux->dato);
    free(nodoAux);

    return TODO_OK;
}

void pila_vaciar(tPila *pila)
{
    tNodo *nodoAux = *pila;

    while (*pila != NULL) {

        nodoAux = *pila;
        *pila = nodoAux->sig;

        free(nodoAux->dato);
        free(nodoAux);
    }
}






