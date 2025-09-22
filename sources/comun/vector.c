#include "../../include/comun/vector.h"

#include <string.h>
#include <stdio.h>

//MACROS
#define CAP_INI             10
#define FACTOR_INC          2
#define SELECCION           1

void _ordenar_seleccion(tVector* vector, Cmp cmp);


int vector_crear(tVector* vector, size_t tamElem)
{
    vector->ce = 0;

    vector->vec = malloc(CAP_INI * tamElem);
    if (!vector->vec) {
        vector->cap = 0;
        vector->tamElem = 0;

        return VECTOR_SIN_MEM;
    }

    vector->cap = CAP_INI;
    vector->tamElem = tamElem;

    return VECTOR_TODO_OK;
}

void vector_vaciar(tVector *vector)
{
    vector->ce = 0;
    vector->cap = CAP_INI;
    vector->vec = realloc(vector->vec, CAP_INI * vector->tamElem);
}

void vector_destruir(tVector* vector)
{
    free(vector->vec);
    vector->vec = NULL;
    vector->cap = 0;
    vector->ce = 0;
    vector->tamElem = 0;
}

int vector_cargar_de_archivo(tVector* vector, const char* nombreArch, size_t tamElem)
{
    size_t tamArch, cantReg;

    FILE* arch = fopen(nombreArch, "rb");
    if (!arch) {
        return VECTOR_ERR_ARCH;
    }

    fseek(arch, 0, SEEK_END);

    tamArch = ftell(arch);
    cantReg = tamArch / tamElem;

    rewind(arch);

    vector->vec = malloc(tamArch);
    if (!vector->vec) {
        fclose(arch);
        return VECTOR_SIN_MEM;
    }
    vector->cap = cantReg;
    vector->ce = cantReg;
    vector->tamElem = tamElem;

    fread(vector->vec, tamElem, cantReg, arch);

    fclose(arch);

    return VECTOR_TODO_OK;
}

void vector_recorrer(tVector* vector, Accion accion, void* extra)
{
    void* ult = vector->vec + ((vector->ce - 1) * vector->tamElem);
    void* i;

    for (i = vector->vec; i <= ult; i += vector->tamElem) {
        accion(i, extra);
    }
}

void vector_ordenar(tVector* vector, int metodo, Cmp cmp)
{
    switch (metodo) {
//    case BURBUJEO:
//        break;
        case SELECCION:
            _ordenar_seleccion(vector, cmp);
            break;
//    case INSERCION:
//        break;
    }
}

void _ordenar_seleccion(tVector* vector, Cmp cmp)
{
    void *ult, *limiteInf, *menor, *cursor;
    void* aux = malloc(vector->tamElem);
    if (!aux) {

        return;
    }

    ult = vector->vec + (vector->ce - 1) * vector->tamElem;

    for (limiteInf = vector->vec; limiteInf < ult; limiteInf += vector->tamElem) {

        menor = limiteInf;

        for (cursor = limiteInf + vector->tamElem; cursor <= ult; cursor += vector->tamElem) {

            if (cmp(menor, cursor) > 0) {

                menor = cursor;
            }
        }

        memcpy(aux, limiteInf, vector->tamElem);
        memcpy(limiteInf, menor, vector->tamElem);
        memcpy(menor, aux, vector->tamElem);

    }

    free(aux);

}

int vector_ord_buscar(tVector* vector, void* elem, Cmp cmp)
{
    void* actual = vector->vec;
    void* ult = vector->vec + ((vector->ce - 1) * vector->tamElem);

    while (actual <= ult && cmp(elem, actual) > 0) {

        actual += vector->tamElem;
    }

    if (actual <= ult && cmp(elem, actual) == 0) {
        memcpy(elem, actual, vector->tamElem);
        return (actual - vector->vec) / vector->tamElem;

    }

    return -1;
}

int vector_ord_insertar(tVector *vector, void *elem, Cmp cmp, Actualizar actualizar)
{
    void *actual, *ult, *posIns;

    if (vector->ce == vector->cap) {

        size_t capNueva = vector->cap * FACTOR_INC;
        void* vecNuevo = realloc(vector->vec, capNueva * vector->tamElem);
        if (!vecNuevo) {

            return VECTOR_SIN_MEM;
        }

        vector->cap = capNueva;
        vector->vec = vecNuevo;
    }

    actual = vector->vec;
    ult = vector->vec + ((vector->ce - 1) * vector->tamElem);

    while (actual <= ult && cmp(elem, actual) > 0) {

        actual += vector->tamElem;
    }

    if (actual <= ult && cmp(elem, actual) == 0) {

        actualizar(actual, elem);
        return VECTOR_TODO_OK;
    }

    posIns = actual;

    for (actual = ult; actual >= posIns; actual -= vector->tamElem) {

        memcpy(actual + vector->tamElem, actual, vector->tamElem);
    }

    memcpy(posIns, elem, vector->tamElem);
    vector->ce++;

    return VECTOR_TODO_OK;
}


int vector_insertar_al_final(tVector* vector, void* elem)
{
    size_t nuevaCap;
    void *vecNue, *posIns;

    if (vector->ce == vector->cap) {

        nuevaCap = vector->cap * FACTOR_INC;
        vecNue = realloc(vector->vec, nuevaCap * vector->tamElem);
        if (!vecNue) {

            return VECTOR_SIN_MEM;
        }

        vector->vec = vecNue;
        vector->cap = nuevaCap;
    }

    posIns = vector->vec + vector->ce * vector->tamElem;
    memcpy(posIns, elem, vector->tamElem);
    vector->ce++;

    return VECTOR_TODO_OK;
}

void vector_it_crear(tVectorIterador* it, tVector* vector)
{
    it->vector = vector;
    it->act = vector->vec;
    it->tamElem = vector->tamElem;
}

void* vector_it_primero(tVectorIterador* it)
{
    if (it->vector->ce == 0) {

        return NULL;
    }

    it->ult = it->vector->vec + ((it->vector->ce - 1) * it->vector->tamElem);
    it->act = it->vector->vec;

    return it->act;

}

void* vector_it_siguiente(tVectorIterador* it)
{
    it->act += it->tamElem;

    if (it->act > it->ult) {

        return NULL;
    }

    return it->act;
}

int vector_it_fin(tVectorIterador* it)
{
    if (it->act <= it->ult) {

        return 1;
    }

    return 0;
}








