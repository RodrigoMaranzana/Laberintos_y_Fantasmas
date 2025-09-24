#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#define INSERTAR_CON_DUP 10
#define INSERTAR_SIN_DUP 11

typedef enum {
    LISTA_TODO_OK,
    LISTA_SIN_MEM,
    LISTA_DUPLICADO,
    LISTA_VACIA,
    LISTA_LLENA,
    LISTA_DATO_PARCIAL,
}eListaRet;

#ifndef T_NODO
#define T_NODO

typedef void (*tAccion)(void *elem, void *extra);
typedef int (*tCmp)(const void *a, const void *b);

typedef struct sNodo {
    void *dato;
    unsigned tamDato;
    struct sNodo *sig;
}tNodo;
#endif
typedef tNodo *tLista;


void lista_crear(tLista *lista);
void lista_recorrer(const tLista *lista, tAccion accion, void *extra);
int lista_insertar_final(tLista *lista, const void *dato, unsigned tamDato);
int lista_insertar_comienzo(tLista *lista, const void *dato, unsigned tamDato);
void lista_vaciar(tLista *lista);
int lista_llena(const tLista *lista, unsigned tamDato);
int lista_vacia(const tLista *lista);
int lista_sacar_primero(tLista *lista, void *dato, unsigned tamDato);
int lista_sacar_ultimo(tLista *lista, void *dato, unsigned tamDato);
int lista_ver_primero(const tLista *lista, void *dato, unsigned tamDato);
int lista_ver_ultimo(const tLista *lista, void *dato, unsigned tamDato);
int lista_insertar_en_orden(tLista *lista, const void *dato, unsigned tamDato, int modo, tCmp cmp);

#endif // LISTA_H_INCLUDED
