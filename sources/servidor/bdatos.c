#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/servidor/bdatos.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/arbol.h"
#include "../../include/comun/lista.h"

typedef int (*tParsear)(tSecuencia *secuencia, void *salida, unsigned tamSalida);

static int _bdatos_dummy();


static int _bdatos_ejecutar_comando(tBDatos *bDatos, eSimbolo comando);
static int _bdatos_insertar(tBDatos *bDatos);
static int _bdatos_crear(tBDatos *bDatos);
static int _bdatos_seleccionar(tBDatos *bDatos);


static eTipoDato _bdatos_convertir_simbolo_a_tipo(eSimbolo simbolo);
static int _bdatos_tabla_existe(const char *idenTabla);
static int _bdatos_buscar_campo(const tCampo *campos, int cantCampos, tCampo *campoEncontrado, const char *nombreCampoLeido);


static int _bdatos_escribir_encabezado(FILE *arch, const char *nombreTabla, int cantCampos, tCampo *campos, int esAI);
static int _bdatos_crear_archivo_tabla(tBDatos *bDatos, const char *idenTabla, tCampo *campos, int cantCampos);


/// IDX
static int _bdatos_guardar_idx(tBDatos *bDatos, const char* nombreIdx);
static int _bdatos_cargar_idx(tBDatos *bDatos, const char* nombreIdx);
static int _bdatos_cmp_indice(const void *a, const void *b);


/// PARSEO Y SECUENCIADOR
static int _bdatos_parsear(tSecuencia *secuencia, tParsear parsear, void *salida, unsigned tamSalida);
static int _bdatos_parsear_identificador(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_simbolo(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_caracter(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_numeros(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _bdatos_parsear_texto(tSecuencia *secuencia, void *salida, unsigned tamSalida);


static void _bdatos_crear_secuencia(tSecuencia *secuencia, const char *buffer);
static eSimbolo _bdatos_comparar_simbolo(const char* simbolo);

static int _bdatos_leer_campos(tSecuencia *secuencia, tCampo *campos, int maxCampos);
static int _bdatos_leer_campo_tipo(tSecuencia *secuencia, tCampo *campo);
static int _bdatos_leer_campos_y_valores(tSecuencia *secuencia, const tEncabezado *encabezado, tDatoParseado *datosParseados, int *cantParseados);



int bdatos_iniciar(tBDatos *bDatos)
{
    memset(bDatos, 0, sizeof(tBDatos));

    return ERROR_TODO_OK;
}

int bdatos_procesar_solcitud(tBDatos *bDatos, const char *solicitud)
{
    int ret = ERROR_TODO_OK;
    eSimbolo simbolo;

    _bdatos_crear_secuencia(&bDatos->secuencia, solicitud);

    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &simbolo, sizeof(eSimbolo)) != ERROR_TODO_OK)) {
        return ret;
    }

    if (!(simbolo >= CREAR && simbolo <= SELECCIONAR)) {
        return ERROR_COMANDO;
    }

    ret = _bdatos_ejecutar_comando(bDatos, simbolo);

    return ret;
}

static int _bdatos_parsear(tSecuencia *secuencia, tParsear parsear, void *salida, unsigned tamSalida)
{
    int ret;

    while (*secuencia->cursor && *secuencia->cursor == ' ') {
        secuencia->cursor++;
    }

    if (!*secuencia->cursor || secuencia->finSec) return ERROR_SINTAXIS;

    if ((ret = parsear(secuencia, salida, tamSalida)) != ERROR_TODO_OK) return ret;

    return ERROR_TODO_OK;
}


static int _bdatos_parsear_identificador(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{

    char *identificador = (char*)salida;
    char *cursor = identificador;

    if (!ES_LETRA(*secuencia->cursor)) return ERROR_SINTAXIS;

    while (*secuencia->cursor && ES_LETRA(*secuencia->cursor) && (cursor - identificador) < (tamSalida - 1)) {
        *cursor++ = *secuencia->cursor++;
    }

    *cursor = '\0';

    return ERROR_TODO_OK;
}

static int _bdatos_parsear_simbolo(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    eSimbolo *simbolo = (eSimbolo*)salida;
    char simboloBuffer[TAM_SIMBOLO] = {0};
    char *cursor = simboloBuffer;

    if (tamSalida != sizeof(eSimbolo)) return ERROR_PARAMETRO;

    while (*secuencia->cursor && ES_LETRA(*secuencia->cursor) && (cursor - simboloBuffer) < (TAM_SIMBOLO - 1)) {
        *cursor++ = *secuencia->cursor++;
    }

    *cursor = '\0';

    *simbolo = _bdatos_comparar_simbolo(simboloBuffer);
    if (*simbolo == DESCONOCIDO) {
        return ERROR_SINTAXIS;
    }

    return ERROR_TODO_OK;
}

static int _bdatos_parsear_caracter(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    char *caracter = (char*)salida;

    if (tamSalida != sizeof(char)) return ERROR_PARAMETRO;

    if (*secuencia->cursor) {
        *caracter = *secuencia->cursor;
    } else {
        return ERROR_SINTAXIS;
    }

    secuencia->cursor++;
    return ERROR_TODO_OK;
}

static int _bdatos_parsear_numeros(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    int *numero = (int*)salida;
    char numBuffer[TAM_INT_TEXTO] = {0};
    char *cursor = numBuffer;

    if (tamSalida != sizeof(int)) return ERROR_PARAMETRO;

    if (*secuencia->cursor == '-' || *secuencia->cursor == '+') {
        *cursor++ = *secuencia->cursor++;
    }

    if (!ES_DIGITO(*secuencia->cursor)) {
        return ERROR_SINTAXIS;
    }

    while (*secuencia->cursor && ES_DIGITO(*secuencia->cursor) && (cursor - numBuffer) < (TAM_INT_TEXTO - 1)) {
        *cursor++ = *secuencia->cursor++;
    }

    *cursor = '\0';
    sscanf(numBuffer, "%d", numero);

    return ERROR_TODO_OK;
}


static int _bdatos_parsear_texto(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    char *texto = (char*)salida;
    char *cursor = texto;

    while (*secuencia->cursor && *secuencia->cursor != ',' && *secuencia->cursor != ')' && (cursor - texto) < (tamSalida - 1)) {
        *cursor++ = *secuencia->cursor++;
    }

    *cursor = '\0';

    return ERROR_TODO_OK;
}






/// CODIGO SIN REVISAR





static int _bdatos_leer_campos(tSecuencia *secuencia, tCampo *campos, int maxCampos)
{
    int ret, i = 0, fin = 0, hayPK = 0;
    char caracter;
    eSimbolo proximoSimbolo;
    unsigned offsetCampo = 0;

    do {
        if (i >= maxCampos) return ERROR_DEMASIADOS_CAMPOS;

        if ((ret = _bdatos_parsear(secuencia, _bdatos_parsear_identificador, campos[i].nombre, TAM_IDENTIFICADOR)) != ERROR_TODO_OK) return ret;
        if ((ret = _bdatos_leer_campo_tipo(secuencia, &campos[i])) != ERROR_TODO_OK) return ret;

        campos[i].offsetCampo = offsetCampo;
        offsetCampo += campos[i].tam;

        _bdatos_parsear(secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
        if (proximoSimbolo == PK) {

            campos[i].esPK = 1;
            hayPK++;

            _bdatos_parsear(secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
            if (proximoSimbolo == AI) {

                if (campos[i].tipo != TIPO_ENTERO) return ERROR_AI_DEBE_SER_ENTERO;

                campos[i].esAI = 1;

            }else if (proximoSimbolo != DESCONOCIDO) {
                return ERROR_SINTAXIS;
            }
        }

        if ((ret = _bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char))) != ERROR_TODO_OK) return ret;

        if (caracter == ')') {
            fin = 1;
        }else if (caracter != ',') {
            return  ERROR_SINTAXIS;
        }

        if (hayPK > 1) return ERROR_DEMASIADOS_PK;

        i++;

    } while (!fin);

    return i;
}

static int _bdatos_leer_campo_tipo(tSecuencia *secuencia, tCampo *campo)
{
    char caracter;
    eSimbolo proximoSimbolo;
    _bdatos_parsear(secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
    campo->tipo = _bdatos_convertir_simbolo_a_tipo(proximoSimbolo);

    if (campo->tipo == TIPO_INVALIDO) return ERROR_SINTAXIS;

    if (campo->tipo == TIPO_ENTERO) {
        campo->tam = sizeof(int);
    } else if (campo->tipo == TIPO_TEXTO) {
        int numero;
        if (_bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != ERROR_TODO_OK || caracter != '(') return ERROR_SINTAXIS;
        if (_bdatos_parsear(secuencia, _bdatos_parsear_numeros, &numero, sizeof(int)) != ERROR_TODO_OK) return ERROR_SINTAXIS;
        if (numero <= 0 || numero > TAM_MAX_TIPO_CHAR) return ERROR_TAM_TEXTO;
        if (_bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != ERROR_TODO_OK || caracter != ')') return ERROR_SINTAXIS;
        campo->tam = numero;
    }
    return ERROR_TODO_OK;
}

static int _bdatos_tabla_existe(const char *idenTabla)
{
    char nombreArchivo[TAM_NOMBRE_ARCH];
    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.dat", idenTabla);

    FILE *arch = fopen(nombreArchivo, "r");
    if (arch) {

        fclose(arch);
        return 1;
    }
    return 0;
}


static int _bdatos_crear_archivo_tabla(tBDatos *bDatos, const char *idenTabla, tCampo *campos, int cantCampos)
{
    int ret, i, encontrado, esAI = 0;
    char nombreArchivo[TAM_NOMBRE_ARCH];
    FILE *archDat = NULL, *archIdx = NULL;

    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.dat", idenTabla);
    archDat = fopen(nombreArchivo, "w+b");
    if (!archDat) {
        return ERR_ARCHIVO;
    }

    i = 0;
    encontrado = 0;
    while (i < cantCampos && !encontrado) {

        if (campos[i].esAI) {

            esAI = 1;
            encontrado = 1;
        }
        ++i;
    }

    if ((ret = _bdatos_escribir_encabezado(archDat, idenTabla, cantCampos, campos, esAI)) != ERROR_TODO_OK) {
        fclose(archDat);
        return ret;
    }

    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.idx", idenTabla);
    archIdx = fopen(nombreArchivo, "w+b");
    if (!archIdx) {
        fclose(archDat);
        return ERR_ARCHIVO;
    }

    fclose(archDat);
    fclose(archIdx);

    return ERROR_TODO_OK;
}

/// PRIMERA REVISION REALIZADA
static int _bdatos_escribir_encabezado(FILE *arch, const char *nombreTabla, int cantCampos, tCampo *campos, int esAI)
{
    int i;
    tEncabezado encabezado;

    if (!arch) return ERROR_ARCH;

    memset(&encabezado, 0, sizeof(tEncabezado));
    encabezado.cantCampos = cantCampos;
    strncpy(encabezado.nombreTabla, nombreTabla, TAM_IDENTIFICADOR - 1);
    encabezado.nombreTabla[TAM_IDENTIFICADOR - 1] = '\0';

    for (i = 0; i < cantCampos; i++) {

        strncpy(encabezado.campos[i].nombre, campos[i].nombre, TAM_IDENTIFICADOR - 1);
        encabezado.campos[i].nombre[TAM_IDENTIFICADOR - 1] = '\0';
        encabezado.campos[i].tipo = campos[i].tipo;
        encabezado.campos[i].tam = campos[i].tam;
        encabezado.campos[i].esPK = campos[i].esPK;
        encabezado.campos[i].esAI = campos[i].esAI;
        encabezado.campos[i].offsetCampo = campos[i].offsetCampo;
        encabezado.tamRegistro += campos[i].tam;
    }

    encabezado.tamRegIdx = sizeof(tIndice);
    encabezado.proximoAI = esAI ? 1 : 0;

    fseek(arch, 0, SEEK_SET);
    if (fwrite(&encabezado, sizeof(tEncabezado), 1, arch) != 1) return ERROR_ESCRITURA;

    fflush(arch);

    return ERROR_TODO_OK;
}

static int _bdatos_leer_encabezado(FILE* arch, tEncabezado *encabezado) {

    if (!arch) {
        return ERROR_ARCH;
    }

    fseek(arch, 0, SEEK_SET);

    if (fread(encabezado, sizeof(tEncabezado), 1, arch) != 1) {
        return ERROR_LECTURA;
    }

    return TODO_OK;
}


static eTipoDato _bdatos_convertir_simbolo_a_tipo(eSimbolo simbolo)
{
    switch (simbolo) {
        case ENTERO:
            return TIPO_ENTERO;
        case TEXTO:
            return TIPO_TEXTO;
        default:
            return TIPO_INVALIDO;
    }
}

/// CREAR TABLA jugadores (username TEXTO(16) PK, record ENTERO, cantPartidas ENTERO)
static int _bdatos_crear(tBDatos *bDatos)
{
    tCampo campos[MAX_CAMPOS_POR_TABLA] = {0};
    char caracter, idenTabla[TAM_IDENTIFICADOR] = {0};
    int cantCampos, retorno;
    eSimbolo proximoSimbolo;

    _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
    if (proximoSimbolo != TABLA) return ERROR_SINTAXIS;
    if ((retorno = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_identificador, idenTabla, TAM_IDENTIFICADOR)) != ERROR_TODO_OK) return retorno;
    if (_bdatos_tabla_existe(idenTabla)) return ERROR_TABLA_YA_EXISTE;
    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != ERROR_TODO_OK || caracter != '(') return ERROR_SINTAXIS;

    cantCampos = _bdatos_leer_campos(&bDatos->secuencia, campos, MAX_CAMPOS_POR_TABLA);
    if (cantCampos < 0) return ERROR_CANT_CAMPOS;

    printf("CREAR TABLA -> Tabla: %s, Campos: %d\n", idenTabla, cantCampos);

    return _bdatos_crear_archivo_tabla(bDatos, idenTabla, campos, cantCampos);
}


static int _bdatos_leer_campos_y_valores(tSecuencia *secuencia, const tEncabezado *encabezado, tDatoParseado *datosParseados, int *cantParseados)
{
    int retorno, i = 0, fin = 0, cantPK = 0;
    char caracter, nombreCampoLeido[TAM_IDENTIFICADOR] = {0};
    tCampo campoEncontrado;

    *cantParseados = 0;

    do {
        if (i >= encabezado->cantCampos) return ERROR_DEMASIADOS_CAMPOS;

        if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_identificador, nombreCampoLeido, TAM_IDENTIFICADOR)) != ERROR_TODO_OK) return retorno;

        memset(&campoEncontrado, 0, sizeof(tCampo));
        if ((retorno = _bdatos_buscar_campo(encabezado->campos, encabezado->cantCampos, &campoEncontrado, nombreCampoLeido)) != ERROR_TODO_OK) return retorno;

        if (campoEncontrado.esPK) {
            ++cantPK;
        }

        if (campoEncontrado.esAI) {
            return ERROR_CAMPO_ES_AI;
        }

        switch (campoEncontrado.tipo) {
            case TIPO_ENTERO: {

                datosParseados[i].valor.dato = malloc(sizeof(int));
                if (!datosParseados[i].valor.dato) return ERROR_SIN_MEMO;

                datosParseados[i].valor.tam = sizeof(int);

                if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_numeros, (int*)datosParseados[i].valor.dato, sizeof(int))) != ERROR_TODO_OK) {
                    free(datosParseados[i].valor.dato);
                    return retorno;
                }
                break;
            }
            case TIPO_TEXTO: {

                datosParseados[i].valor.dato = malloc(campoEncontrado.tam);
                if (!datosParseados[i].valor.dato) return ERROR_SIN_MEMO;
                memset(datosParseados[i].valor.dato, 0, campoEncontrado.tam);

                datosParseados[i].valor.tam = campoEncontrado.tam;

                if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_texto, (char*)datosParseados[i].valor.dato, campoEncontrado.tam)) != ERROR_TODO_OK) {
                    free(datosParseados[i].valor.dato); /// MEMORY LEAK, NO LIBERA LOS ANTERIORES
                    return retorno;
                }

                break;
            }
            default:
                return ERROR_CAMPO_INVALIDO;
        }

        memcpy(&datosParseados[i].campo, &campoEncontrado, sizeof(tCampo));

        ++i;

        if ((retorno = _bdatos_parsear(secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char))) != ERROR_TODO_OK) return retorno;

        if (caracter == ')') {
            fin = 1;
        } else if (caracter != ',') {
            return ERROR_SINTAXIS;
        }

    } while (!fin);

    if (cantPK < 1 && encabezado->proximoAI == 0) return ERROR_SIN_PK;

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
        if (!datosParseados[i].valor.dato) return ERROR_SIN_MEMO;
        memset(datosParseados[i].valor.dato, 0, sizeof(int));

        datosParseados[i].valor.tam = sizeof(int);

        *(int*)datosParseados[i].valor.dato = encabezado->proximoAI;
        i++;
    }

    *cantParseados = i;
    return ERROR_TODO_OK;
}

/// INSERTAR EN jugadores (username PEPE, record 15, cantPartidas 5)
static int _bdatos_insertar(tBDatos *bDatos)
{
    int ret, cantParseados = 0, offset = 0, i, j;
    tDatoParseado datosParseados[MAX_CAMPOS_POR_TABLA] = {0};
    char caracter, idenTabla[TAM_IDENTIFICADOR] = {0}, *registro = NULL;
    eSimbolo proximoSimbolo;
    tIndice nuevoIndiceReg;

    _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
    if (proximoSimbolo != EN) return ERROR_SINTAXIS;
    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_identificador, idenTabla, TAM_IDENTIFICADOR)) != ERROR_TODO_OK) return ret;
    if (!_bdatos_tabla_existe(idenTabla)) return ERROR_TABLA_NO_EXISTE;
    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != ERROR_TODO_OK || caracter != '(') return ERROR_SINTAXIS;

    ret = _bdatos_leer_campos_y_valores(&bDatos->secuencia, &bDatos->encabezado, datosParseados, &cantParseados);
    if (ret != ERROR_TODO_OK) {
        for (i = 0; i < cantParseados; i++) {
            if (datosParseados[i].valor.dato) free(datosParseados[i].valor.dato);
        }
        return ret;
    }

    registro = (char*)malloc(bDatos->encabezado.tamRegistro);
    if (!registro) return ERROR_SIN_MEMO;
    memset(registro, 0, bDatos->encabezado.tamRegistro);
    memset(&nuevoIndiceReg, 0, sizeof(tIndice));

    for (i = 0; i < bDatos->encabezado.cantCampos; ++i) {

        int encontrado = 0;
        for (j = 0; j < cantParseados && !encontrado; ++j) {

            if (strcmp(bDatos->encabezado.campos[i].nombre, datosParseados[j].campo.nombre) == 0) {

                memcpy(registro + offset, datosParseados[j].valor.dato, datosParseados[j].valor.tam);
                if (datosParseados[j].campo.esPK) {

                    if (datosParseados[j].campo.tipo == TIPO_TEXTO) {

                        sprintf(nuevoIndiceReg.clave, "%s", (char*)datosParseados[j].valor.dato);
                    } else {

                        sprintf(nuevoIndiceReg.clave, "%011d", *(int*)datosParseados[j].valor.dato);
                    }
                }
                encontrado = 1;
            }
        }
        offset += bDatos->encabezado.campos[i].tam;
    }

    fseek(bDatos->arch, 0, SEEK_END);
    nuevoIndiceReg.offset = ftell(bDatos->arch);

    if ((ret = arbol_insertar_rec(&bDatos->arbol, &nuevoIndiceReg, sizeof(tIndice), _bdatos_cmp_indice)) != ARBOL_TODO_OK) {

        for (i = 0; i < cantParseados; i++) {
            free(datosParseados[i].valor.dato);
        }
        free(registro);
        return ret;
    }

    fwrite(registro, bDatos->encabezado.tamRegistro, 1, bDatos->arch);

    bDatos->encabezado.cantRegistros++;
    if (bDatos->encabezado.proximoAI != 0) {

        ++bDatos->encabezado.proximoAI;
    }

    for (i = 0; i < cantParseados; i++) {

        free(datosParseados[i].valor.dato);
    }
    free(registro);

    return ERROR_TODO_OK;
}


/// ABRIR TABLA jugadores
static int _bdatos_abrir(tBDatos *bDatos)
{
    int retorno;
    char idenTabla[TAM_IDENTIFICADOR] = {0}, nombreArchivo[TAM_NOMBRE_ARCH];
    eSimbolo proximoSimbolo;

    _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
    if (proximoSimbolo != TABLA) return ERROR_SINTAXIS;
    if ((retorno = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_identificador, idenTabla, TAM_IDENTIFICADOR)) != ERROR_TODO_OK) return retorno;
    if (!_bdatos_tabla_existe(idenTabla)) return ERROR_TABLA_NO_EXISTE;

    if (bDatos->arch != NULL) {
        bdatos_cerrar(bDatos);
    }

    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.dat", idenTabla);
    bDatos->arch = fopen(nombreArchivo, "r+b");
    if (!bDatos->arch) {

        return ERROR_ARCH;
    }

    if (_bdatos_leer_encabezado(bDatos->arch, &bDatos->encabezado) != TODO_OK) {
        fclose(bDatos->arch);
        bDatos->arch = NULL;
        return ERROR_LECTURA;
    }

    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.idx", idenTabla);
    bDatos->archIdx = fopen(nombreArchivo, "r+b");
    if (!bDatos->archIdx) {
        fclose(bDatos->arch);
        bDatos->arch = NULL;
        return ERROR_ARCH;
    }

    if (_bdatos_cargar_idx(bDatos, nombreArchivo) != TODO_OK) {
        fclose(bDatos->arch);
        fclose(bDatos->archIdx);
        bDatos->arch = NULL;
        bDatos->archIdx = NULL;
        return ERR_ARCHIVO;
    }

    printf("Tabla '%s' abierta correctamente.\n", bDatos->encabezado.nombreTabla);

    return ERROR_TODO_OK;
}

/////ACTUALIZAR EN jugadores (puntajeMax 25) DONDE idJugador IGUAL 21")
//static int _bdatos_actualizar(tBDatos *bDatos)
//{
//
//
//
//
//    return ERROR_TODO_OK;
//}

/// SELECCIONAR DESDE jugadores DONDE (username IGUAL PEPE)
static int _bdatos_seleccionar(tBDatos *bDatos)
{
    int ret, valor;
    char *buffer = NULL, *registro = NULL, caracter, idenTabla[TAM_IDENTIFICADOR] = {0}, nombreCampoLeido[TAM_IDENTIFICADOR] = {0};
    eSimbolo proximoSimbolo;
    tCampo campoEncontrado;
    tLista listaResultados;

    _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
    if (proximoSimbolo != DESDE) return ERROR_SINTAXIS;
    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_identificador, idenTabla, TAM_IDENTIFICADOR)) != ERROR_TODO_OK) return ret;
    if (!_bdatos_tabla_existe(idenTabla)) return ERROR_TABLA_NO_EXISTE;
    _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
    if (proximoSimbolo != DONDE) return ERROR_SINTAXIS;
    if (_bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_caracter, &caracter, sizeof(char)) != ERROR_TODO_OK || caracter != '(') return ERROR_SINTAXIS;
    if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_identificador, nombreCampoLeido, TAM_IDENTIFICADOR)) != ERROR_TODO_OK) return ret;

    memset(&campoEncontrado, 0, sizeof(tCampo));
    if ((ret = _bdatos_buscar_campo(bDatos->encabezado.campos, bDatos->encabezado.cantCampos, &campoEncontrado, nombreCampoLeido)) != ERROR_TODO_OK) return ret;

    _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_simbolo, &proximoSimbolo, sizeof(eSimbolo));
    if (proximoSimbolo != IGUAL) return ERROR_SINTAXIS; /// ESCALABLE -> IGUAL, MAYOR, MENOR, DISTINTO

    if (campoEncontrado.tipo == TIPO_ENTERO) {
        if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_numeros, &valor, sizeof(int))) != ERROR_TODO_OK) return ret;
    } else if (campoEncontrado.tipo == TIPO_TEXTO) {
        buffer = (char*)malloc(campoEncontrado.tam);
        if (!buffer) return ERROR_SIN_MEMO;
        memset(buffer, 0, campoEncontrado.tam);
        if ((ret = _bdatos_parsear(&bDatos->secuencia, _bdatos_parsear_texto, buffer, campoEncontrado.tam)) != ERROR_TODO_OK) {
            free(buffer);
            return ret;
        }
    } else {
        return ERROR_CAMPO_INVALIDO;
    }

    registro = (char*)malloc(bDatos->encabezado.tamRegistro);
    if (!registro) {
        free(buffer);
        return ERROR_SIN_MEMO;
    }

    lista_crear(&listaResultados);

    if (campoEncontrado.esPK) {

        tIndice indiceReg;

        if (campoEncontrado.tipo == TIPO_ENTERO) {
            sprintf(indiceReg.clave, "%011d", valor);
        } else {
            strcpy(indiceReg.clave, buffer);
        }

        if (arbol_buscar(&bDatos->arbol, &indiceReg, sizeof(tIndice), _bdatos_cmp_indice) == ARBOL_TODO_OK) {

            fseek(bDatos->arch, indiceReg.offset, SEEK_SET);
            fread(registro, bDatos->encabezado.tamRegistro, 1, bDatos->arch);
            lista_insertar_final(&listaResultados, registro, bDatos->encabezado.tamRegistro);
        }
    } else {

        fseek(bDatos->arch, sizeof(tEncabezado), SEEK_SET);

        while (fread(registro, bDatos->encabezado.tamRegistro, 1, bDatos->arch) == 1) {

            int coincidencia = 0;

            if (campoEncontrado.tipo == TIPO_ENTERO) {
                if (*(int*)(registro + campoEncontrado.offsetCampo) == valor) {
                    coincidencia = 1;
                }
            } else {
                if (strncmp(registro + campoEncontrado.offsetCampo, buffer, strlen(buffer)) == 0) {
                    coincidencia = 1;
                }
            }

            if (coincidencia) lista_insertar_final(&listaResultados, registro, bDatos->encabezado.tamRegistro);
        }
    }

    while (lista_sacar_primero(&listaResultados, registro, bDatos->encabezado.tamRegistro) == LISTA_TODO_OK) {

        printf("SACADO DE LA LISTA\n");
    }

    lista_vaciar(&listaResultados);
    free(registro);
    free(buffer);

    return ERROR_TODO_OK;
}



static int _bdatos_cmp_indice(const void *a, const void *b)
{
    tIndice *indiceA = (tIndice*)a;
    tIndice *indiceB = (tIndice*)b;

    return strcmp(indiceA->clave, indiceB->clave);
}


int bdatos_cerrar(tBDatos *bDatos)
{
    char nombreIdx[TAM_NOMBRE_ARCH];

    if (bDatos->arch == NULL) {
        return TODO_OK;
    }

    sprintf(nombreIdx, "%s.idx", bDatos->encabezado.nombreTabla);
    printf("Cerrando tabla: %s\n", bDatos->encabezado.nombreTabla);

    _bdatos_guardar_idx(bDatos, nombreIdx);

    fseek(bDatos->arch, 0, SEEK_SET);
    fwrite(&bDatos->encabezado, sizeof(tEncabezado), 1, bDatos->arch);

    fclose(bDatos->arch);


    arbol_vaciar(&bDatos->arbol);
    memset(bDatos, 0, sizeof(tBDatos));

    return TODO_OK;
}

static int _bdatos_guardar_idx(tBDatos *bDatos, const char* nombreIdx)
{
    if (bDatos->archIdx != NULL) {
        fclose(bDatos->archIdx);
        bDatos->archIdx = NULL;
    }

    if (arbol_escribir_en_arch(&bDatos->arbol, nombreIdx) != ARBOL_TODO_OK) {

        return ERR_ARCHIVO;
    }

    return ERROR_TODO_OK;
}

static int _bdatos_cargar_idx(tBDatos *bDatos, const char* nombreIdx)
{
    fseek(bDatos->archIdx, 0, SEEK_END);
    long tamArch = ftell(bDatos->archIdx);
    rewind(bDatos->archIdx);

    if (tamArch == 0) {

        arbol_crear(&bDatos->arbol);
        return TODO_OK;
    }

    return arbol_cargar_de_archivo(&bDatos->arbol, nombreIdx, bDatos->encabezado.tamRegIdx, _bdatos_cmp_indice);
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

    return encontrado ? ERROR_TODO_OK : ERROR_CAMPO_INEXISTENTE;
}

static eSimbolo _bdatos_comparar_simbolo(const char* simbolo)
{
    if (strcmp(simbolo, "CREAR") == 0) {
        return CREAR;
    } else if (strcmp(simbolo, "ABRIR") == 0) {
        return ABRIR;
    } else if (strcmp(simbolo, "INSERTAR") == 0) {
        return INSERTAR;
    } else if (strcmp(simbolo, "ACTUALIZAR") == 0) {
        return ACTUALIZAR;
    } else if (strcmp(simbolo, "SELECCIONAR") == 0) {
        return SELECCIONAR;
    } else if (strcmp(simbolo, "TABLA") == 0) {
        return TABLA;
    } else if (strcmp(simbolo, "EN") == 0) {
        return EN;
    } else if (strcmp(simbolo, "DESDE") == 0) {
        return DESDE;
    } else if (strcmp(simbolo, "DONDE") == 0) {
        return DONDE;
    } else if (strcmp(simbolo, "IGUAL") == 0) {
        return IGUAL;
    } else if (strcmp(simbolo, "MAYOR") == 0) {
        return MAYOR;
    } else if (strcmp(simbolo, "MENOR") == 0) {
        return MENOR;
    } else if (strcmp(simbolo, "DISTINTO") == 0) {
        return DISTINTO;
    } else if (strcmp(simbolo, "ENTERO") == 0) {
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

static void _bdatos_crear_secuencia(tSecuencia *secuencia, const char *buffer)
{
    secuencia->cursor = (char*)buffer;
    secuencia->finSec = 0;
}

static int _bdatos_ejecutar_comando(tBDatos *bDatos, eSimbolo comando)
{
    switch (comando) {
        // Comandos
        case CREAR:
            return _bdatos_crear(bDatos);
        case ABRIR:
            return _bdatos_abrir(bDatos);
        case INSERTAR:
            return _bdatos_insertar(bDatos);
        case ACTUALIZAR:
            return _bdatos_dummy();
        case SELECCIONAR:
            return _bdatos_seleccionar(bDatos);
        default:
            break;
    }

    return ERROR_COMANDO;
}

static int _bdatos_dummy()
{
    return ERROR_TODO_OK;
}
