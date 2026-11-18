#include "Menu.hpp"

#include "Paginacion.hpp"
#include "Pantalla.hpp"
#include "Sincronizacion.hpp"

#include <cstddef>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

namespace {

AlgoritmoReemplazo aAlgoritmo(int opcion) {
    switch (opcion) {
        case 1:
            return AlgoritmoReemplazo::FIFO;
        case 2:
            return AlgoritmoReemplazo::LRU;
        default:
            return AlgoritmoReemplazo::Reloj;
    }
}

EscenarioCarga aEscenario(int opcion) {
    switch (opcion) {
        case 1:
            return EscenarioCarga::Vuelos10;
        case 2:
            return EscenarioCarga::Vuelos50;
        default:
            return EscenarioCarga::Personalizado;
    }
}

}

void Menu::ejecutar(std::istream& entrada, std::ostream& salida) {
    while (ejecutarPrincipal(entrada, salida)) {
    }
}

bool Menu::ejecutarPrincipal(std::istream& entrada, std::ostream& salida) {
    Pantalla::limpiar(salida);
    Pantalla::marcoPrincipal(salida, "SIMULADOR AEROPUERTO MANUEL CARLOS PIAR");
    salida << "Carga de trafico actual: " << nombreEscenario(configuracion_.escenario);
    if (configuracion_.escenario == EscenarioCarga::Personalizado) {
        salida << " (" << configuracion_.cantidadPersonalizada << " vuelos)";
    }
    salida << "\n\n";
    Pantalla::opcionPlana(salida, 1, "Configurar carga de vuelos");
    Pantalla::opcionPlana(salida, 2, "Ejecutar Simulacion de Pistas y Concurrencia");
    Pantalla::opcionPlana(salida, 3, "Seleccionar y Ejecutar Algoritmo de Paginacion");
    Pantalla::opcionPlana(salida, 4, "Salir");
    salida << '\n';

    const std::optional<int> opcion = leerEntero(entrada, salida, 1, 4);
    if (!opcion || *opcion == 4) {
        salida << "[MENU] Saliendo...\n";
        return false;
    }

    switch (*opcion) {
        case 1:
            configurarEscenario(entrada, salida);
            break;
        case 2:
            ejecutarSincronizacion(entrada, salida);
            break;
        case 3:
            seleccionarAlgoritmo(entrada, salida);
            break;
    }
    return true;
}

void Menu::seleccionarAlgoritmo(std::istream& entrada, std::ostream& salida) {
    while (true) {
        Pantalla::limpiar(salida);
        Pantalla::marco(salida, "ALGORITMO DE REEMPLAZO");
        salida << "  Algoritmo actual: " << nombreAlgoritmo(configuracion_.algoritmo) << "\n\n";
        Pantalla::opcion(salida, 1, "FIFO");
        Pantalla::opcion(salida, 2, "LRU");
        Pantalla::opcion(salida, 3, "Reloj (segunda oportunidad)");
        Pantalla::opcion(salida, 4, "Ejecutar");
        Pantalla::opcion(salida, 5, "Volver atras");
        salida << '\n';

        const std::optional<int> opcion = leerEntero(entrada, salida, 1, 5);
        if (!opcion || *opcion == 5) {
            return;
        }
        if (*opcion == 4) {
            ejecutarPaginacion(entrada, salida);
            return;
        }
        configuracion_.algoritmo = aAlgoritmo(*opcion);
    }
}

void Menu::configurarEscenario(std::istream& entrada, std::ostream& salida) {
    Pantalla::limpiar(salida);
    Pantalla::marco(salida, "ESCENARIO DE CARGA");
    salida << "  Escenario actual: " << nombreEscenario(configuracion_.escenario);
    if (configuracion_.escenario == EscenarioCarga::Personalizado) {
        salida << " (" << configuracion_.cantidadPersonalizada << " vuelos)";
    }
    salida << "\n\n";
    Pantalla::opcion(salida, 1, "10 vuelos");
    Pantalla::opcion(salida, 2, "50 vuelos (estres)");
    Pantalla::opcion(salida, 3, "Personalizada");
    Pantalla::opcion(salida, 4, "Volver atras");
    salida << '\n';

    const std::optional<int> opcion = leerEntero(entrada, salida, 1, 4);
    if (!opcion || *opcion == 4) {
        return;
    }
    configuracion_.escenario = aEscenario(*opcion);
    if (configuracion_.escenario == EscenarioCarga::Personalizado) {
        const std::optional<int> cantidad = leerEntero(entrada, salida, 1, 500);
        if (!cantidad) {
            return;
        }
        configuracion_.cantidadPersonalizada = static_cast<std::size_t>(*cantidad);
    }
}

void Menu::ejecutarSincronizacion(std::istream& entrada, std::ostream& salida) {
    Pantalla::limpiar(salida);
    Pantalla::marco(salida, "PISTAS Y CONCURRENCIA");
    salida << "  Vuelos: " << cantidadVuelos(configuracion_) << '\n';
    salida << "  Unica pista con monitor TorreControl, mutex y condition_variable\n\n";
    salida.flush();

    ejecutarSimulacionPistas(cantidadVuelos(configuracion_));

    salida << '\n';
    salida << "Presione ENTER para volver al menu...";
    salida.flush();

    std::string linea;
    std::getline(entrada, linea);
}

void Menu::ejecutarPaginacion(std::istream& entrada, std::ostream& salida) {
    Pantalla::limpiar(salida);
    Pantalla::marco(salida, "PAGINACION");
    salida << "  Algoritmo: " << nombreAlgoritmo(configuracion_.algoritmo) << '\n';
    salida << "  Escenario: " << nombreEscenario(configuracion_.escenario) << "\n\n";
    MemoriaRadar memoria(configuracion_.algoritmo, configuracion_.escenario,
                         configuracion_.cantidadPersonalizada);
    memoria.ejecutar(salida);
    salida << '\n';
    salida << "Presione ENTER para volver al menu...";
    salida.flush();

    std::string linea;
    std::getline(entrada, linea);
}

std::optional<int> Menu::leerEntero(std::istream& entrada, std::ostream& salida,
                                    int minimo, int maximo) {
    std::string linea;
    while (true) {
        salida << "Opcion:";
        salida.flush();
        if (!std::getline(entrada, linea)) {
            return std::nullopt;
        }

        std::istringstream flujo(linea);
        int opcion = 0;
        if (flujo >> opcion && (flujo >> std::ws).eof()
            && opcion >= minimo && opcion <= maximo) {
            return opcion;
        }

        salida << "[MENU] Opcion invalida. Intente de nuevo.\n";
    }
}

std::string_view nombreAlgoritmo(AlgoritmoReemplazo algoritmo) {
    switch (algoritmo) {
        case AlgoritmoReemplazo::FIFO:
            return "FIFO";
        case AlgoritmoReemplazo::LRU:
            return "LRU";
        case AlgoritmoReemplazo::Reloj:
            return "Reloj";
    }
    return "Desconocido";
}

std::string_view nombreEscenario(EscenarioCarga escenario) {
    switch (escenario) {
        case EscenarioCarga::Vuelos10:
            return "10 vuelos";
        case EscenarioCarga::Vuelos50:
            return "50 vuelos (estres)";
        case EscenarioCarga::Personalizado:
            return "Personalizada";
    }
    return "Desconocido";
}

std::size_t cantidadVuelos(const ConfiguracionMenu& configuracion) {
    switch (configuracion.escenario) {
        case EscenarioCarga::Vuelos10:
            return 10;
        case EscenarioCarga::Vuelos50:
            return 50;
        case EscenarioCarga::Personalizado:
            return configuracion.cantidadPersonalizada;
    }
    return 10;
}
