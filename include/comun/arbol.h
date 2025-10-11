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
typedef int (*tAccionLimite)(void *elem, void *extra);
typedef void (*tActualizar)(void* actualizado, const void* actualizador);
typedef void (*tEscribir)(void *dato, FILE *arch);
typedef void (*tDestruir)(void *dato);
typedef int (*tLeer)(void **dato, unsigned *tamDato, FILE *arch);

void arbol_crear(tArbol *arbol);
int arbol_buscar(const tArbol *arbol, void *dato, unsigned tamDato, tCmp cmp);
int arbol_insertar_rec(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp);
int arbol_insertar_rec_con_act(tArbol *arbol, const void *dato, unsigned tamDato, tCmp cmp, tActualizar actualizar);
void arbol_recorrer_orden_inverso(const tArbol *arbol, void *extra, tAccion accion);
void arbol_recorrer_orden_inverso_con_limite(const tArbol *arbol, void *extra, tAccionLimite accionLimite);
void arbol_recorrer_preorden(const tArbol *arbol, void *extra, tAccion accion);
void arbol_recorrer_posorden(const tArbol *arbol, void *extra, tAccion accion);
int arbol_cargar_de_arch(FILE *arch, tArbol *arbol, unsigned tamReg, tCmp cmp);
int arbol_cargar_de_arch_reg_variable(FILE *arch, tArbol *arbol, tLeer leer, tCmp cmp);
int arbol_escribir_en_arch(FILE *arch, tArbol *arbol);
int arbol_escribir_en_arch_reg_variable(FILE *arch, tArbol *arbol, tEscribir escribir);
void arbol_vaciar(tArbol *arbol);
void arbol_vaciar_destructor(tArbol *arbol, tDestruir destruir);
#endif // ARBOL_H_INCLUDED
