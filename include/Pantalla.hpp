#ifndef PANTALLA_HPP
#define PANTALLA_HPP

#include <iosfwd>
#include <string_view>

namespace Pantalla {

void limpiar(std::ostream& salida);
void marco(std::ostream& salida, std::string_view titulo);
void marcoPrincipal(std::ostream& salida, std::string_view titulo);
void opcion(std::ostream& salida, int numero, std::string_view texto);
void opcionPlana(std::ostream& salida, int numero, std::string_view texto);

}

#endif
