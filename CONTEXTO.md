# AGENTS.md - Contexto y Reglas del Proyecto

## 1. Descripción General
- **Proyecto:** "Sistema Automatizado para control de las Operaciones del Aeropuerto Manuel Carlos Piar" (Puerto Ordaz, Venezuela)[cite: 1].
- **Objetivo:** Simulador multihilo en tiempo real en consola desarrollado en **C++ (estándar C++20)** que modela los subsistemas operativos de un aeropuerto inteligente[cite: 1, 2].
- **Entorno y Compilación:** Windows con MinGW-w64 (`g++ -std=c++20 -pthread -Iinclude src/*.cpp -o aeropuerto.exe`).

## 2. Alcance Estricto: Segunda Entrega (Fase Actual)
El proyecto se divide en tres entregas. **Actualmente estamos en el Entregable 2 ("Cerebro y RAM")**[cite: 6].
- **INCLUIDO EN ESTA ENTREGA:**
  1. **Motor de Sincronización y Concurrencia (Módulo 2):** Gestión de hilos por aeronave[cite: 1], exclusión mutua en pistas con `std::mutex`, `std::unique_lock` y `std::condition_variable`[cite: 2, 7], problema Productor-Consumidor con buffer circular finito y semáforos (`<semaphore>` o `sem_t`)[cite: 2, 7], prevención de inanición mediante envejecimiento (*aging*)[cite: 2, 7] y prevención de *deadlocks*[cite: 2, 6].
  2. **Administrador de Memoria Virtual del Radar (Módulo 3):** Tabla de páginas y marcos físicos de tamaño fijo[cite: 2, 7], traducción de direcciones virtuales a físicas[cite: 2, 7], manejo de fallos de página (*Page Faults*) con envío a espacio de intercambio (*Swap*) simulado[cite: 3, 7], y los 3 algoritmos de reemplazo: **FIFO**, **LRU** y **Algoritmo del Reloj (Segunda Oportunidad)**[cite: 3, 7].
- **REGLA DE ORO (PROHIBIDO IMPLEMENTAR AHORA):**
  - **NO** programar el subsistema de E/S ni cintas de equipaje (algoritmos de disco SSTF/SCAN)[cite: 6].
  - **NO** crear archivos físicos/binarios para persistencia de bitácora[cite: 6].
  - **NO** programar el sistema de inicio de sesión (*login*) ni la matriz de roles[cite: 6].
  *(Todo lo anterior pertenece exclusivamente al Entregable 3)[cite: 6].*

## 3. Arquitectura y Organización de Archivos
El código debe mantenerse estrictamente modularizado para permitir el desarrollo en paralelo por ramas Git:
- `include/` (Archivos de cabecera `.hpp` - Contratos públicos):
  - `Logger.hpp`: Sistema de impresión en consola *thread-safe* mediante `std::mutex`.
  - `Menu.hpp`: Interfaz de consola inicial para seleccionar algoritmo de reemplazo (`FIFO`, `LRU`, `Reloj`)[cite: 3, 7] y escenario de carga (ej. 10 vuelos vs. 50 vuelos en estrés)[cite: 7].
  - `Sincronizacion.hpp`: Clase `TorreControl` (Rama `feature/sincronizacion-pistas`).
  - `Paginacion.hpp`: Clase `MemoriaRadar` (Rama `feature/memoria-radar`).
- `src/` (Archivos de implementación `.cpp`):
  - `main.cpp`, `Logger.cpp`, `Menu.cpp`, `Sincronizacion.cpp`, `Paginacion.cpp`.

## 4. Convenciones de Código y Calidad
- **Concurrencia Segura:** Todo recurso compartido (pistas, buffer circular, tabla de páginas y salida `std::cout`) debe estar protegido contra condiciones de carrera[cite: 1].
- **Trazabilidad Visual (Obligatoria para Evidencias):** Cada evento crítico debe imprimir una traza clara y etiquetada en consola (ej. `[PISTA - MUTEX]`, `[RADAR - PROD]`, `[MEMORIA - HIT]`, `[PAGE FAULT - SWAP]`, `[TORRE - AGING]`)[cite: 8], ya que estas salidas son la evidencia directa para el informe técnico[cite: 8].
- **Métricas:** Al finalizar la ejecución, el módulo de memoria debe desplegar un resumen con el total de *Page Hits* y *Page Faults* para facilitar la comparación entre FIFO, LRU y Reloj[cite: 3, 7].