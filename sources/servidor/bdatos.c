#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../../include/servidor/bdatos.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/arbol.h"
#include "../../include/comun/lista.h"

/// Salto de linea omitido intencionalmente
#define ES_BLANCO(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\v' || (c) == '\f')

typedef enum {
    MODO_INSERCION,
    MODO_ACTUALIZACION
} eModoParseoValores;

typedef enum {
    AGREGAR_OFFSET,
    QUITAR_OFFSET
} eModoOffset;

typedef struct {
    int limite;
    int cantActual;
    tLista* listaOffsets;
} tContextoTop;

/// DEBUG
typedef struct {
    int lecturasRegistros;
    int accesosAleatoriosDisco;
    double tiempoMs;
} tMetricas;

tMetricas gMetricas;
/// DEBUG


typedef int (*tParsear)(tSecuencia *secuencia, void *salida, unsigned tamSalida);

static int _bdatos_cerrar_tabla(tBDatos *bDatos);
static int _bdatos_insertar(tBDatos *bDatos);
static int _bdatos_seleccionar(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos);
static int _bdatos_seleccionar_por_pk(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos, tCampo *campo, int valor, char* buffer);
static int _bdatos_seleccionar_por_is(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos, tCampo *campo, int valor, char* buffer);
static int _bdatos_seleccionar_por_escaneo(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos, tCampo *campo, int valor, char* buffer, eSimbolo operador);
static int _bdatos_actualizar(tBDatos *bDatos);
static int _bdatos_tabla_existe(const char *nombreTabla);
static int _bdatos_crear_tabla(tBDatos *bDatos, const char *nombreTabla);
static int _bdatos_buscar_campo(const tEncabezado *encabezado, tCampo *campoEncontrado, const char *nombreCampoLeido);
static int _bdatos_manejar_apertura_tabla(tBDatos *bDatos, const char *nombreTabla);
static int _bdatos_cmp_indice(const void *a, const void *b);
static int _bdatos_cmp_indice_secundario(const void *a, const void *b);
static int _bdatos_parsear(tSecuencia *secuencia, tParsear parsear, void *salida, unsigned tamSalida);
static int _bdatos_parsear_identificador(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_simbolo(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_caracter(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_numeros(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_texto(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static void _bdatos_crear_secuencia(tSecuencia *secuencia, const char *buffer);
static int _bdatos_parsear_declaracion_campos(tSecuencia *secuencia, tVector *vecCampos);
static int _bdatos_leer_campo_tipo(tSecuencia *secuencia, tCampo *campo);
static int _bdatos_parsear_valores(tSecuencia *secuencia, tEncabezado *encabezado, eModoParseoValores modo, tDatoParseado *datosParseados, int *cantParseados);
static eSimbolo _bdatos_comparar_simbolo(const char* simbolo);
static int _bdatos_leer_indice_secundario(void **indiceIS, unsigned *tamindiceIS, FILE *arch);
static void _bdatos_escribir_indice_secundario(void *dato, FILE *arch);
static int _bdatos_cmp_offset(const void *a, const void *b);
static void _bdatos_destruir_indice_secundario(void *dato);
static int _bdatos_accion_limite_top(void *elem, void *extra);
static int _bdatos_seleccionar_por_top(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos, tCampo *campo, int limite);
static void _bdatos_actualizar_is(tArbol* arbolIS, int valorClave, long offset, eModoOffset modo);
static tCampo* _bdatos_encabezado_buscar_campo_por_tipo(tEncabezado* encabezado, eSimbolo tipoBuscado);
static int _bdatos_cmp_campo_por_nombre(const void* a, const void* b);
static void _bdatos_accion_escribir_campo(void* elem, void* extra);


/// DEBUG
void _bdatos_reset_metricas() {
    memset(&gMetricas, 0, sizeof(tMetricas));
}
/// DEBUG


static void _bdatos_escribir_indice_secundario(void *dato, FILE *arch)
{
    long *offset;
    tListaIterador listaIt;
    tIndiceIS *indice = (tIndiceIS*)dato;

    if (!dato) {
        return;
    }

    lista_it_crear(&indice->listaOffsets, &listaIt);

    fwrite(indice->clave, sizeof(indice->clave), 1, arch);
    fwrite(&indice->cantOffsets, sizeof(indice->cantOffsets), 1, arch);

    offset = (long*)lista_it_primero(&listaIt);
    while (offset) {
        fwrite(offset, sizeof(long), 1, arch);
        offset = (long*)lista_it_siguiente(&listaIt);
    }
}


static int _bdatos_cerrar_tabla(tBDatos *bDatos)
{
    int ret = BD_TODO_OK;
    FILE *archIdx;
    tCampo* campoIS;
    char nombreIdx[TAM_NOMBRE_ARCH];

    if (!bDatos->tablaAbierta.arch) return BD_TODO_OK;

    printf("Cerrando y guardando tabla: %s\n", bDatos->tablaAbierta.encabezado.nombreTabla);

    sprintf(nombreIdx, "%s.idx", bDatos->tablaAbierta.encabezado.nombreTabla);
    archIdx = fopen(nombreIdx, "wb");

    if (archIdx) {
        if (arbol_escribir_en_arch(archIdx, &bDatos->tablaAbierta.arbolPK) != ARBOL_TODO_OK) {
            ret = BD_ERROR_ESCRITURA;
        }
        fclose(archIdx);
    } else {
        ret = BD_ERROR_ESCRITURA;
    }

    if (ret == BD_TODO_OK && (campoIS = _bdatos_encabezado_buscar_campo_por_tipo(&bDatos->tablaAbierta.encabezado, IS))) {

        char nombreIdxS[TAM_NOMBRE_ARCH + TAM_IDENTIFICADOR + 2];

        sprintf(nombreIdxS, "%s_%s.idxs", bDatos->tablaAbierta.encabezado.nombreTabla, campoIS->nombre);
        archIdx = fopen(nombreIdxS, "wb");
        if (archIdx) {

            if (arbol_escribir_en_arch_con_escritor(archIdx, &bDatos->tablaAbierta.arbolIS, _bdatos_escribir_indice_secundario) != ARBOL_TODO_OK) {
                ret = BD_ERROR_ESCRITURA;
            }
            fclose(archIdx);
        } else {
            ret = BD_ERROR_ESCRITURA;
        }
    }

    if (ret == BD_TODO_OK) {

        rewind(bDatos->tablaAbierta.arch);

        if (fwrite(&bDatos->tablaAbierta.encabezado, sizeof(tEncabezado) - sizeof(tVector), 1, bDatos->tablaAbierta.arch) != 1) {
            ret = BD_ERROR_ESCRITURA;
        }

        if (ret == BD_TODO_OK) {
             vector_recorrer(&bDatos->tablaAbierta.encabezado.vecCampos, _bdatos_accion_escribir_campo, bDatos->tablaAbierta.arch);
        }
    }

    vector_destruir(&bDatos->tablaAbierta.encabezado.vecCampos);
    arbol_vaciar(&bDatos->tablaAbierta.arbolPK);
    arbol_vaciar_destructor(&bDatos->tablaAbierta.arbolIS, _bdatos_destruir_indice_secundario);
    fclose(bDatos->tablaAbierta.arch);
    memset(&bDatos->tablaAbierta, 0, sizeof(tTabla));

    return ret;
}

static void _bdatos_accion_escribir_campo(void* elem, void* extra)
{
    fwrite(elem, sizeof(tCampo), 1, (FILE*)extra);
}

static void _bdatos_destruir_indice_secundario(void *dato)
{
    tIndiceIS *indiceIS = (tIndiceIS*)dato;

    lista_vaciar(&indiceIS->listaOffsets);
}

static int _bdatos_parsear(tSecuencia *secuencia, tParsear parsear, void *salida, unsigned tamSalida)
{
    while (*secuencia->cursor && ES_BLANCO(*secuencia->cursor)) {
        secuencia->cursor++;
    }

    if (!*secuencia->cursor || secuencia->finSec) {
       return BD_ERROR_SINTAXIS;
    }

    return parsear(secuencia, salida, tamSalida);
}


static int _bdatos_parsear_identificador(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    char *identificador = (char*)salida;
    char *cursor = identificador;

    if (!ES_LETRA(*secuencia->cursor)) return BD_ERROR_SINTAXIS;

    while (*secuencia->cursor && ES_LETRA(*secuencia->cursor) && (cursor - identificador) < (tamSalida - 1)) {
        *cursor++ = *secuencia->cursor++;
    }

    *cursor = '\0';

    return BD_TODO_OK;
}

static int _bdatos_parsear_simbolo(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    eSimbolo *simbolo = (eSimbolo*)salida;
    char simboloBuffer[TAM_SIMBOLO] = {0};
    char *cursor = simboloBuffer;

    if (tamSalida != sizeof(eSimbolo)) return BD_ERROR_PARAMETRO;

    while (*secuencia->cursor && ES_LETRA(*secuencia->cursor) && (cursor - simboloBuffer) < (TAM_SIMBOLO - 1)) {
        *cursor++ = *secuencia->cursor++;
    }

    *cursor = '\0';

    *simbolo = _bdatos_comparar_simbolo(simboloBuffer);
    if (*simbolo == DESCONOCIDO) {
        return BD_ERROR_SINTAXIS;
    }

    return BD_TODO_OK;
}

static int _bdatos_parsear_caracter(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    char *caracter = (char*)salida;

    if (tamSalida != sizeof(char)) return BD_ERROR_PARAMETRO;

    if (*secuencia->cursor) {
        *caracter = *secuencia->cursor;
    } else {
        return BD_ERROR_SINTAXIS;
    }

    secuencia->cursor++;
    return BD_TODO_OK;
}

static int _bdatos_parsear_numeros(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    int *numero = (int*)salida;
    char numBuffer[TAM_INT_TEXTO] = {0};
    char *cursorBuffer = numBuffer;

    if (tamSalida != sizeof(int)) return BD_ERROR_PARAMETRO;

    if (*secuencia->cursor == '-' || *secuencia->cursor == '+') {
        *cursorBuffer++ = *secuencia->cursor++;
    }

    if (!ES_DIGITO(*secuencia->cursor)) return BD_ERROR_SINTAXIS;

    while (*secuencia->cursor && ES_DIGITO(*secuencia->cursor) && (cursorBuffer - numBuffer) < (TAM_INT_TEXTO - 1)) {
        *cursorBuffer++ = *secuencia->cursor++;
    }

    if (*secuencia->cursor && !ES_BLANCO(*secuencia->cursor) && *secuencia->cursor != ',' && *secuencia->cursor != ')') return BD_ERROR_SINTAXIS;

    *cursorBuffer = '\0';
    if (sscanf(numBuffer, "%d", numero) != 1) {
        return BD_ERROR_SINTAXIS;
    }

    return BD_TODO_OK;
}


static int _bdatos_parsear_texto(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    char *texto = (char*)salida;
    char *cursor = texto;

    while (*secuencia->cursor && *secuencia->cursor != ',' && *secuencia->cursor != ')' && (cursor - texto) < (tamSalida - 1)) {
        *cursor++ = *secuencia->cursor++;
    }

    *cursor = '\0';

    return BD_TODO_OK;
}

static int _bdatos_leer_campo_tipo(tSecuencia *secuencia, tCampo *campo)
{
    char caracter;
    eSimbolo proximoSimbolo;
    if (_bdatos_parsear(secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo)) != BD_TODO_OK) {
        return BD_ERROR_SINTAXIS;
    }

    if (proximoSimbolo == ENTERO) {
        campo->tam = sizeof(int);
        campo->tipo = TIPO_ENTERO;
    } else if (proximoSimbolo == TEXTO) {
        int numero;
        if (_bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != '(') return BD_ERROR_SINTAXIS;
        if (_bdatos_parsear(secuencia, _bdatos_parsear_numeros, &numero, sizeof(int)) != BD_TODO_OK) return BD_ERROR_SINTAXIS;
        if (numero <= 0 || numero > TAM_MAX_TIPO_CHAR) return BD_ERROR_TAM_TEXTO;
        if (_bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != ')') return BD_ERROR_SINTAXIS;
        campo->tipo = TIPO_TEXTO;
        campo->tam = numero;
    } else {
        return BD_ERROR_SINTAXIS;
    }

    return BD_TODO_OK;
}

static int _bdatos_leer_encabezado(FILE* arch, tEncabezado *encabezado)
{
    if (!arch) return BD_ERROR_ARCHIVO;

    fseek(arch, 0, SEEK_SET);
    if (fread(encabezado, sizeof(tEncabezado) - sizeof(tVector), 1, arch) != 1) return BD_ERROR_LECTURA;
    if (vector_crear(&encabezado->vecCampos, sizeof(tCampo)) != VECTOR_TODO_OK) return BD_ERROR_SIN_MEMO;

    tCampo campo_leido;
    for (unsigned i = 0; i < encabezado->cantCampos; i++) {

        if (fread(&campo_leido, sizeof(tCampo), 1, arch) != 1) {
            vector_destruir(&encabezado->vecCampos);
            return BD_ERROR_LECTURA;
        }
        vector_ord_insertar(&encabezado->vecCampos, &campo_leido, _bdatos_cmp_campo_por_nombre, NULL);
    }

    return BD_TODO_OK;
}

static void _bdatos_crear_secuencia(tSecuencia *secuencia, const char *buffer)
{
    secuencia->cursor = buffer;
    secuencia->finSec = 0;
}

static eSimbolo _bdatos_comparar_simbolo(const char* simbolo)
{
    if (strcmp(simbolo, "CREAR") == 0) {
        return CREAR;
    } else if (strcmp(simbolo, "INSERTAR") == 0) {
        return INSERTAR;
    } else if (strcmp(simbolo, "ACTUALIZAR") == 0) {
        return ACTUALIZAR;
    } else if (strcmp(simbolo, "SELECCIONAR") == 0) {
        return SELECCIONAR;
    } else if (strcmp(simbolo, "IGUAL") == 0) {
        return IGUAL;
    } else if (strcmp(simbolo, "MAYOR") == 0) {
        return MAYOR;
    } else if (strcmp(simbolo, "MENOR") == 0) {
        return MENOR;
    } else if (strcmp(simbolo, "DISTINTO") == 0) {
        return DISTINTO;
    } else if (strcmp(simbolo, "TOP") == 0) {
        return TOP;
    } else if (strcmp(simbolo, "DONDE") == 0) {
        return DONDE;
    } else if (strcmp(simbolo, "ENTERO") == 0) {
        return ENTERO;
    } else if (strcmp(simbolo, "TEXTO") == 0) {
        return TEXTO;
    } else if (strcmp(simbolo, "PK") == 0) {
        return PK;
    } else if (strcmp(simbolo, "AI") == 0) {
        return AI;
    } else if (strcmp(simbolo, "IS") == 0) {
        return IS;
    } else {
        return DESCONOCIDO;
    }
}

static int _bdatos_tabla_existe(const char *nombreTabla)
{
    char nombreDat[TAM_NOMBRE_ARCH];
    FILE *arch;

    sprintf(nombreDat,"%s.dat", nombreTabla);

    arch = fopen(nombreDat, "rb");

    if (arch) {
        fclose(arch);
        return 1;
    }

    return 0;
}

int bdatos_procesar_solcitud(tBDatos *bDatos, const char *solicitud, tLista *listaDatos, int *cantRegistrosDatos, int *tamRegistroDatos)
{
    int ret;
    eSimbolo simboloComando;
    char nombreTabla[TAM_IDENTIFICADOR];

    /// DEBUG
    LARGE_INTEGER frecuencia;
    LARGE_INTEGER inicio, fin;
    QueryPerformanceFrequency(&frecuencia);
    QueryPerformanceCounter(&inicio);
    /// DEBUG

    _bdatos_crear_secuencia(&bDatos->secuencia, solicitud);

    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &simboloComando, sizeof(eSimbolo))) != BD_TODO_OK) return ret;
    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_identificador, nombreTabla, TAM_IDENTIFICADOR)) != BD_TODO_OK) return ret;

    if (simboloComando == CREAR) {

        if (_bdatos_tabla_existe(nombreTabla)) {
            return BD_ERROR_TABLA_EXISTE;
        }

        if (bDatos->tablaAbierta.arch) {
            _bdatos_cerrar_tabla(bDatos);
        }
        ret = _bdatos_crear_tabla(bDatos, nombreTabla);

    } else {
        if ((ret = _bdatos_manejar_apertura_tabla(bDatos, nombreTabla)) != BD_TODO_OK) {
             return ret;
        }

        switch (simboloComando) {
            case INSERTAR:
                ret = _bdatos_insertar(bDatos);
                break;
            case SELECCIONAR: {
                ret = _bdatos_seleccionar(bDatos, listaDatos, cantRegistrosDatos);
                *tamRegistroDatos = (ret == BD_DATOS_OBTENIDOS) ? bDatos->tablaAbierta.encabezado.tamRegistro : 0;
                break;
            }
            case ACTUALIZAR:
                ret = _bdatos_actualizar(bDatos);
                break;
            default:
                ret = BD_ERROR_COMANDO;
                break;
        }
    }

    /// DEBUG
    QueryPerformanceCounter(&fin);
    if (frecuencia.QuadPart != 0) {
        gMetricas.tiempoMs = ((double)(fin.QuadPart - inicio.QuadPart) / frecuencia.QuadPart) * 1000.0;
    } else {
        gMetricas.tiempoMs = 0.0;
    }
    printf("-- La operacion ha demorado: %f milisegundos --\n", gMetricas.tiempoMs);
    /// DEBUG

    return ret;
}

static int _bdatos_cmp_indice_secundario(const void *a, const void *b)
{
    tIndiceIS *indiceA = (tIndiceIS*)a;
    tIndiceIS *indiceB = (tIndiceIS*)b;

    return strcmp(indiceA->clave, indiceB->clave);
}

static int _bdatos_leer_indice_secundario(void **indiceIS, unsigned *tamindiceIS, FILE *arch)
{
    int i;
    tIndiceIS *nuevoIndice = malloc(sizeof(tIndiceIS));
    if (!nuevoIndice) {
        return 0;
    }

    if (fread(&nuevoIndice->clave, sizeof(nuevoIndice->clave), 1, arch) != 1) {
        free(nuevoIndice);
        return 0;
    }

    if (fread(&nuevoIndice->cantOffsets, sizeof(unsigned), 1, arch) != 1) {
        free(nuevoIndice);
        return 0;
    }

    lista_crear(&nuevoIndice->listaOffsets);

    for (i = 0; i < nuevoIndice->cantOffsets; i++) {
        long offset;
        if (fread(&offset, sizeof(long), 1, arch) != 1) {
            lista_vaciar(&nuevoIndice->listaOffsets);
            free(nuevoIndice);
            return 0;
        }
        lista_insertar_final(&nuevoIndice->listaOffsets, &offset, sizeof(long));
    }

    *indiceIS = nuevoIndice;
    *tamindiceIS = sizeof(tIndiceIS);

    return 1;
}

static int _bdatos_manejar_apertura_tabla(tBDatos *bDatos, const char *nombreTabla)
{
    int ret;
    tArbol arbolPK, arbolIS;
    FILE *archDat, *archIdx;
    tEncabezado encabezado;
    tCampo* campoIS;
    char nombreArch[TAM_NOMBRE_ARCH];

    if (bDatos->tablaAbierta.arch && strcmp(bDatos->tablaAbierta.encabezado.nombreTabla, nombreTabla) == 0) return BD_TODO_OK;

    sprintf(nombreArch, "%s.dat", nombreTabla);
    archDat = fopen(nombreArch, "r+b");
    if (!archDat) return BD_ERROR_TABLA_NO_EXISTE;

    if (_bdatos_leer_encabezado(archDat, &encabezado) != BD_TODO_OK) {
        fclose(archDat);
        return BD_ERROR_ENCABEZADO;
    }

    if (strcmp(encabezado.nombreTabla, nombreTabla) != 0) {
        fclose(archDat);
        vector_destruir(&encabezado.vecCampos);
        return BD_ERROR_ENCABEZADO;
    }

    arbol_crear(&arbolPK);
    sprintf(nombreArch, "%s.idx", nombreTabla);
    archIdx = fopen(nombreArch, "rb");
    if (!archIdx) {
        fclose(archDat);
        vector_destruir(&encabezado.vecCampos);
        return BD_ERROR_ARCHIVO;
    }

    ret = arbol_cargar_de_arch(archIdx, &arbolPK, encabezado.tamRegIdx, _bdatos_cmp_indice);
    fclose(archIdx);
    if (ret != ARBOL_TODO_OK) {
        fclose(archDat);
        vector_destruir(&encabezado.vecCampos);
        return BD_ERROR_ARCHIVO;
    }

    arbol_crear(&arbolIS);
    campoIS = _bdatos_encabezado_buscar_campo_por_tipo(&encabezado, IS);
    if (campoIS) {
        FILE* archIdxS;
        char nombreIdxS[TAM_NOMBRE_ARCH + TAM_IDENTIFICADOR + 2];
        sprintf(nombreIdxS, "%s_%s.idxs", encabezado.nombreTabla, campoIS->nombre);

        archIdxS = fopen(nombreIdxS, "rb");
        if (!archIdxS) {
            fclose(archDat);
            vector_destruir(&encabezado.vecCampos);
            arbol_vaciar(&arbolPK);
            return BD_ERROR_ARCHIVO;
        }

        ret = arbol_cargar_de_arch_con_lector(archIdxS, &arbolIS, _bdatos_leer_indice_secundario, _bdatos_cmp_indice_secundario);
        fclose(archIdxS);
        if (ret != ARBOL_TODO_OK) {
            fclose(archDat);
            vector_destruir(&encabezado.vecCampos);
            arbol_vaciar(&arbolPK);
            arbol_vaciar_destructor(&bDatos->tablaAbierta.arbolIS, _bdatos_destruir_indice_secundario);
            return BD_ERROR_ARCHIVO;
        }
    }

    if (bDatos->tablaAbierta.arch) {
        if ((ret = _bdatos_cerrar_tabla(bDatos)) != BD_TODO_OK) {
            fclose(archDat);
            vector_destruir(&encabezado.vecCampos);
            arbol_vaciar(&arbolPK);
            arbol_vaciar_destructor(&bDatos->tablaAbierta.arbolIS, _bdatos_destruir_indice_secundario);
            return ret;
        }
    }

    bDatos->tablaAbierta.arbolPK = arbolPK;
    bDatos->tablaAbierta.arbolIS = arbolIS;
    bDatos->tablaAbierta.arch = archDat;
    memcpy(&bDatos->tablaAbierta.encabezado, &encabezado, sizeof(tEncabezado));

    printf("Tabla '%s' abierta correctamente.\n", bDatos->tablaAbierta.encabezado.nombreTabla);

    return BD_TODO_OK;
}

/// CREAR jugadores (username TEXTO(16) PK, record ENTERO IS, cantPartidas ENTERO)
static int _bdatos_crear_tabla(tBDatos *bDatos, const char *nombreTabla)
{
    int ret;
    FILE *archIdx;
    tCampo *campoIS, *campoActual;
    tEncabezado encabezado = {0};
    tVectorIterador vecIt;
    char caracter, nombreDat[TAM_NOMBRE_ARCH], nombreIdx[TAM_NOMBRE_ARCH];

    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != '(') return BD_ERROR_SINTAXIS;
    if (vector_crear(&encabezado.vecCampos, sizeof(tCampo)) != VECTOR_TODO_OK) return BD_ERROR_SIN_MEMO;
    if ((ret = _bdatos_parsear_declaracion_campos(&bDatos->secuencia, &encabezado.vecCampos)) != BD_TODO_OK) return ret;

    sprintf(nombreDat, "%s.dat", nombreTabla);
    sprintf(nombreIdx, "%s.idx", nombreTabla);

    bDatos->tablaAbierta.arch = fopen(nombreDat, "w+b");
    if (!bDatos->tablaAbierta.arch) {
        vector_destruir(&encabezado.vecCampos);
        return BD_ERROR_ARCHIVO;
    }

    strncpy(encabezado.nombreTabla, nombreTabla, TAM_IDENTIFICADOR - 1);
    encabezado.cantCampos = vector_obtener_cantidad_elem(&encabezado.vecCampos);
    vector_it_crear(&vecIt, &encabezado.vecCampos);
    campoActual = vector_it_primero(&vecIt);
    while (campoActual) {
        encabezado.tamRegistro += campoActual->tam;
        if (campoActual->esAI) {
            encabezado.proximoAI = 1;
        }
        campoActual = vector_it_siguiente(&vecIt);
    }
    encabezado.tamRegIdx = sizeof(tIndicePK);

    rewind(bDatos->tablaAbierta.arch);
    if (fwrite(&encabezado, sizeof(tEncabezado) - sizeof(tVector), 1, bDatos->tablaAbierta.arch) != 1) {
        fclose(bDatos->tablaAbierta.arch);
        bDatos->tablaAbierta.arch = NULL;
        remove(nombreDat);
        return BD_ERROR_ESCRITURA;
    }
    vector_recorrer(&encabezado.vecCampos, _bdatos_accion_escribir_campo, bDatos->tablaAbierta.arch);
    fflush(bDatos->tablaAbierta.arch);

    archIdx = fopen(nombreIdx, "w+b");
    if (!archIdx) {
        vector_destruir(&encabezado.vecCampos);
        fclose(bDatos->tablaAbierta.arch);
        bDatos->tablaAbierta.arch = NULL;
        remove(nombreDat);
        return BD_ERROR_ARCHIVO;
    }
    fclose(archIdx);

    campoIS = _bdatos_encabezado_buscar_campo_por_tipo(&encabezado, IS);
    if (campoIS) {
        FILE *archIdxS;
        char nombreIdxS[TAM_NOMBRE_ARCH + TAM_IDENTIFICADOR + 2];
        sprintf(nombreIdxS, "%s_%s.idxs", nombreTabla, campoIS->nombre);
        archIdxS = fopen(nombreIdxS, "w+b");
        if (!archIdxS) {
            fclose(bDatos->tablaAbierta.arch);
            bDatos->tablaAbierta.arch = NULL;
            remove(nombreDat);
            remove(nombreIdx);
            return BD_ERROR_ARCHIVO;
        }
        fclose(archIdxS);
    }

    bDatos->tablaAbierta.encabezado = encabezado;
    arbol_crear(&bDatos->tablaAbierta.arbolPK);
    arbol_crear(&bDatos->tablaAbierta.arbolIS);
    return BD_TODO_OK;
}

static int _bdatos_cmp_indice(const void *a, const void *b)
{
    tIndicePK *indiceA = (tIndicePK*)a;
    tIndicePK *indiceB = (tIndicePK*)b;

    return strcmp(indiceA->clave, indiceB->clave);
}

static int _bdatos_buscar_campo(const tEncabezado *encabezado, tCampo *campoEncontrado, const char *nombreCampoLeido)
{
    strncpy(campoEncontrado->nombre, nombreCampoLeido, TAM_IDENTIFICADOR);
    campoEncontrado->nombre[TAM_IDENTIFICADOR - 1] = '\0';

    return vector_ord_buscar_binaria((tVector*)&encabezado->vecCampos, campoEncontrado, _bdatos_cmp_campo_por_nombre) != -1 ? BD_TODO_OK : BD_ERROR_CAMPO_INEXISTENTE;
}

static int _bdatos_cmp_offset(const void *a, const void *b)
{
    return *(long*)a - *(long*)b;
}


static void _bdatos_actualizar_is(tArbol* arbolIS, int valorClave, long offset, eModoOffset modo)
{
    tIndiceIS indice;
    int resultado;

    memset(indice.clave, 0, sizeof(indice.clave));
    sprintf(indice.clave, "%011d", valorClave);

    resultado = arbol_eliminar(arbolIS, &indice, sizeof(tIndiceIS), _bdatos_cmp_indice_secundario);
    if (resultado == ARBOL_NO_ENCONTRADO)
    {
        if (modo == AGREGAR_OFFSET) {

            lista_crear(&indice.listaOffsets);
            lista_insertar_final(&indice.listaOffsets, &offset, sizeof(long));
            indice.cantOffsets = 1;

            arbol_insertar_rec(arbolIS, &indice, sizeof(tIndiceIS), _bdatos_cmp_indice_secundario);
        }
        return;
    }

    if (modo == AGREGAR_OFFSET) {
        lista_insertar_final(&indice.listaOffsets, &offset, sizeof(long));
        indice.cantOffsets++;
    } else {
        lista_eliminar(&indice.listaOffsets, &offset, sizeof(long), _bdatos_cmp_offset);
        indice.cantOffsets--;
    }

    if (lista_vacia(&indice.listaOffsets) != LISTA_VACIA) {
        arbol_insertar_rec(arbolIS, &indice, sizeof(tIndiceIS), _bdatos_cmp_indice_secundario);
    }
}

/// ACTUALIZAR jugadores (puntajeMax 300) DONDE (username IGUAL PEPE)
static int _bdatos_actualizar(tBDatos *bDatos)
{
    int ret, cantParseados = 0, i, cantRegistrosDatos;
    tLista listaDatos;
    eSimbolo simboloConector;
    tDatoParseado datosParseados[MAX_CAMPOS_POR_TABLA];
    char caracter, *registro = NULL, *registroActualizado = NULL;
    tCampo *campoPK = _bdatos_encabezado_buscar_campo_por_tipo(&bDatos->tablaAbierta.encabezado, PK), *campoIS;

    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != '(') return BD_ERROR_SINTAXIS;

    memset(datosParseados, 0, sizeof(datosParseados));
    ret = _bdatos_parsear_valores(&bDatos->secuencia, &bDatos->tablaAbierta.encabezado, MODO_ACTUALIZACION, datosParseados, &cantParseados);
    if (ret != BD_TODO_OK) {
        for (i = 0; i < cantParseados; i++) {
            free(datosParseados[i].valor.dato);
        }
        return ret;
    }

    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &simboloConector, sizeof(eSimbolo))) != BD_TODO_OK || (simboloConector != DONDE)) return BD_ERROR_SINTAXIS;

    lista_crear(&listaDatos);
    _bdatos_seleccionar(bDatos, &listaDatos, &cantRegistrosDatos);

    registro = (char*)malloc(bDatos->tablaAbierta.encabezado.tamRegistro);
    if (!registro || !(registroActualizado = (char*)malloc(bDatos->tablaAbierta.encabezado.tamRegistro))) {
        free(registro);
        free(registroActualizado);
        for (i = 0; i < cantParseados; i++) {
            free(datosParseados[i].valor.dato);
        }
        return BD_ERROR_SIN_MEMO;
    }

    while (lista_sacar_primero(&listaDatos, registro, bDatos->tablaAbierta.encabezado.tamRegistro) != LISTA_VACIA) {

        tIndicePK indicePK;
        long offsetRegistro;
        memset(&indicePK, 0, sizeof(tIndicePK));

        if (campoPK->tipo == TIPO_ENTERO) {
            int valorPK = *(int*)(registro + campoPK->offsetCampo);
            sprintf(indicePK.clave, "%011d", valorPK);
        } else {
            strncpy(indicePK.clave, registro + campoPK->offsetCampo, campoPK->tam);
        }
        arbol_buscar(&bDatos->tablaAbierta.arbolPK, &indicePK, sizeof(tIndicePK), _bdatos_cmp_indice);
        offsetRegistro = indicePK.offset;

        campoIS = _bdatos_encabezado_buscar_campo_por_tipo(&bDatos->tablaAbierta.encabezado, IS);
        if (campoIS) {
            int valorAnteriorIS = *(int*)(registro + campoIS->offsetCampo);
            int valorNuevoIS;
            int actualizado = 0;

            for(i = 0; i < cantParseados && !actualizado; i++) {
                if (strcmp(datosParseados[i].campo.nombre, campoIS->nombre) == 0) {
                    valorNuevoIS = *(int*)datosParseados[i].valor.dato;
                    actualizado = 1;
                }
            }

            if (actualizado && valorAnteriorIS != valorNuevoIS) {
                _bdatos_actualizar_is(&bDatos->tablaAbierta.arbolIS, valorAnteriorIS, offsetRegistro, QUITAR_OFFSET);
                _bdatos_actualizar_is(&bDatos->tablaAbierta.arbolIS, valorNuevoIS, offsetRegistro, AGREGAR_OFFSET);
            }
        }

        memcpy(registroActualizado, registro, bDatos->tablaAbierta.encabezado.tamRegistro);
        for (i = 0; i < cantParseados; i++) {
            memcpy(registroActualizado + datosParseados[i].campo.offsetCampo, datosParseados[i].valor.dato, datosParseados[i].valor.tam);
        }

        fseek(bDatos->tablaAbierta.arch, indicePK.offset, SEEK_SET);
        fwrite(registroActualizado, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch);
    }

    for (i = 0; i < cantParseados; i++) {
        free(datosParseados[i].valor.dato);
    }
    free(registro);
    free(registroActualizado);
    return BD_TODO_OK;
}

/// INSERTAR jugadores (username PEPE, record 15, cantPartidas 5)
static int _bdatos_insertar(tBDatos *bDatos)
{
    long offset;
    int ret, i, cantParseados = 0, valorIS = 0;
    tDatoParseado datosParseados[MAX_CAMPOS_POR_TABLA];
    char caracter, *registro = NULL;
    tIndicePK nuevoIndiceReg;

    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != '(') return BD_ERROR_SINTAXIS;

    memset(datosParseados, 0, sizeof(datosParseados));
    ret = _bdatos_parsear_valores(&bDatos->secuencia, &bDatos->tablaAbierta.encabezado, MODO_INSERCION, datosParseados, &cantParseados);
    if (ret != BD_TODO_OK) {
        for (i = 0; i < cantParseados; i++) {
            free(datosParseados[i].valor.dato);
        }
        return ret;
    }

    registro = (char*)malloc(bDatos->tablaAbierta.encabezado.tamRegistro);
    if (!registro) {
        for (i = 0; i < cantParseados; i++) {
            free(datosParseados[i].valor.dato);
        }
        return BD_ERROR_SIN_MEMO;
    }

    memset(registro, 0, bDatos->tablaAbierta.encabezado.tamRegistro);
    memset(&nuevoIndiceReg, 0, sizeof(tIndicePK));

    for (i = 0; i < cantParseados; i++) {
        memcpy(registro + datosParseados[i].campo.offsetCampo, datosParseados[i].valor.dato, datosParseados[i].valor.tam);
        if (datosParseados[i].campo.esPK) {
            if (datosParseados[i].campo.tipo == TIPO_TEXTO) {
                strncpy(nuevoIndiceReg.clave, (char*)datosParseados[i].valor.dato, sizeof(nuevoIndiceReg.clave) - 1);
                nuevoIndiceReg.clave[sizeof(nuevoIndiceReg.clave) - 1] = '\0';
            } else {
                sprintf(nuevoIndiceReg.clave, "%011d", *(int*)datosParseados[i].valor.dato);
            }
        }else if (datosParseados[i].campo.esIS) {
            valorIS = *(int*)datosParseados[i].valor.dato;
        }
    }

    fseek(bDatos->tablaAbierta.arch, 0, SEEK_END);
    offset = ftell(bDatos->tablaAbierta.arch);

    nuevoIndiceReg.offset = offset;
    if ((ret = arbol_insertar_rec(&bDatos->tablaAbierta.arbolPK, &nuevoIndiceReg, sizeof(tIndicePK), _bdatos_cmp_indice)) != ARBOL_TODO_OK) {
        for (i = 0; i < cantParseados; i++) {
            free(datosParseados[i].valor.dato);
        }
        free(registro);
        return ret == ARBOL_DATO_DUP ? BD_ERROR_DUPLICADO_PK : BD_ERROR_SIN_MEMO;
    }

    if ( _bdatos_encabezado_buscar_campo_por_tipo(&bDatos->tablaAbierta.encabezado, IS)) {
        _bdatos_actualizar_is(&bDatos->tablaAbierta.arbolIS, valorIS, offset, AGREGAR_OFFSET);
    }

    fwrite(registro, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch);
    bDatos->tablaAbierta.encabezado.cantRegistros++;
    if ( _bdatos_encabezado_buscar_campo_por_tipo(&bDatos->tablaAbierta.encabezado, AI)) {
        ++bDatos->tablaAbierta.encabezado.proximoAI;
    }

    for (i = 0; i < cantParseados; i++) {
        free(datosParseados[i].valor.dato);
    }
    free(registro);

    return BD_TODO_OK;
}

/// SELECCIONAR jugadores (ranking TOP 3)
/// SELECCIONAR jugadores (username IGUAL PEPE)
static int _bdatos_seleccionar(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos)
{
    int ret, valor;
    char *buffer = NULL, caracter, nombreCampoLeido[TAM_IDENTIFICADOR] = {0};
    eSimbolo proximoSimbolo;
    tCampo campoEncontrado;

    *cantRegistrosDatos = 0;

    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != '(') return BD_ERROR_SINTAXIS;
    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_identificador, nombreCampoLeido, TAM_IDENTIFICADOR)) != BD_TODO_OK) return ret;
    if ((ret = _bdatos_buscar_campo(&bDatos->tablaAbierta.encabezado, &campoEncontrado, nombreCampoLeido)) != BD_TODO_OK) return ret;
    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo)) != BD_TODO_OK) return BD_ERROR_SINTAXIS;

    lista_crear(listaDatos);

    if (proximoSimbolo == IGUAL || proximoSimbolo == DISTINTO) {

        if (campoEncontrado.tipo == TIPO_ENTERO) {
            if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_numeros, &valor, sizeof(int))) != BD_TODO_OK) return ret;
        } else {
            buffer = malloc(campoEncontrado.tam);
            if (!buffer) return BD_ERROR_SIN_MEMO;
            if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_texto, buffer, campoEncontrado.tam)) != BD_TODO_OK) {
                free(buffer);
                return ret;
            }
        }

        if (proximoSimbolo == IGUAL) {
            if (campoEncontrado.esPK) {
                ret = _bdatos_seleccionar_por_pk(bDatos, listaDatos, cantRegistrosDatos, &campoEncontrado, valor, buffer);
            } else if (campoEncontrado.esIS) {
                ret = _bdatos_seleccionar_por_is(bDatos, listaDatos, cantRegistrosDatos, &campoEncontrado, valor, buffer);
            } else {
                ret = _bdatos_seleccionar_por_escaneo(bDatos, listaDatos, cantRegistrosDatos, &campoEncontrado, valor, buffer, IGUAL);
            }
        } else {
            ret = _bdatos_seleccionar_por_escaneo(bDatos, listaDatos, cantRegistrosDatos, &campoEncontrado, valor, buffer, DISTINTO);
        }

    } else if (proximoSimbolo == TOP) {

        int limite;

        if (!campoEncontrado.esIS) {
            free(buffer);
            return BD_ERROR_NECESITA_INDICE;
        }

        if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_numeros, &limite, sizeof(int))) != BD_TODO_OK) {
            free(buffer);
            return ret;
        }

        ret = _bdatos_seleccionar_por_top(bDatos, listaDatos, cantRegistrosDatos, &campoEncontrado, limite);
    }
    else {
        ret = BD_ERROR_SINTAXIS;
    }

    free(buffer);

    if (ret != BD_TODO_OK) return ret;

    return *cantRegistrosDatos == 0 ? BD_ERROR_SIN_RESULTADOS : BD_DATOS_OBTENIDOS;
}

static int _bdatos_seleccionar_por_top(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos, tCampo *campo, int limite)
{
    tLista listaOffsets;
    tContextoTop contextoTop;
    long *offset;
    tListaIterador listaIt;

    contextoTop.limite = limite;
    contextoTop.cantActual = 0;
    contextoTop.listaOffsets = &listaOffsets;
    lista_crear(contextoTop.listaOffsets);
    arbol_recorrer_orden_inverso_con_limite(&bDatos->tablaAbierta.arbolIS, &contextoTop, _bdatos_accion_limite_top);

    char* registro = (char*)malloc(bDatos->tablaAbierta.encabezado.tamRegistro);
    if (!registro) {
        return BD_ERROR_SIN_MEMO;
    }

    lista_it_crear(&listaOffsets, &listaIt);

    offset = lista_it_primero(&listaIt);
    while (offset) {
        fseek(bDatos->tablaAbierta.arch, *offset, SEEK_SET);
        if (fread(registro, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch) == 1) {
            lista_insertar_final(listaDatos, registro, bDatos->tablaAbierta.encabezado.tamRegistro);
        }
        offset = lista_it_siguiente(&listaIt);
    }

    *cantRegistrosDatos = contextoTop.cantActual;
    free(registro);
    lista_vaciar(contextoTop.listaOffsets);

    return BD_TODO_OK;
}


static int _bdatos_accion_limite_top(void *elem, void *extra)
{
    tIndiceIS *indiceIS = (tIndiceIS*)elem;
    tContextoTop *contextoTop = (tContextoTop*)extra;
    long *offset;
    tListaIterador listaIt;

    if (contextoTop->cantActual >= contextoTop->limite) {
        return 0;
    }

    lista_it_crear(&indiceIS->listaOffsets, &listaIt);

    offset = lista_it_primero(&listaIt);
    while (offset && contextoTop->cantActual < contextoTop->limite) {
        lista_insertar_comienzo(contextoTop->listaOffsets, offset, sizeof(long));
        offset = lista_it_siguiente(&listaIt);
        ++contextoTop->cantActual;
    }

    return contextoTop->cantActual >= contextoTop->limite ? 0 : 1;
}

static int _bdatos_seleccionar_por_pk(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos, tCampo *campo, int valor, char* buffer)
{
    char* registro = (char*)malloc(bDatos->tablaAbierta.encabezado.tamRegistro);
    if (!registro) {
        return BD_ERROR_SIN_MEMO;
    }

    tIndicePK indicePK;
    if (campo->tipo == TIPO_ENTERO) {
        sprintf(indicePK.clave, "%011d", valor);
    } else {
        strncpy(indicePK.clave, buffer, sizeof(indicePK.clave) - 1);
        indicePK.clave[sizeof(indicePK.clave) - 1] = '\0';
    }

    if (arbol_buscar(&bDatos->tablaAbierta.arbolPK, &indicePK, sizeof(tIndicePK), _bdatos_cmp_indice) == ARBOL_TODO_OK) {
        fseek(bDatos->tablaAbierta.arch, indicePK.offset, SEEK_SET);
        fread(registro, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch);
        lista_insertar_final(listaDatos, registro, bDatos->tablaAbierta.encabezado.tamRegistro);
        (*cantRegistrosDatos)++;
    }

    free(registro);
    return BD_TODO_OK;
}

static int _bdatos_seleccionar_por_is(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos, tCampo *campo, int valor, char* buffer)
{
    char* registro = (char*)malloc(bDatos->tablaAbierta.encabezado.tamRegistro);
    if (!registro) {
        return BD_ERROR_SIN_MEMO;
    }

    tIndiceIS indiceIS;
    if (campo->tipo == TIPO_ENTERO) {
        sprintf(indiceIS.clave, "%011d", valor);
    } else {
        strncpy(indiceIS.clave, buffer, sizeof(indiceIS.clave) - 1);
    }

    if (arbol_buscar(&bDatos->tablaAbierta.arbolIS, &indiceIS, sizeof(tIndiceIS), _bdatos_cmp_indice_secundario) == ARBOL_TODO_OK) {
        tListaIterador listaIt;
        long* offset;

        lista_it_crear(&indiceIS.listaOffsets, &listaIt);

        offset = (long*)lista_it_primero(&listaIt);
        while (offset) {
            fseek(bDatos->tablaAbierta.arch, *offset, SEEK_SET);
            fread(registro, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch);
            lista_insertar_final(listaDatos, registro, bDatos->tablaAbierta.encabezado.tamRegistro);
            (*cantRegistrosDatos)++;
            offset = (long*)lista_it_siguiente(&listaIt);
        }
    }

    free(registro);
    return BD_TODO_OK;
}

static int _bdatos_seleccionar_por_escaneo(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos, tCampo *campo, int valor, char* buffer, eSimbolo operador)
{
    char* registro = (char*)malloc(bDatos->tablaAbierta.encabezado.tamRegistro);
    if (!registro) {
        return BD_ERROR_SIN_MEMO;
    }

    fseek(bDatos->tablaAbierta.arch, sizeof(tEncabezado), SEEK_SET);
    while (fread(registro, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch) == 1) {

        int coincidencia = 0;

        if (campo->tipo == TIPO_ENTERO) {
            int valorEnRegistro = *(int*)(registro + campo->offsetCampo);
            if (operador == IGUAL) {
                coincidencia = (valorEnRegistro == valor);
            } else {
                coincidencia = (valorEnRegistro != valor);
            }
        } else {
            int cmp = strncmp(registro + campo->offsetCampo, buffer, campo->tam);
            if (operador == IGUAL) {
                coincidencia = (cmp == 0);
            } else {
                coincidencia = (cmp != 0);
            }
        }

        if (coincidencia) {
            lista_insertar_final(listaDatos, registro, bDatos->tablaAbierta.encabezado.tamRegistro);
            (*cantRegistrosDatos)++;
        }
    }

    free(registro);
    return BD_TODO_OK;
}


static int _bdatos_parsear_declaracion_campos(tSecuencia *secuencia, tVector *vecCampos)
{
    int ret, fin = 0, hayPK = 0;
    char caracter;
    eSimbolo proximoSimbolo;
    unsigned offsetCampo = 0;
    tCampo campoActual;

    do {

        memset(&campoActual, 0, sizeof(tCampo));

        if ((ret = _bdatos_parsear(secuencia, _bdatos_parsear_identificador, campoActual.nombre, TAM_IDENTIFICADOR)) != BD_TODO_OK) {
            return ret;
        }
        if ((ret = _bdatos_leer_campo_tipo(secuencia, &campoActual)) != BD_TODO_OK) {
            return ret;
        }

        campoActual.offsetCampo = offsetCampo;
        offsetCampo += campoActual.tam;

        if(_bdatos_parsear(secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo)) == BD_TODO_OK) {

            if (proximoSimbolo == PK) {
                campoActual.esPK = 1;
                hayPK++;

                if (_bdatos_parsear(secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo)) == BD_TODO_OK) {
                    if (proximoSimbolo == AI) {
                        if (campoActual.tipo != TIPO_ENTERO) return BD_ERROR_AI_NO_ES_ENTERO;
                        campoActual.esAI = 1;
                    } else {
                        return BD_ERROR_SINTAXIS;
                    }
                }
            }else if (proximoSimbolo == IS) {
                campoActual.esIS = 1;
            }else if (proximoSimbolo == AI) {
                return BD_ERROR_AI_NO_ES_PK;
            } else {
                return BD_ERROR_SINTAXIS;
            }
        }

        if (vector_ord_insertar(vecCampos, &campoActual, _bdatos_cmp_campo_por_nombre, NULL) != VECTOR_TODO_OK) return BD_ERROR_SIN_MEMO;
        if ((ret = _bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char))) != BD_TODO_OK) return ret;

        if (caracter == ')') {
            fin = 1;
        }else if (caracter != ',') {
            return  BD_ERROR_SINTAXIS;
        }

        if (hayPK > 1) return BD_ERROR_DEMASIADOS_PK;

    } while (!fin);

    if (hayPK == 0) return BD_ERROR_SIN_PK;

    return BD_TODO_OK;
}


static int _bdatos_parsear_valores(tSecuencia *secuencia, tEncabezado *encabezado, eModoParseoValores modo, tDatoParseado *datosParseados, int *cantParseados)
{
    int retorno, i = 0, j, fin = 0, cantPK = 0;
    char caracter, nombreCampoLeido[TAM_IDENTIFICADOR];
    tCampo *campoAI, campoEncontrado;

    memset(nombreCampoLeido, 0, sizeof(nombreCampoLeido));
    *cantParseados = 0;

    do {

        if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_identificador, nombreCampoLeido, TAM_IDENTIFICADOR)) != BD_TODO_OK) return retorno;

        memset(&campoEncontrado, 0, sizeof(tCampo));
        if ((retorno = _bdatos_buscar_campo(encabezado, &campoEncontrado, nombreCampoLeido)) != BD_TODO_OK) return retorno;

        if ((modo == MODO_ACTUALIZACION && campoEncontrado.esPK) || campoEncontrado.esAI){
            for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
            return campoEncontrado.esAI ? BD_ERROR_CAMPO_ES_AI : BD_ERROR_ACTUALIZAR_PK;
        }

        if (campoEncontrado.esPK) ++cantPK;

        switch (campoEncontrado.tipo) {
            case TIPO_ENTERO: {

                datosParseados[i].valor.dato = malloc(sizeof(int));
                if (!datosParseados[i].valor.dato) {
                    for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
                    return BD_ERROR_SIN_MEMO;
                }

                datosParseados[i].valor.tam = sizeof(int);
                if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_numeros, (int*)datosParseados[i].valor.dato, sizeof(int))) != BD_TODO_OK) {
                    for (j = 0; j <= i; j++) free(datosParseados[j].valor.dato);
                    return retorno;
                }
                break;
            }
            case TIPO_TEXTO: {

                datosParseados[i].valor.dato = malloc(campoEncontrado.tam);
                if (!datosParseados[i].valor.dato) {
                    for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
                    return BD_ERROR_SIN_MEMO;
                }

                memset(datosParseados[i].valor.dato, 0, campoEncontrado.tam);
                datosParseados[i].valor.tam = campoEncontrado.tam;
                if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_texto, (char*)datosParseados[i].valor.dato, campoEncontrado.tam)) != BD_TODO_OK) {
                    for (j = 0; j <= i; j++) free(datosParseados[j].valor.dato);
                    return retorno;
                }
                break;
            }
            default:
                for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
                return BD_ERROR_CAMPO_INVALIDO;
        }

        memcpy(&datosParseados[i].campo, &campoEncontrado, sizeof(tCampo));
        ++i;

        if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char))) != BD_TODO_OK) {
            for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
            return retorno;
        }

        if (caracter == ')') {
            fin = 1;
        } else if (caracter != ',') {
            for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
            return BD_ERROR_SINTAXIS;
        }

    } while (!fin);

    if (modo == MODO_INSERCION && cantPK < 1 && encabezado->proximoAI == 0) {
        for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
        return BD_ERROR_SIN_PK;
    }

    campoAI = _bdatos_encabezado_buscar_campo_por_tipo(encabezado, AI);
    if (campoAI) {

        memcpy(&datosParseados[i].campo, campoAI, sizeof(tCampo));
        datosParseados[i].valor.dato = malloc(sizeof(int));
        if (!datosParseados[i].valor.dato) {
            for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
            return BD_ERROR_SIN_MEMO;
        }

        memset(datosParseados[i].valor.dato, 0, sizeof(int));
        datosParseados[i].valor.tam = sizeof(int);
        *(int*)datosParseados[i].valor.dato = encabezado->proximoAI;
        i++;
    }

    *cantParseados = i;
    return BD_TODO_OK;
}

static int _bdatos_cmp_campo_por_nombre(const void* a, const void* b)
{
    tCampo* campoA = (tCampo*)a;
    tCampo* campoB = (tCampo*)b;

    return strcmp(campoA->nombre, campoB->nombre);
}

static tCampo* _bdatos_encabezado_buscar_campo_por_tipo(tEncabezado* encabezado, eSimbolo tipoBuscado)
{
    tVectorIterador vecIt;
    tCampo* campoActual;

    vector_it_crear(&vecIt, &encabezado->vecCampos);

    campoActual = vector_it_primero(&vecIt);
    while (campoActual) {
        if (tipoBuscado == PK && campoActual->esPK) return campoActual;
        if (tipoBuscado == AI && campoActual->esAI) return campoActual;
        if (tipoBuscado == IS && campoActual->esIS) return campoActual;
        campoActual = vector_it_siguiente(&vecIt);
    }
    return NULL;
}

const char* bdatos_obtener_mensaje(eBDRetorno codigoError)
{
    switch (codigoError) {
        case BD_TODO_OK:                 return "OK";
        case BD_DATOS_OBTENIDOS:         return "Datos obtenidos";
        case BD_ERROR_DUPLICADO_PK:      return "No se permite insertar valores duplicados en una Clave Primaria (PK)";
        case BD_ERROR_SIN_RESULTADOS:    return "No se encontraron coincidencias";
        case BD_ERROR_SINTAXIS:          return "Error de sintaxis en la solicitud";
        case BD_ERROR_COMANDO:           return "Comando desconocido";
        case BD_ERROR_TABLA_EXISTE:      return "La tabla ya existe";
        case BD_ERROR_TABLA_NO_EXISTE:   return "La tabla no existe";
        case BD_ERROR_SIN_PK:            return "La operacion requiere una Clave Primaria (PK)";
        case BD_ERROR_ACTUALIZAR_PK:     return "No se permite actualizar el valor de una Clave Primaria (PK)";
        case BD_ERROR_DEMASIADOS_PK:     return "Solo se permite una Clave Primaria (PK) por tabla";
        case BD_ERROR_DEMASIADOS_AI:     return "Solo se permite un campo Autoincremental (AI) por tabla";
        case BD_ERROR_DEMASIADOS_IS:     return "Solo se permite un Indice Secundario (IS) por tabla";
        case BD_ERROR_NECESITA_INDICE:   return "La operacion solciitada requiere un Indice Secundario (IS)";
        case BD_ERROR_ESCRITURA:         return "Error de escritura";
        case BD_ERROR_LECTURA:           return "Error de lectura";
        case BD_ERROR_CANT_CAMPOS:       return "Numero de campos incorrecto";
        case BD_ERROR_CAMPO_INEXISTENTE: return "Un campo porporcionado no existe en la tabla";
        case BD_ERROR_CAMPO_INVALIDO:    return "Tipo de campo invalido";
        case BD_ERROR_SIN_MEMO:          return "Memoria RAM insuficiente";
        case BD_ERROR_ARCHIVO:           return "Error de archivo";
        case BD_ERROR_PARAMETRO:         return "Error de parametro interno";
        case BD_ERROR_TAM_TEXTO:         return "La longitud del campo TEXTO es invalido";
        case BD_ERROR_CAMPO_ES_AI:       return "No se puede insertar manualmente un campo Autoincremental (AI)";
        case BD_ERROR_AI_NO_ES_ENTERO:   return "El campo Autoincremental (AI) debe ser de tipo ENTERO";
        case BD_ERROR_ENCABEZADO:        return "El encabezado de la tabla esta corrupto";
        case BD_ERROR_AI_NO_ES_PK:       return "El campo Autoincremental (AI) debe ser Clave Primaria (PK)";
        default:                         return "Error desconocido";
    }
}

int bdatos_iniciar(tBDatos *bDatos)
{
    memset(bDatos, 0, sizeof(tBDatos));

    return BD_TODO_OK;
}

int bdatos_apagar(tBDatos *bDatos)
{
    return _bdatos_cerrar_tabla(bDatos);
}
