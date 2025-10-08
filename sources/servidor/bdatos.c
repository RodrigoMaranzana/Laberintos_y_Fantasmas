#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

typedef int (*tParsear)(tSecuencia *secuencia, void *salida, unsigned tamSalida);

static int _bdatos_cerrar_tabla(tBDatos *bDatos);
static int _bdatos_insertar(tBDatos *bDatos);
static int _bdatos_seleccionar(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos);
static int _bdatos_actualizar(tBDatos *bDatos);
static int _bdatos_tabla_existe(const char *nombreTabla);
static int _bdatos_crear_tabla(tBDatos *bDatos, const char *nombreTabla);
static int _bdatos_construir_encabezado(tEncabezado *encabezado, const char *nombreTabla, int cantCampos, tCampo *campos);
static int _bdatos_buscar_campo(const tCampo *campos, int cantCampos, tCampo *campoEncontrado, const char *nombreCampoLeido);
static int _bdatos_manejar_apertura_tabla(tBDatos *bDatos, const char *nombreTabla);
static int _bdatos_cmp_indice(const void *a, const void *b);
static int _bdatos_parsear(tSecuencia *secuencia, tParsear parsear, void *salida, unsigned tamSalida);
static int _bdatos_parsear_identificador(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_simbolo(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_caracter(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_numeros(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_texto(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static void _bdatos_crear_secuencia(tSecuencia *secuencia, const char *buffer);
static int _bdatos_parsear_declaracion_campos(tSecuencia *secuencia, tCampo *campos, int maxCampos, int *cantCamposLeidos);
static int _bdatos_leer_campo_tipo(tSecuencia *secuencia, tCampo *campo);
static int _bdatos_parsear_valores(tSecuencia *secuencia, const tEncabezado *encabezado, eModoParseoValores modo, tDatoParseado *datosParseados, int *cantParseados);
static eSimbolo _bdatos_comparar_simbolo(const char* simbolo);

static int _bdatos_cerrar_tabla(tBDatos *bDatos)
{
    int ret = BD_TODO_OK;
    FILE *archIdx;
    char nombreIdx[TAM_NOMBRE_ARCH];

    if (!bDatos->tablaAbierta.arch) return BD_TODO_OK;

    sprintf(nombreIdx, "%s.idx", bDatos->tablaAbierta.encabezado.nombreTabla);
    printf("Cerrando y guardando tabla: %s\n", bDatos->tablaAbierta.encabezado.nombreTabla);

    archIdx = fopen(nombreIdx, "wb");

    if (archIdx) {

        if (arbol_escribir_en_arch(archIdx, &bDatos->tablaAbierta.arbol) != ARBOL_BD_TODO_OK) {
            ret = BD_ERROR_ESCRITURA;
        }

        fclose(archIdx);

        if (ret == BD_TODO_OK) {
            rewind(bDatos->tablaAbierta.arch);
            if (fwrite(&bDatos->tablaAbierta.encabezado, sizeof(tEncabezado), 1, bDatos->tablaAbierta.arch) != 1) {
                ret = BD_ERROR_ESCRITURA;
            }
        }

    } else {
        ret = BD_ERROR_ESCRITURA;
    }

    arbol_vaciar(&bDatos->tablaAbierta.arbol);
    fclose(bDatos->tablaAbierta.arch);
    memset(&bDatos->tablaAbierta, 0, sizeof(tTabla));

    return ret;
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
    if (fread(encabezado, sizeof(tEncabezado), 1, arch) != 1)  return BD_ERROR_LECTURA;

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
    } else if (strcmp(simbolo, "DONDE") == 0) {
        return DONDE;
    }  else if (strcmp(simbolo, "ENTERO") == 0) {
        return ENTERO;
    } else if (strcmp(simbolo, "TEXTO") == 0) {
        return TEXTO;
    } else if (strcmp(simbolo, "PK") == 0) {
        return PK;
    } else if (strcmp(simbolo, "AI") == 0) {
        return AI;
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

        return _bdatos_crear_tabla(bDatos, nombreTabla);

    } else {
        if ((ret = _bdatos_manejar_apertura_tabla(bDatos, nombreTabla)) != BD_TODO_OK) {
            return ret;
        }
    }

    switch (simboloComando) {
        case INSERTAR:
            return _bdatos_insertar(bDatos);
        case SELECCIONAR: {
            ret = _bdatos_seleccionar(bDatos, listaDatos, cantRegistrosDatos);

            *tamRegistroDatos = ret == BD_DATOS_OBTENIDOS ? bDatos->tablaAbierta.encabezado.tamRegistro : 0;

            return ret;
        }

        case ACTUALIZAR:
            return _bdatos_actualizar(bDatos);
        default:
            return BD_ERROR_COMANDO;
    }
}

static int _bdatos_manejar_apertura_tabla(tBDatos *bDatos, const char *nombreTabla)
{
    int ret;
    tArbol arbol;
    FILE *archDat, *archIdx;
    tEncabezado encabezado;
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
        return BD_ERROR_ENCABEZADO;
    }

    arbol_crear(&arbol);
    sprintf(nombreArch, "%s.idx", nombreTabla);
    archIdx = fopen(nombreArch, "rb");
    if (!archIdx) {
        fclose(archDat);
        return BD_ERROR_ARCHIVO;
    }

    ret = arbol_cargar_de_archivo(archIdx, &arbol, encabezado.tamRegIdx, _bdatos_cmp_indice);
    fclose(archIdx);

    if (ret != ARBOL_BD_TODO_OK) {
        fclose(archDat);
        return BD_ERROR_ARCHIVO;
    }

    if (bDatos->tablaAbierta.arch) {
        if ((ret = _bdatos_cerrar_tabla(bDatos)) != BD_TODO_OK) {
            fclose(archDat);
            arbol_vaciar(&arbol);
            return ret;
        }
    }

    bDatos->tablaAbierta.arbol = arbol;
    bDatos->tablaAbierta.arch = archDat;
    memcpy(&bDatos->tablaAbierta.encabezado, &encabezado, sizeof(tEncabezado));

    printf("Tabla '%s' abierta correctamente.\n", bDatos->tablaAbierta.encabezado.nombreTabla);

    return BD_TODO_OK;
}

/// CREAR jugadores (username TEXTO(16) PK, record ENTERO, cantPartidas ENTERO)
static int _bdatos_crear_tabla(tBDatos *bDatos, const char *nombreTabla)
{
    int cantCamposLeidos, ret;
    FILE *archIdx;
    tEncabezado encabezado;
    tCampo campos[MAX_CAMPOS_POR_TABLA];
    char caracter, nombreDat[TAM_NOMBRE_ARCH], nombreIdx[TAM_NOMBRE_ARCH];

    memset(campos, 0, sizeof(campos));
    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != '(') return BD_ERROR_SINTAXIS;
    if ((ret = _bdatos_parsear_declaracion_campos(&bDatos->secuencia, campos, MAX_CAMPOS_POR_TABLA, &cantCamposLeidos)) != BD_TODO_OK) return ret;

    sprintf(nombreDat, "%s.dat", nombreTabla);
    sprintf(nombreIdx, "%s.idx", nombreTabla);

    bDatos->tablaAbierta.arch = fopen(nombreDat, "w+b");
    if (!bDatos->tablaAbierta.arch) return BD_ERROR_ARCHIVO;

    if ((ret = _bdatos_construir_encabezado(&encabezado, nombreTabla, cantCamposLeidos, campos)) != BD_TODO_OK) {
        fclose(bDatos->tablaAbierta.arch);
        bDatos->tablaAbierta.arch = NULL;
        remove(nombreDat);
        return ret;
    }

    fseek(bDatos->tablaAbierta.arch, 0, SEEK_SET);
    if (fwrite(&encabezado, sizeof(tEncabezado), 1, bDatos->tablaAbierta.arch) != 1) {
        fclose(bDatos->tablaAbierta.arch);
        bDatos->tablaAbierta.arch = NULL;
        remove(nombreDat);
        return BD_ERROR_ESCRITURA;
    }
    fflush(bDatos->tablaAbierta.arch);

    archIdx = fopen(nombreIdx, "w+b");
    if (!archIdx) {
        fclose(bDatos->tablaAbierta.arch);
        bDatos->tablaAbierta.arch = NULL;
        remove(nombreDat);
        return BD_ERROR_ARCHIVO;
    }
    fclose(archIdx);

    memcpy(&bDatos->tablaAbierta.encabezado, &encabezado, sizeof(tEncabezado));
    arbol_crear(&bDatos->tablaAbierta.arbol);

    return BD_TODO_OK;
}

static int _bdatos_construir_encabezado(tEncabezado *encabezado, const char *nombreTabla, int cantCampos, tCampo *campos)
{
    int i, esAI = 0;

    memset(encabezado, 0, sizeof(tEncabezado));
    encabezado->cantCampos = cantCampos;
    strncpy(encabezado->nombreTabla, nombreTabla, TAM_IDENTIFICADOR - 1);
    encabezado->nombreTabla[TAM_IDENTIFICADOR - 1] = '\0';

    for (i = 0; i < cantCampos; i++) {

        strncpy(encabezado->campos[i].nombre, campos[i].nombre, TAM_IDENTIFICADOR - 1);
        encabezado->campos[i].nombre[TAM_IDENTIFICADOR - 1] = '\0';
        encabezado->campos[i].tipo = campos[i].tipo;
        encabezado->campos[i].tam = campos[i].tam;
        encabezado->campos[i].esPK = campos[i].esPK;
        encabezado->campos[i].esAI = campos[i].esAI;

        if (campos[i].esAI) {
            if (esAI == 1) {
                return BD_ERROR_DEMASIADOS_AI;
            }
            esAI = 1;
        }

        encabezado->campos[i].offsetCampo = campos[i].offsetCampo;
        encabezado->tamRegistro += campos[i].tam;
    }

    encabezado->tamRegIdx = sizeof(tIndice);
    encabezado->proximoAI = esAI ? 1 : 0;

    return BD_TODO_OK;
}

static int _bdatos_cmp_indice(const void *a, const void *b)
{
    tIndice *indiceA = (tIndice*)a;
    tIndice *indiceB = (tIndice*)b;

    return strcmp(indiceA->clave, indiceB->clave);
}

static int _bdatos_buscar_campo(const tCampo *campos, int cantCampos, tCampo *campoEncontrado, const char *nombreCampoLeido)
{
    int campo, encontrado = 0;

    memset(campoEncontrado, 0, sizeof(tCampo));

    for (campo = 0; campo < cantCampos && !encontrado; campo++) {

        if (strcmp(campos[campo].nombre, nombreCampoLeido) == 0) {
            encontrado = 1;
            memcpy(campoEncontrado, &campos[campo], sizeof(tCampo));
        }
    }

    return encontrado ? BD_TODO_OK : BD_ERROR_CAMPO_INEXISTENTE;
}

/// ACTUALIZAR jugadores (puntajeMax 300) DONDE (username IGUAL PEPE)
static int _bdatos_actualizar(tBDatos *bDatos)
{
    int ret, cantParseados = 0, i, cantRegistrosDatos, encontradoPK = 0;
    tCampo campoPK;
    tLista listaDatos;
    eSimbolo simboloConector;
    tDatoParseado datosParseados[MAX_CAMPOS_POR_TABLA];
    char caracter, *registro = NULL, *registroActualizado = NULL;

    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != '(') return BD_ERROR_SINTAXIS;

    memset(datosParseados, 0, sizeof(datosParseados));
    ret = _bdatos_parsear_valores(&bDatos->secuencia, &bDatos->tablaAbierta.encabezado, MODO_ACTUALIZACION, datosParseados, &cantParseados);
    if (ret != BD_TODO_OK) {
        for (i = 0; i < cantParseados; i++) {
            free(datosParseados[i].valor.dato);
        }
        return ret;
    }

    for (i = 0; i < cantParseados; i++) {
        if (datosParseados[i].campo.esPK) {
            for (int j = 0; j < cantParseados; j++) {
                free(datosParseados[j].valor.dato);
            }
            return BD_ERROR_ACTUALIZAR_PK;
        }
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

    for (i = 0; i < bDatos->tablaAbierta.encabezado.cantCampos && !encontradoPK; i++) {
        if (bDatos->tablaAbierta.encabezado.campos[i].esPK) {
            campoPK = bDatos->tablaAbierta.encabezado.campos[i];
            encontradoPK = 1;
        }
    }

    while (lista_sacar_primero(&listaDatos, registro, bDatos->tablaAbierta.encabezado.tamRegistro) != LISTA_VACIA) {

        tIndice indiceReg;
        memset(&indiceReg, 0, sizeof(tIndice));

        if (campoPK.tipo == TIPO_ENTERO) {
            int valorPK = *(int*)(registro + campoPK.offsetCampo);
            sprintf(indiceReg.clave, "%011d", valorPK);
        } else {
            strncpy(indiceReg.clave, registro + campoPK.offsetCampo, campoPK.tam);
        }

        memcpy(registroActualizado, registro, bDatos->tablaAbierta.encabezado.tamRegistro);

        for (i = 0; i < cantParseados; i++) {
            memcpy(registroActualizado + datosParseados[i].campo.offsetCampo, datosParseados[i].valor.dato, datosParseados[i].valor.tam);
        }

        arbol_buscar(&bDatos->tablaAbierta.arbol, &indiceReg, sizeof(tIndice), _bdatos_cmp_indice);

        fseek(bDatos->tablaAbierta.arch, indiceReg.offset, SEEK_SET);
        fwrite(registroActualizado, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch);
    }

    for (i = 0; i < cantParseados; i++) {
        free(datosParseados[i].valor.dato);
    }
    free(registro);
    free(registroActualizado);
    return BD_TODO_OK;
}


/// SELECCIONAR jugadores (username IGUAL PEPE)
static int _bdatos_seleccionar(tBDatos *bDatos, tLista *listaDatos, int *cantRegistrosDatos)
{
    int ret, valor;
    char *buffer = NULL, *registro = NULL, caracter, nombreCampoLeido[TAM_IDENTIFICADOR] = {0};
    eSimbolo proximoSimbolo;
    tCampo campoEncontrado;

    *cantRegistrosDatos = 0;

    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != BD_TODO_OK || caracter != '(') return BD_ERROR_SINTAXIS;
    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_identificador, nombreCampoLeido, TAM_IDENTIFICADOR)) != BD_TODO_OK) return ret;

    memset(&campoEncontrado, 0, sizeof(tCampo));
    if ((ret = _bdatos_buscar_campo(bDatos->tablaAbierta.encabezado.campos, bDatos->tablaAbierta.encabezado.cantCampos, &campoEncontrado, nombreCampoLeido)) != BD_TODO_OK) return ret;

    // ESCALABLE -> IGUAL, MAYOR, MENOR, DISTINTO
    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo)) != BD_TODO_OK || proximoSimbolo != IGUAL) return BD_ERROR_SINTAXIS;

    if (campoEncontrado.tipo == TIPO_ENTERO) {
        if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_numeros, &valor, sizeof(int))) != BD_TODO_OK) return ret;
    } else if (campoEncontrado.tipo == TIPO_TEXTO) {
        buffer = (char*)malloc(campoEncontrado.tam);
        if (!buffer) return BD_ERROR_SIN_MEMO;
        memset(buffer, 0, campoEncontrado.tam);
        if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_texto, buffer, campoEncontrado.tam)) != BD_TODO_OK) {
            free(buffer);
            return ret;
        }
    } else {
        return BD_ERROR_CAMPO_INVALIDO;
    }

    registro = (char*)malloc(bDatos->tablaAbierta.encabezado.tamRegistro);
    if (!registro) {
        free(buffer);
        return BD_ERROR_SIN_MEMO;
    }

    lista_crear(listaDatos);

    if (campoEncontrado.esPK) {
        tIndice indiceReg;
        if (campoEncontrado.tipo == TIPO_ENTERO) {
            sprintf(indiceReg.clave, "%011d", valor);
        } else {
            strncpy(indiceReg.clave, buffer, sizeof(indiceReg.clave) - 1);
            indiceReg.clave[sizeof(indiceReg.clave) - 1] = '\0';
        }
        if (arbol_buscar(&bDatos->tablaAbierta.arbol, &indiceReg, sizeof(tIndice), _bdatos_cmp_indice) == ARBOL_BD_TODO_OK) {
            fseek(bDatos->tablaAbierta.arch, indiceReg.offset, SEEK_SET);
            fread(registro, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch);
            lista_insertar_final(listaDatos, registro, bDatos->tablaAbierta.encabezado.tamRegistro);
            ++(*cantRegistrosDatos);
        }
    } else {

        fseek(bDatos->tablaAbierta.arch, sizeof(tEncabezado), SEEK_SET);
        while (fread(registro, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch) == 1) {
            int coincidencia = 0;

            if (campoEncontrado.tipo == TIPO_ENTERO) {
                coincidencia = (*(int*)(registro + campoEncontrado.offsetCampo) == valor);
            } else {
                coincidencia = (strncmp(registro + campoEncontrado.offsetCampo, buffer, campoEncontrado.tam) == 0);
            }

            if (coincidencia) {
                lista_insertar_final(listaDatos, registro, bDatos->tablaAbierta.encabezado.tamRegistro);
                ++(*cantRegistrosDatos);
            }
        }
    }

    free(registro);
    free(buffer);
    return *cantRegistrosDatos == 0 ? BD_TODO_OK : BD_DATOS_OBTENIDOS;
}

static int _bdatos_parsear_declaracion_campos(tSecuencia *secuencia, tCampo *campos, int maxCampos, int *cantCamposLeidos)
{
    int ret, i = 0, fin = 0, hayPK = 0;
    char caracter;
    eSimbolo proximoSimbolo;
    unsigned offsetCampo = 0;

    do {
        if (i >= maxCampos) return BD_ERROR_DEMASIADOS_CAMPOS;

        if ((ret = _bdatos_parsear(secuencia, _bdatos_parsear_identificador, campos[i].nombre, TAM_IDENTIFICADOR)) != BD_TODO_OK) return ret;
        if ((ret = _bdatos_leer_campo_tipo(secuencia, &campos[i])) != BD_TODO_OK) return ret;

        campos[i].offsetCampo = offsetCampo;
        offsetCampo += campos[i].tam;

        if(_bdatos_parsear(secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo)) == BD_TODO_OK) {

            if (proximoSimbolo == PK) {
                campos[i].esPK = 1;
                hayPK++;

                if (_bdatos_parsear(secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo)) == BD_TODO_OK) {
                    if (proximoSimbolo == AI) {
                        if (campos[i].tipo != TIPO_ENTERO) return BD_ERROR_AI_NO_ES_ENTERO;
                        campos[i].esAI = 1;
                    } else {
                        return BD_ERROR_SINTAXIS;
                    }
                }
            } else if (proximoSimbolo == AI) {
                return BD_ERROR_AI_NO_ES_PK;
            } else {
                return BD_ERROR_SINTAXIS;
            }
        }

        if ((ret = _bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char))) != BD_TODO_OK) return ret;

        if (caracter == ')') {
            fin = 1;
        }else if (caracter != ',') {
            return  BD_ERROR_SINTAXIS;
        }

        if (hayPK > 1) return BD_ERROR_DEMASIADOS_PK;

        i++;

    } while (!fin);

    if (hayPK == 0) return BD_ERROR_SIN_PK;

    *cantCamposLeidos = i;

    return BD_TODO_OK;
}

/// INSERTAR jugadores (username PEPE, record 15, cantPartidas 5)
static int _bdatos_insertar(tBDatos *bDatos)
{
    int ret, cantParseados = 0, i;
    tDatoParseado datosParseados[MAX_CAMPOS_POR_TABLA];
    char caracter, *registro = NULL;
    tIndice nuevoIndiceReg;

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
    memset(&nuevoIndiceReg, 0, sizeof(tIndice));
    for (i = 0; i < cantParseados; i++) {
        memcpy(registro + datosParseados[i].campo.offsetCampo, datosParseados[i].valor.dato, datosParseados[i].valor.tam);
            if (datosParseados[i].campo.esPK) {
            if (datosParseados[i].campo.tipo == TIPO_TEXTO) {
                strncpy(nuevoIndiceReg.clave, (char*)datosParseados[i].valor.dato, sizeof(nuevoIndiceReg.clave) - 1);
                nuevoIndiceReg.clave[sizeof(nuevoIndiceReg.clave) - 1] = '\0';
            } else {
                sprintf(nuevoIndiceReg.clave, "%011d", *(int*)datosParseados[i].valor.dato);
            }
        }
    }

    fseek(bDatos->tablaAbierta.arch, 0, SEEK_END);
    nuevoIndiceReg.offset = ftell(bDatos->tablaAbierta.arch);
    if ((ret = arbol_insertar_rec(&bDatos->tablaAbierta.arbol, &nuevoIndiceReg, sizeof(tIndice), _bdatos_cmp_indice)) != ARBOL_BD_TODO_OK) {
        for (i = 0; i < cantParseados; i++) {
            free(datosParseados[i].valor.dato);
        }
        free(registro);
        return ret;
    }

    fwrite(registro, bDatos->tablaAbierta.encabezado.tamRegistro, 1, bDatos->tablaAbierta.arch);
    bDatos->tablaAbierta.encabezado.cantRegistros++;
    if (bDatos->tablaAbierta.encabezado.proximoAI != 0) {
        ++bDatos->tablaAbierta.encabezado.proximoAI;
    }

    for (i = 0; i < cantParseados; i++) {
        free(datosParseados[i].valor.dato);
    }
    free(registro);

    return BD_TODO_OK;
}

static int _bdatos_parsear_valores(tSecuencia *secuencia, const tEncabezado *encabezado, eModoParseoValores modo, tDatoParseado *datosParseados, int *cantParseados)
{
    int retorno, i = 0, j, fin = 0, cantPK = 0;
    char caracter, nombreCampoLeido[TAM_IDENTIFICADOR];
    tCampo campoEncontrado;

    memset(nombreCampoLeido, 0, sizeof(nombreCampoLeido));
    *cantParseados = 0;

    do {
        if (i >= encabezado->cantCampos) return BD_ERROR_DEMASIADOS_CAMPOS;

        if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_identificador, nombreCampoLeido, TAM_IDENTIFICADOR)) != BD_TODO_OK) return retorno;

        memset(&campoEncontrado, 0, sizeof(tCampo));
        if ((retorno = _bdatos_buscar_campo(encabezado->campos, encabezado->cantCampos, &campoEncontrado, nombreCampoLeido)) != BD_TODO_OK) return retorno;

        if (campoEncontrado.esAI) {
            for (j = 0; j < i; j++) free(datosParseados[j].valor.dato);
            return BD_ERROR_CAMPO_ES_AI;
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

    if (encabezado->proximoAI != 0) {

        int j = 0, encontrado = 0, campoAI = 0;
        while (j < encabezado->cantCampos && !encontrado) {

            if (encabezado->campos[j].esAI) {
                campoAI = j;
                encontrado = 1;
            }
            ++j;
        }

        memcpy(&datosParseados[i].campo, &encabezado->campos[campoAI], sizeof(tCampo));
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

const char* bdatos_obtener_mensaje(eBDRetorno codigoError)
{
    switch (codigoError) {
        case BD_TODO_OK:                 return "OK";
        case BD_DATOS_OBTENIDOS:         return "Datos obtenidos";
        case BD_ERROR_SIN_RESULTADOS:    return "No se encontraron coincidencias";
        case BD_ERROR_SINTAXIS:          return "Error de sintaxis en la solicitud";
        case BD_ERROR_COMANDO:           return "Comando desconocido";
        case BD_ERROR_TABLA_EXISTE:      return "La tabla ya existe";
        case BD_ERROR_TABLA_NO_EXISTE:   return "La tabla no existe";
        case BD_ERROR_DEMASIADOS_CAMPOS: return "La cantidad de campos supera el maximo permitido";
        case BD_ERROR_SIN_PK:            return "La operacion requiere una Clave Primaria (PK)";
        case BD_ERROR_ACTUALIZAR_PK:     return "No se permite actualizar el valor de una Clave Primaria (PK)";
        case BD_ERROR_DEMASIADOS_PK:     return "Solo se permite una Clave Primaria (PK) por tabla";
        case BD_ERROR_DEMASIADOS_AI:     return "Solo se permite un campo Autoincremental (AI) por tabla";
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
