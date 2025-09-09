#ifndef COMUN_H_INCLUDED
#define COMUN_H_INCLUDED

//MACRO FUNCIONES
#define ES_DIGITO(x)((x) >= '0' && (x) <= '9')
#define ES_LETRA(x)(((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z'))

//MACROS
#define PIXELES_TILE 48
#define MARGEN_VENTANA 64


typedef struct sNodo{
    void *dato;
    unsigned tamDato;
    struct sNodo *sig;
}tNodo;

#endif // COMUN_H_INCLUDED
