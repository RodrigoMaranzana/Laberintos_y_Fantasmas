#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/servidor/bdatos.h"
#include "../../include/comun/comun.h"
#include "../../include/comun/arbol.h"

static int _bdatos_ejecutar_comando(tBDatos *bDatos, eSimbolo comando);
static int _bdatos_insertar(tBDatos *bDatos);
static int _bdatos_crear(tBDatos *bDatos);

static int _bdatos_dummy();
static void _bdatos_crear_secuencia(tSecuencia *secuencia, const char *buffer);
static eSimbolo _bdatos_comparar_simbolo(const char* simbolo);
static eSimbolo _bdatos_parsear_simbolo(tSecuencia *secuencia);
static int _bdatos_parsear_identificador(tSecuencia *secuencia, char *identificador, unsigned tamIden);
static int _bdatos_parsear_caracter(tSecuencia *secuencia, char caracter);
static eTipoDato _bdatos_convertir_simbolo_a_tipo(eSimbolo simbolo);
static int _bdatos_tabla_existe(const char *idenTabla);
static int _bdatos_escribir_encabezado(FILE *arch, const char *nombreTabla, int cantCampos, tCampo *campos, int cantPK);
static int _bdatos_parsear_campos(tSecuencia *secuencia, tCampo *campos, int maxCampos);
static int _bdatos_parsear_tipo_campo(tSecuencia *secuencia, tCampo *campo);
static int _bdatos_crear_archivo_tabla(tBDatos *bDatos, const char *idenTabla, tCampo *campos, int cantCampos, int cantPK);
static int _bdatos_parsear_campos_y_valores(tSecuencia *secuencia, const tEncabezado *encabezado, tDatoParseado *datosParseados, int *cantParseados);
static int _bdatos_buscar_campo(const tCampo *campos, int cantCampos, tCampo *campoEncontrado, const char *nombreCampoLeido);

int bdatos_iniciar(tBDatos *bDatos)
{
    memset(bDatos, 0, sizeof(tBDatos));
    arbol_crear(&bDatos->arbol);

    return TODO_OK;
}

int bdatos_procesar_solcitud(tBDatos *bDatos, const char *solicitud)
{
    int retorno = TODO_OK;
    eSimbolo simbolo;
    _bdatos_crear_secuencia(&bDatos->secuencia, solicitud);

    simbolo = _bdatos_parsear_simbolo(&bDatos->secuencia);
    if (!(simbolo >= CREAR && simbolo <= SELECCIONAR)) {

        return ERROR_COMANDO;
    }

    retorno = _bdatos_ejecutar_comando(bDatos, simbolo);

    return retorno;
}


static int _bdatos_parsear_identificador(tSecuencia *secuencia, char *identificador, unsigned tamIden)
{
    char *pIdentificador = identificador;

    while(*secuencia->cursor && !ES_LETRA(*secuencia->cursor)) {

        secuencia->cursor++;
    }

    if (!*secuencia->cursor) {

        *pIdentificador = '\0';
        return ERROR_SINTAXIS;
    }

    while (*secuencia->cursor && ES_LETRA(*secuencia->cursor) && (pIdentificador - identificador) < (tamIden - 1)) {

        *pIdentificador++ = *secuencia->cursor++;
    }

    *pIdentificador = '\0';

    return TODO_OK;
}



static eSimbolo _bdatos_parsear_simbolo(tSecuencia *secuencia)
{
    char comando[TAM_SIMBOLO] = {0}, *pComando = comando;

    while(*secuencia->cursor && !ES_LETRA(*secuencia->cursor)) {

        secuencia->cursor++;
    }

    while (*secuencia->cursor && ES_LETRA(*secuencia->cursor)) {

        *pComando++ = *secuencia->cursor++;
    }

    *pComando = '\0';

    return _bdatos_comparar_simbolo(comando);
}


static int _bdatos_parsear_caracter(tSecuencia *secuencia, char caracter)
{
    while(*secuencia->cursor && *secuencia->cursor == ' ') {

        secuencia->cursor++;
    }

    if (*secuencia->cursor != caracter) {

        return ERROR_SINTAXIS;
    }

    secuencia->cursor++;

    return TODO_OK;
}


static int _bdatos_parsear_numeros(tSecuencia *secuencia, int *numero)
{
    char buffer[TAM_INT_TEXTO], *pBuffer = buffer;

    while(*secuencia->cursor && *secuencia->cursor == ' ') {

        secuencia->cursor++;
    }

    if (!*secuencia->cursor || !ES_DIGITO(*secuencia->cursor)) {

        return ERROR_SINTAXIS;
    }

    while(*secuencia->cursor && ES_DIGITO(*secuencia->cursor) && (pBuffer - buffer) < (TAM_INT_TEXTO - 1)) {

        *pBuffer++ = *secuencia->cursor++;
    }

    *pBuffer = '\0';
    sscanf(buffer, "%d", numero);

    return TODO_OK;
}


static int _bdatos_tabla_existe(const char *idenTabla)
{
    char nombreArchivo[TAM_IDENTIFICADOR + 4];
    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.dat", idenTabla);

    FILE *arch = fopen(nombreArchivo, "r");
    if (arch) {

        fclose(arch);
        return 1;
    }
    return 0;
}

static int _bdatos_parsear_campos(tSecuencia *secuencia, tCampo *campos, int maxCampos)
{
    int i = 0;
    int fin = 0;
    char nombreCampo[TAM_IDENTIFICADOR];

    do {
        if (i >= maxCampos) return ERROR_DEMASIADOS_CAMPOS;

        if (_bdatos_parsear_identificador(secuencia, nombreCampo, TAM_IDENTIFICADOR) != TODO_OK) return ERROR_SINTAXIS;

        if (strcmp(nombreCampo, "PK") == 0) {
            fin = 1;
        } else {
            strcpy(campos[i].nombre, nombreCampo);
            campos[i].esPk = 0;

            if (_bdatos_parsear_tipo_campo(secuencia, &campos[i]) != TODO_OK) return ERROR_SINTAXIS;

            i++;

            if (_bdatos_parsear_caracter(secuencia, ',') != TODO_OK) {
                fin = 1;
            }
        }
    } while (!fin);

    return i;
}

static int _bdatos_parsear_tipo_campo(tSecuencia *secuencia, tCampo *campo)
{
    eSimbolo proximoSimbolo = _bdatos_parsear_simbolo(secuencia);
    campo->tipo = _bdatos_convertir_simbolo_a_tipo(proximoSimbolo);

    if (campo->tipo == TIPO_INVALIDO) return ERROR_SINTAXIS;

    if (campo->tipo == TIPO_ENTERO) {
        campo->tam = sizeof(int);
    } else if (campo->tipo == TIPO_FLOTANTE) {
        campo->tam = sizeof(float);
    } else if (campo->tipo == TIPO_TEXTO) {
        int numero;
        if (_bdatos_parsear_caracter(secuencia, '(') != TODO_OK) return ERROR_SINTAXIS;
        if (_bdatos_parsear_numeros(secuencia, &numero) != TODO_OK) return ERROR_SINTAXIS;
        if (_bdatos_parsear_caracter(secuencia, ')') != TODO_OK) return ERROR_SINTAXIS;
        campo->tam = numero;
    }
    return TODO_OK;
}

static int _bdatos_parsear_pk(tSecuencia *secuencia, char idenPK[][TAM_IDENTIFICADOR], int maxPK)
{
    int cantPK = 0;
    int fin = 0;

    if (_bdatos_parsear_caracter(secuencia, '(') != TODO_OK) return ERROR_SINTAXIS;

    do {
        if (cantPK >= maxPK) return ERROR_DEMASIADOS_PK;
        if (_bdatos_parsear_identificador(secuencia, idenPK[cantPK], TAM_IDENTIFICADOR) != TODO_OK) return ERROR_SINTAXIS;

        cantPK++;

        if (_bdatos_parsear_caracter(secuencia, ',') != TODO_OK) {
            fin = 1;
        }
    } while (!fin);

    if (_bdatos_parsear_caracter(secuencia, ')') != TODO_OK) return ERROR_SINTAXIS;

    return cantPK;
}

static int _bdatos_validar_pk(tCampo *campos, int cantCampos, const char idenPK[][TAM_IDENTIFICADOR], int cantPK)
{
    for (int pk = 0; pk < cantPK; pk++) {

        int encontrado = 0;

        for (int campo = 0; campo < cantCampos && !encontrado; campo++) {

            if (strcmp(campos[campo].nombre, idenPK[pk]) == 0) {

                campos[campo].esPk = 1;
                encontrado = 1;
            }
        }

        if (!encontrado) return ERROR_PK_INEXISTENTE;
    }

    return TODO_OK;
}

static int _bdatos_crear_archivo_tabla(tBDatos *bDatos, const char *idenTabla, tCampo *campos, int cantCampos, int cantPK)
{
    char nombreArchivo[TAM_IDENTIFICADOR + 4];
    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.dat", idenTabla);

    bDatos->arch = fopen(nombreArchivo, "w+b");
    if (!bDatos->arch) {

        bDatos->archIdx = NULL;
        return ERR_ARCHIVO;
    }

    return _bdatos_escribir_encabezado(bDatos->arch, idenTabla, cantCampos, campos, cantPK);
}

static int _bdatos_escribir_encabezado(FILE *arch, const char *nombreTabla, int cantCampos, tCampo *campos, int cantPK)
{
    tEncabezado encabezado;
    int i;

    memset(&encabezado, 0, sizeof(tEncabezado));

    encabezado.cantCampos = cantCampos;
    encabezado.cantPK = cantPK;
    encabezado.cantRegistros = 0;
    encabezado.tamRegistro = 0;
    strcpy(encabezado.nombreTabla, nombreTabla);

    for (i = 0; i < cantCampos; i++) {

        strncpy(encabezado.campos[i].nombre, campos[i].nombre, TAM_IDENTIFICADOR);
        encabezado.campos[i].tipo = campos[i].tipo;
        encabezado.campos[i].tam = campos[i].tam;
        encabezado.campos[i].esPk = campos[i].esPk;
        encabezado.tamRegistro += campos[i].tam;
    }

    fseek(arch, 0, SEEK_SET);

    if (fwrite(&encabezado, sizeof(tEncabezado), 1, arch) != 1) {

        fclose(arch);
        return ERROR_ESCRITURA;
    }

    fclose(arch);
    return TODO_OK;
}

static int _bdatos_leer_encabezado(const char *nombreTabla, tEncabezado *encabezado) {

    char nombreArchivo[TAM_IDENTIFICADOR + 4];
    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.dat", nombreTabla);

    FILE* arch = fopen(nombreArchivo, "r+b");
    if (!arch) {

        return ERROR_ARCH;
    }

    if (fread(encabezado, sizeof(tEncabezado), 1, arch) != 1) {

        fclose(arch);
        return ERROR_LECTURA;
    }

    fclose(arch);
    return TODO_OK;
}


static eTipoDato _bdatos_convertir_simbolo_a_tipo(eSimbolo simbolo)
{
    switch (simbolo) {
        case ENTERO:
            return TIPO_ENTERO;
        case FLOTANTE:
            return TIPO_FLOTANTE;
        case TEXTO:
            return TIPO_TEXTO;
        default:
            return TIPO_INVALIDO;
    }
}

/// CREAR TABLA jugadores (idJugador ENTERO, nombre TEXTO(16), puntajeMax ENTERO, PK(idJugador))
/// CREAR TABLA partidas (idPartida ENTERO, puntaje ENTERO, PK(idJugador, idPartida))
static int _bdatos_crear(tBDatos *bDatos)
{
    tCampo campos[MAX_CAMPOS_POR_TABLA];
    char idenPK[MAX_PK_POR_TABLA][TAM_IDENTIFICADOR] = {0};
    char idenTabla[TAM_IDENTIFICADOR] = {0};
    int cantCampos, cantPK, retorno;
    eSimbolo proximoSimbolo;

    proximoSimbolo = _bdatos_parsear_simbolo(&bDatos->secuencia);
    if (proximoSimbolo != TABLA) return ERROR_SINTAXIS;
    if (_bdatos_parsear_identificador(&bDatos->secuencia, idenTabla, TAM_IDENTIFICADOR) != TODO_OK) return ERROR_SINTAXIS;
    if (_bdatos_tabla_existe(idenTabla)) return ERROR_TABLA_YA_EXISTE;
    if (_bdatos_parsear_caracter(&bDatos->secuencia, '(') != TODO_OK) return ERROR_SINTAXIS;

    cantCampos = _bdatos_parsear_campos(&bDatos->secuencia, campos, MAX_CAMPOS_POR_TABLA);
    if (cantCampos < 0) return ERROR_CANT_CAMPOS;

    cantPK = _bdatos_parsear_pk(&bDatos->secuencia, idenPK, MAX_PK_POR_TABLA);
    if (cantPK > 3) return ERROR_DEMASIADOS_PK;
    if (cantPK == 0) return ERROR_SIN_PK;

    retorno = _bdatos_validar_pk(campos, cantCampos, idenPK, cantPK);
    if (retorno != TODO_OK) return retorno;

    printf("CREAR TABLA -> Tabla: %s, Campos: %d, PKs: %d\n", idenTabla, cantCampos, cantPK);

    return _bdatos_crear_archivo_tabla(bDatos, idenTabla, campos, cantCampos, cantPK);
}

/// INSERTAR EN jugadores (idJugador 1000, nombre RODRIGO, puntajeMax 21)
static int _bdatos_insertar(tBDatos *bDatos)
{
    tDatoParseado datosParseados[MAX_CAMPOS_POR_TABLA] = {0};
    char idenTabla[TAM_IDENTIFICADOR] = {0}, *registro;
    int cantParseados = 0, i, offset, pkProporcionadas, retorno;
    eSimbolo proximoSimbolo;

    proximoSimbolo = _bdatos_parsear_simbolo(&bDatos->secuencia);
    if (proximoSimbolo != EN) return ERROR_SINTAXIS;
    if (_bdatos_parsear_identificador(&bDatos->secuencia, idenTabla, TAM_IDENTIFICADOR) != TODO_OK) return ERROR_SINTAXIS;
    if (!_bdatos_tabla_existe(idenTabla)) return ERROR_TABLA_NO_EXISTE;
    if (_bdatos_parsear_caracter(&bDatos->secuencia, '(') != TODO_OK) return ERROR_SINTAXIS;

    retorno = _bdatos_parsear_campos_y_valores(&bDatos->secuencia, &bDatos->encabezado, datosParseados, &cantParseados);
    if (retorno != TODO_OK) {

        for (i = 0; i < cantParseados; i++) {

            free(datosParseados[i].valor.dato);
        }
        return retorno;
    }

    registro = (char*)malloc(bDatos->encabezado.tamRegistro);
    if(!registro) return ERROR_SIN_MEMO;
    memset(registro, 0, bDatos->encabezado.tamRegistro);

    offset = 0;
    pkProporcionadas = 0;
    for (int i = 0; i < bDatos->encabezado.cantCampos; i++) {

        int encontrado = 0;

        for (int j = 0; j < cantParseados && !encontrado; j++) {

            if (strcmp(bDatos->encabezado.campos[i].nombre, datosParseados[j].campo.nombre) == 0) {

                memcpy(registro + offset, datosParseados[j].valor.dato, datosParseados[j].valor.tam);
                if (datosParseados[j].campo.esPk) pkProporcionadas++;
                encontrado = 1;
            }
        }

        offset += bDatos->encabezado.campos[i].tam;
    }

    if (pkProporcionadas != bDatos->encabezado.cantPK) {

        for (i = 0; i < cantParseados; i++) {

            free(datosParseados[i].valor.dato);
        }

        free(registro);
        return ERROR_SIN_PK;
    }


    /// BUSCAR EN EL ARBOL SI LA PK YA ESTA INSERTADA


    fseek(bDatos->arch, 0, SEEK_END);
    fwrite(registro, bDatos->encabezado.tamRegistro, 1, bDatos->arch);

    bDatos->encabezado.cantRegistros++;

    for (i = 0; i < cantParseados; i++) {

        free(datosParseados[i].valor.dato);
    }

    free(registro);

    return TODO_OK;
}

/// ABRIR TABLA jugadores
static int _bdatos_abrir(tBDatos *bDatos)
{
    int retorno;
    FILE *nuevoArch;
    tEncabezado nuevoEncabezado;
    char idenTabla[TAM_IDENTIFICADOR] = {0}, nombreArchivo[TAM_IDENTIFICADOR + 4];
    eSimbolo proximoSimbolo;

    proximoSimbolo = _bdatos_parsear_simbolo(&bDatos->secuencia);
    if (proximoSimbolo != TABLA) return ERROR_SINTAXIS;
    if (_bdatos_parsear_identificador(&bDatos->secuencia, idenTabla, TAM_IDENTIFICADOR) != TODO_OK) return ERROR_SINTAXIS;
    if (!_bdatos_tabla_existe(idenTabla)) return ERROR_TABLA_NO_EXISTE;

    snprintf(nombreArchivo, sizeof(nombreArchivo), "%s.dat", idenTabla);
    nuevoArch = fopen(nombreArchivo, "r+b");
    if (!nuevoArch) {

        return ERROR_ARCH;
    }

    retorno = _bdatos_leer_encabezado(idenTabla, &nuevoEncabezado);
    if (retorno != TODO_OK) return retorno;

    if (bDatos->arch != NULL) {

        printf("Cerrando tabla: %s\n", bDatos->encabezado.nombreTabla);

        fseek(bDatos->arch, 0, SEEK_SET);
        fwrite(&bDatos->encabezado, sizeof(tEncabezado), 1, bDatos->arch);
        fclose(bDatos->arch);
    }

    bDatos->arch = nuevoArch;
    bDatos->encabezado = nuevoEncabezado;

    printf("Tabla '%s' abierta correctamente.\n", bDatos->encabezado.nombreTabla);

    return TODO_OK;
}

///ACTUALIZAR EN jugadores (puntajeMax 25) DONDE idJugador IGUAL 21")
static int _bdatos_actualizar(tBDatos *bDatos)
{
    return TODO_OK;
}

int bdatos_cerrar(tBDatos *bDatos)
{
    if (bDatos->arch != NULL) {

        printf("Cerrando tabla: %s\n", bDatos->encabezado.nombreTabla);

        fseek(bDatos->arch, 0, SEEK_SET);
        fwrite(&bDatos->encabezado, sizeof(tEncabezado), 1, bDatos->arch);

        fclose(bDatos->arch);
        bDatos->arch = NULL;
        memset(&bDatos->encabezado, 0, sizeof(tEncabezado));
    }

    /// GRABAR Y LIBERAR ARBOL DEL IDX

    return TODO_OK;
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

    return encontrado ? TODO_OK : ERROR_CAMPO_INEXISTENTE;
}

static int _bdatos_parsear_campos_y_valores(tSecuencia *secuencia, const tEncabezado *encabezado, tDatoParseado *datosParseados, int *cantParseados)
{
    int i = 0, fin = 0, retorno;
    char nombreCampoLeido[TAM_IDENTIFICADOR];
    tCampo campoEncontrado;

    do {
        if (i >= encabezado->cantCampos) return ERROR_DEMASIADOS_CAMPOS;

        if ((retorno = _bdatos_parsear_identificador(secuencia, nombreCampoLeido, TAM_IDENTIFICADOR)) != TODO_OK) return retorno;

        if ((retorno = _bdatos_buscar_campo(encabezado->campos, encabezado->cantCampos, &campoEncontrado, nombreCampoLeido)) != TODO_OK) return retorno;

        memcpy(&datosParseados[i].campo, &campoEncontrado, sizeof(tCampo));

        switch (campoEncontrado.tipo) {
            case TIPO_ENTERO: {

                int valorInt;
                if ((retorno = _bdatos_parsear_numeros(secuencia, &valorInt)) != TODO_OK) return retorno;

                datosParseados[i].valor.dato = malloc(sizeof(int));
                if (!datosParseados[i].valor.dato) return ERROR_SIN_MEMO;

                memcpy(datosParseados[i].valor.dato, &valorInt, sizeof(int));
                datosParseados[i].valor.tam = sizeof(int);
                break;
            }
            case TIPO_TEXTO: {

                char textoLeido[campoEncontrado.tam];
                if ((retorno = _bdatos_parsear_identificador(secuencia, textoLeido, campoEncontrado.tam)) != TODO_OK) return retorno; /// MODIFICAR _bdatos_parsear_texto()

                datosParseados[i].valor.dato = malloc(campoEncontrado.tam);
                if (!datosParseados[i].valor.dato) return ERROR_SIN_MEMO;

                strncpy((char*)datosParseados[i].valor.dato, textoLeido, campoEncontrado.tam);
                datosParseados[i].valor.tam = campoEncontrado.tam;
                break;
            }
            default:
                return ERROR_CAMPO_INVALIDO;
        }

        if (retorno != TODO_OK) return retorno;

        i++;

        if (_bdatos_parsear_caracter(secuencia, ',') != TODO_OK) {

            fin = 1;
        }


    } while (!fin);

    if (_bdatos_parsear_caracter(secuencia, ')') != TODO_OK) return ERROR_SINTAXIS;

    *cantParseados = i;

    return TODO_OK;
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
    } else if (strcmp(simbolo, "DONDE") == 0) {
        return DONDE;
    } else if (strcmp(simbolo, "TODO") == 0) {
        return TODO;
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
    } else if (strcmp(simbolo, "FLOTANTE") == 0) {
        return FLOTANTE;
    } else if (strcmp(simbolo, "TEXTO") == 0) {
        return TEXTO;
    } else if (strcmp(simbolo, "PK") == 0) {
        return PK;
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
            return _bdatos_dummy();
        default:
            break;
    }

    return ERROR_COMANDO;
}

static int _bdatos_dummy()
{
    return TODO_OK;
}
