#ifndef COLA_H_INCLUDED
#define COLA_H_INCLUDED

typedef enum {
    COLA_TODO_OK,
    COLA_VACIA,
    COLA_LLENA,
    COLA_SIN_MEM
} eColaRet;

#ifndef TNODO_INCLUDED
#define TNODO_INCLUDED
typedef struct sNodo{
    void *dato;
    unsigned tamDato;
    struct sNodo *sig;
}tNodo;
#endif

typedef struct {
    tNodo *pri;
    tNodo *ult;
} tCola;

void cola_crear(tCola *cola);
int cola_encolar(tCola *cola, const void *dato, unsigned tamDato);
int cola_desencolar(tCola *cola, void *dato, unsigned tamDato);
int cola_ver_primero(const tCola *cola, void *dato, unsigned tamDato);
int cola_vacia(const tCola *cola);
int cola_llena(const tCola *cola, unsigned tamDato);
void cola_vaciar(tCola *cola);

#endif // COLA_H_INCLUDED
