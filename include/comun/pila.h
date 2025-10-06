#ifndef PILA_H_INCLUDED
#define PILA_H_INCLUDED
#include <stdlib.h>

typedef enum {
    PILA_BD_TODO_OK,
    PILA_VACIA,
    PILA_LLENA,
    PILA_SIN_MEM
} ePilaRet;

#ifndef TNODO_INCLUDED
#define TNODO_INCLUDED
typedef struct sNodo{
    void *dato;
    unsigned tamDato;
    struct sNodo *sig;
}tNodo;
#endif

typedef tNodo *tPila;

void pila_crear(tPila *pila);
int pila_llena(const tPila *pila, unsigned tamDato);
int pila_apilar(tPila *pila, const void *dato, unsigned tamDato);
int pila_tope(const tPila *pila, void *dato, unsigned tamDato);
int pila_vacia(const tPila *pila);
int pila_desapilar(tPila *pila, void *dato, unsigned tamDato);
void pila_vaciar(tPila *pila);

#endif // PILA_H_INCLUDED
