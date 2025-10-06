#ifndef ARBOL_H_INCLUDED
#define ARBOL_H_INCLUDED

#include <stdio.h>

typedef enum {
    ARBOL_BD_TODO_OK,
    ARBOL_SIN_MEM,
    ARBOL_DATO_DUP,
    ARBOL_NO_ENCONTRADO,
    ARBOL_NO_INICIALIZADO,
    ARBOL_ERR_ARCH,
} eArbolRet;

typedef struct sNodoArbol {
    void *dato;
    unsigned tamDato;
    struct sNodoArbol *izq;
    struct sNodoArbol *der;
}tNodoArbol;

typedef tNodoArbol *tArbol;
typedef int (*tCmp)(const void *a, const void *b);
typedef void (*tAccion)(void *elem, void *extra);

void arbol_crear(tArbol *arbol);
int arbol_buscar(const tArbol *arbol, void *dato, unsigned tamDato, tCmp cmp);
int arbol_insertar_rec(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp);
void arbol_recorrer_preorden(const tArbol *arbol, void *extra, tAccion accion);
void arbol_recorrer_posorden(const tArbol *arbol, void *extra, tAccion accion);
int arbol_cargar_de_archivo(FILE *arch, tArbol *arbol, unsigned tamReg, tCmp cmp);
int arbol_escribir_en_arch(FILE *arch, tArbol *arbol);
void arbol_vaciar(tArbol *arbol);

#endif // ARBOL_H_INCLUDED
