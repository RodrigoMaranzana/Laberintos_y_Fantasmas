#ifndef ESCENARIO_H_INCLUDED
#define ESCENARIO_H_INCLUDED

typedef enum{
    ESCENARIO_PARED,
    ESCENARIO_PISO,
    ESCENARIO_PUERTA_ENTRADA,
    ESCENARIO_PUERTA_SALIDA,
    ESCENARIO_CANTIDAD
}eTiposEscenario;

/// IDEA
typedef struct{
    eTiposEscenario tipo;
    /// Puntero a funcion con accion sobre el jugador;
}tPuerta;

#endif // ESCENARIO_H_INCLUDED
