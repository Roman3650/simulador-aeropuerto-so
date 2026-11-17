#include "Menu.hpp"

#include "Pantalla.hpp"

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
    return opcion == 1 ? EscenarioCarga::Vuelos10 : EscenarioCarga::Vuelos50;
}

}

void Menu::ejecutar(std::istream& entrada, std::ostream& salida) {
    while (ejecutarPrincipal(entrada, salida)) {
    }
}

bool Menu::ejecutarPrincipal(std::istream& entrada, std::ostream& salida) {
    Pantalla::limpiar(salida);
    Pantalla::marco(salida, "SIMULADOR AEROPUERTO MANUEL CARLOS PIAR");
    salida << "  Algoritmo actual: " << nombreAlgoritmo(configuracion_.algoritmo) << '\n';
    salida << "  Escenario actual: " << nombreEscenario(configuracion_.escenario) << "\n\n";
    Pantalla::opcion(salida, 1, "Seleccionar algoritmo");
    Pantalla::opcion(salida, 2, "Seleccionar escenario");
    Pantalla::opcion(salida, 3, "Iniciar simulacion");
    Pantalla::opcion(salida, 4, "Salir");
    salida << '\n';

    const std::optional<int> opcion = leerEntero(entrada, salida, 1, 4);
    if (!opcion || *opcion == 4) {
        salida << "[MENU] Saliendo...\n";
        return false;
    }

    switch (*opcion) {
        case 1:
            configurarAlgoritmo(entrada, salida);
            break;
        case 2:
            configurarEscenario(entrada, salida);
            break;
        case 3:
            iniciarSimulacion(entrada, salida);
            break;
    }
    return true;
}

void Menu::configurarAlgoritmo(std::istream& entrada, std::ostream& salida) {
    Pantalla::limpiar(salida);
    Pantalla::marco(salida, "ALGORITMO DE REEMPLAZO");
    salida << "  Algoritmo actual: " << nombreAlgoritmo(configuracion_.algoritmo) << "\n\n";
    Pantalla::opcion(salida, 1, "FIFO");
    Pantalla::opcion(salida, 2, "LRU");
    Pantalla::opcion(salida, 3, "Reloj (segunda oportunidad)");
    Pantalla::opcion(salida, 4, "Volver atras");
    salida << '\n';

    const std::optional<int> opcion = leerEntero(entrada, salida, 1, 4);
    if (!opcion || *opcion == 4) {
        return;
    }
    configuracion_.algoritmo = aAlgoritmo(*opcion);
}

void Menu::configurarEscenario(std::istream& entrada, std::ostream& salida) {
    Pantalla::limpiar(salida);
    Pantalla::marco(salida, "ESCENARIO DE CARGA");
    salida << "  Escenario actual: " << nombreEscenario(configuracion_.escenario) << "\n\n";
    Pantalla::opcion(salida, 1, "10 vuelos");
    Pantalla::opcion(salida, 2, "50 vuelos (estres)");
    Pantalla::opcion(salida, 3, "Volver atras");
    salida << '\n';

    const std::optional<int> opcion = leerEntero(entrada, salida, 1, 3);
    if (!opcion || *opcion == 3) {
        return;
    }
    configuracion_.escenario = aEscenario(*opcion);
}

void Menu::iniciarSimulacion(std::istream& entrada, std::ostream& salida) {
    Pantalla::limpiar(salida);
    Pantalla::marco(salida, "SIMULACION");
    salida << "  Algoritmo: " << nombreAlgoritmo(configuracion_.algoritmo) << '\n';
    salida << "  Escenario: " << nombreEscenario(configuracion_.escenario) << "\n\n";
    salida << "[SIMULACION] Inicio simulado (modulo pendiente).\n\n";
    salida << "Presione ENTER para volver al menu...";
    salida.flush();

    std::string linea;
    std::getline(entrada, linea);
}

std::optional<int> Menu::leerEntero(std::istream& entrada, std::ostream& salida,
                                    int minimo, int maximo) {
    std::string linea;
    while (true) {
        salida << "Opcion: ";
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
    }
    return "Desconocido";
}
