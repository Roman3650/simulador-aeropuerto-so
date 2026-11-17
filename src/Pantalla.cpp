#include "Pantalla.hpp"

#include <cstddef>
#include <cstdlib>
#include <ostream>
#include <string>

namespace Pantalla {

namespace {

constexpr std::size_t ancho = 50;

}

void limpiar(std::ostream& salida) {
    salida.flush();
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void marco(std::ostream& salida, std::string_view titulo) {
    const std::string borde(ancho, '-');
    salida << '+' << borde << "+\n";

    const std::size_t util = ancho - 4;
    const std::size_t longitud = titulo.size() > util ? util : titulo.size();
    const std::size_t espacios = util - longitud;
    const std::size_t izquierda = espacios / 2;
    const std::size_t derecha = espacios - izquierda;

    salida << "| " << std::string(izquierda, ' ')
           << titulo.substr(0, longitud)
           << std::string(derecha, ' ') << " |\n";
    salida << '+' << borde << "+\n\n";
}

void opcion(std::ostream& salida, int numero, std::string_view texto) {
    salida << "  " << numero << ") " << texto << '\n';
}

}
