#ifndef ARBOL_H_INCLUDED
#define ARBOL_H_INCLUDED

#include <stdio.h>

typedef enum {
    ARBOL_TODO_OK,
    ARBOL_SIN_MEM,
    ARBOL_DATO_DUP,
    ARBOL_NO_ENCONTRADO,
    ARBOL_NO_INICIALIZADO,
    ARBOL_VACIO,
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
typedef void (*tAccionArbol)(void *elem, unsigned tamDato, unsigned nivel, void *extra);
typedef void (*tDestruir)(void *dato);
typedef int (*tEscribir)(void *dato, FILE *arch, void *extra);
typedef unsigned (*tLeer)(void **datoDest, void *datoFuente, int pos, void *extra);

/// REVISAR
typedef int (*tAccionLimite)(void *elem, void *extra);
/// REVISAR

void arbol_crear(tArbol *arbol);
void arbol_vaciar(tArbol *arbol);
int arbol_insertar(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp);
void arbol_recorrer_preorden(const tArbol *arbol, unsigned nivel, void *extra, tAccionArbol accion);
void arbol_recorrer_orden(const tArbol *arbol, unsigned nivel, void *extra, tAccionArbol accion);
void arbol_recorrer_orden_inverso(const tArbol *arbol, unsigned nivel, void *extra, tAccionArbol accion);
void arbol_recorrer_posorden(const tArbol *arbol, unsigned nivel, void *extra, tAccionArbol accion);
int arbol_buscar(const tArbol *arbol, void *dato, unsigned tamDato, tCmp cmp);
int arbol_escribir_arch_bin_orden(const tArbol *arbol, FILE *arch);
int arbol_escribir_arch_bin_orden_con_escritor(const tArbol *arbol, FILE *arch, tEscribir escribir, void *extra);
void arbol_vaciar_destructor(tArbol *arbol, tDestruir destruir);
int arbol_eliminar(tArbol *arbol, void *dato, unsigned tamDato, tCmp cmp);
int arbol_cargar_arch_bin_ordenado(tArbol *arbol, FILE* arch, unsigned tamDato);
int arbol_cargar_datos_ordenados(tArbol *arbol, void *datos, int cantReg, void *extra, tLeer leer);

/// REVISAR
void arbol_recorrer_orden_inverso_con_limite(const tArbol *arbol, void *extra, tAccionLimite accionLimite);
/// REVISAR

#endif // ARBOL_H_INCLUDED















