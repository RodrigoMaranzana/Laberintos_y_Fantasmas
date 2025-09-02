# Laberintos y Fantasmas

TP Algoritmos y Estructuras de Datos - Grupo Pixelados

**Convenciones de Nombres**

|                    | **Formato**                | **Ejemplos**                                                        |
| ------------------ | -------------------------- | ------------------------------------------------------------------- |
| Variable           | `pascalCase`               | `contadorFrames`, `indiceActual`                                    |
| Typedef            | `pascalCase` (prefijo `t`) | `tNodo`, `tPila`, `tJugador`                                        |
| Struct             | `pascalCase` (prefijo `s`) | `sNodo`                                                             |
| Enum               | `pascalCase` (prefijo `e`) | `eRetornos`, `eMotorEstado`                                         |
| Elementos del Enum | `SCREAMING_SNAKE_CASE``    | `SPRITE_JUGADOR`                                                    |
| Macro              | `SCREAMING_SNAKE_CASE`     | `CANT_PIXELES`, `TAM_VEC`                                           |
| Funcion            | `snake_case`               | `inicializar_motor()`, `movimiento_es_valido()`, `destruir_juego()` |
| Funcion Static     | `snake_case `(prefijo `_`) | `_puede_avanzar()`, `_calcular_trayectoria()`                       |

#### Librerias de terceros utilizadas

-  [SDL  2.32.8](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.8): **Simple DirectMedia Layer**.

-  [SDL2_image-2.8.8](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.8): Decodificador de formatos de imagen para **Simple DirectMedia Layer**.

- [SDL mixer 2.8.1](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.1): Mezclador de audio para **Simple DirectMedia Layer**. 

- [SDL ttf 2.24.0](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-2.24.0): Soporte de fuentes TrueType para **Simple DirectMedia Layer**.

##### Pasos para la configuración del entorno:

1. Descomprimir los siguientes archivos comprimidos en la carpeta `<raíz_del_proyecto>\libraries`:
   
   1. `SDL2-devel-2.32.8-mingw`
   
   2. `SDL2_image-devel-2.8.8-mingw.zip`
   
   3. `SDL2_mixer-devel-2.8.1-mingw.zip`
   
   4. `SDL2_ttf-devel-2.24.0-mingw.zip`

2. En Code::Blocks, click derecho sobre el nombre del proyecto, estando a la izquierda seleccionado debug:
   
   1. Acceda al tab `linker settings`: 
      
      1. En `Other linker options` agregue:
         
         - `-lmingw32`
         
         - `-lSDL2main`
         
         - `-lSDL2`
         
         - `-lSDL2_image`
         
         - `-lSDL2_ttf`
         
         - `-lSDL2_mixer`
   
   2. Acceda al tab `Search Directories`: 
      
      1. En el tab `Compiler` agregue:
         
         - `libraries\SDL2-2.32.8\x86_64-w64-mingw32\include\SDL2`.
         
         - `libraries\SDL2_image-2.8.8\x86_64-w64-mingw32\include\SDL2`.
         
         - `libraries\SDL2_mixer-2.8.1\x86_64-w64-mingw32\include\SDL2`.
         
         - `libraries\SDL2_ttf-2.24.0\x86_64-w64-mingw32\include\SDL2`.
      
      2. En el tab `Linker` agregue:
         
         - `libraries\SDL2-2.32.8\x86_64-w64-mingw32\include\SDL2`.
         
         - `libraries\SDL2_image-2.8.8\x86_64-w64-mingw32\include\SDL2`.
         
         - `libraries\SDL2_mixer-2.8.1\x86_64-w64-mingw32\include\SDL2`.
         
         - `libraries\SDL2_ttf-2.24.0\x86_64-w64-mingw32\include\SDL2`.

3. Pulse `OK` para guardar los cambios.

4. Copie las DLL necesarias en la carpeta donde Code::Blocks genera el `.exe`. Por defecto se guarda en `<raíz_del_proyecto>\bin\Debug\`:
   
   | Librería       | DLL a copiar     | Ruta de origen                                      |
   | -------------- | ---------------- | --------------------------------------------------- |
   | **SDL2**       | `SDL2.dll`       | `libraries\SDL2-2.32.8\x86_64-w64-mingw32\bin`      |
   | **SDL2_image** | `SDL2_image.dll` | `libraries\SDL2_image-2.8.8\x86_64-w64-mingw32\bin` |
   | **SDL2_ttf**   | `SDL2_ttf.dll`   | `libraries\SDL2_ttf-2.24.0\x86_64-w64-mingw32\bin`  |
   | **SDL2_mixer** | `SDL2_mixer.dll` | `libraries\SDL2_mixer-2.8.1\x86_64-w64-mingw32\bin` |
