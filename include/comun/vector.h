#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <stdlib.h>

typedef enum {
    VECTOR_TODO_OK,
    VECTOR_VACIA,
    VECTOR_LLENA,
    VECTOR_SIN_MEM,
    VECTOR_ERR_ARCH,
    VECTOR_DUPLICADO,
} eVectorRet;

typedef struct {
    void* vec;
    int ce;
    size_t cap;
    size_t tamElem;
} tVector;

typedef struct {
    tVector* vector;
    void* act;
    void* ult;
    size_t tamElem;
} tVectorIterador;

//PUNTEROS A FUNCION
typedef int (*Cmp)(const void* e1, const void* e2);
typedef void (*Actualizar)(void* actualizado, const void* actualizador);
typedef void (*Accion)(void* elem, void* extra);


//FUNCIONES PRIMITIVAS
int vector_crear(tVector* vector, size_t tamElem);
void vector_vaciar(tVector *vector);
void vector_destruir(tVector* vector);
int vector_cargar_de_archivo(tVector* vector, const char* nombreArch, size_t tamElem);
void vector_recorrer(tVector* vector, Accion accion, void* extra);
void vector_ordenar(tVector* vector, int metodo, Cmp cmp);
int vector_ord_buscar(tVector* vector, void* elem, Cmp cmp);
int vector_ord_buscar_binaria(const tVector* vector, void* elem, Cmp cmp);
int vector_ord_insertar(tVector* vector, void* elem, Cmp cmp, Actualizar actualizar);
int vector_insertar_al_final(tVector* vector, void* elem);
size_t vector_obtener_cantidad_elem(const tVector* vector);

void vector_it_crear(tVectorIterador* it, tVector* vector);
void* vector_it_primero(tVectorIterador* it);
void* vector_it_siguiente(tVectorIterador* it);
int vector_it_fin(tVectorIterador* it);


#endif // VECTOR_H_INCLUDED
