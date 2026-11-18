# Simulador Aeropuerto Manuel Carlos Piar

Simulador multihilo en tiempo real en consola, desarrollado en **C++20**.
Entregable 2: "Cerebro y RAM" (sincronización de pistas + memoria virtual del radar).

## Requisitos

- **CMake 3.16** o superior
- Compilador con soporte **C++20** (`g++` en Linux, MinGW-w64 en Windows)
- Biblioteca de hilos (`pthread` en Linux; incluida en MinGW-w64)
- En Windows, abrir la terminal de **MinGW-w64 / MSYS2** para que `g++` esté en el `PATH`.
  Así CMake detecta el compilador automáticamente. No se instala nada extra.

## Estándar de compilación del equipo

**Un único flujo para todos.** No hay que elegir motor de build ni instalar herramientas adicionales.

Estando siempre en la **raíz del proyecto** (donde está `CMakeLists.txt`):

```bash
# 1. Configurar (una sola vez, la primera)
cmake -S . -B build

# 2. Compilar (cada vez que cambies código)
cmake --build build

# 3. Ejecutar (elegí según tu sistema)
./build/aeropuerto          # Linux
.\build\aeropuerto.exe      # Windows
```

Los dos primeros comandos son **idénticos en Linux y en Windows**. Solo cambia la
última línea según el sistema. `cmake --build` se encarga por detrás de llamar a
`make` en Linux y a `mingw32-make` en Windows.

Después de la primera configuración, para trabajar día a día alcanza con:

```bash
cmake --build build
```

Si aparece un error que dice `generator ... does not match the generator used previously`,
borrá la carpeta `build/` (`rm -rf build` en Linux, `rmdir /S /Q build` en Windows)
y repetí desde el paso 1. Es la única recuperación necesaria.

## Resultado esperado

Al ejecutar el programa se muestra un **menú interactivo en consola**.
Las opciones se eligen escribiendo su número y presionando Enter.

## Estructura del proyecto

- `include/` — cabeceras públicas (`.hpp`)
- `src/` — implementaciones (`.cpp`)
- `CMakeLists.txt` — receta de compilación
- `CONTEXTO.md` — reglas y alcance del proyecto

Los archivos `.cpp` que se agreguen dentro de `src/` (por ejemplo
`Sincronizacion.cpp`, `Paginacion.cpp` o `Logger.cpp`) se incluyen
**automáticamente** gracias al `CONFIGURE_DEPENDS` del `CMakeLists.txt`.
No hace falta editar la receta de compilación.
