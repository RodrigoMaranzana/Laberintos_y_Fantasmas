#ifndef ARBOL_H_INCLUDED
#define ARBOL_H_INCLUDED

typedef enum {
    ARBOL_TODO_OK,
    ARBOL_SIN_MEM,
    ARBOL_DATO_DUP,
    ARBOL_NO_ENCONTRADO
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
int arbol_buscar_y_actualizar(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp);

#endif // ARBOL_H_INCLUDED
