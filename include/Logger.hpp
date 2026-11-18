#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>
#include <string_view>
#include <utility>

namespace Logger {

void imprimir(std::string_view mensaje);

template <typename... Partes>
void traza(Partes&&... partes) {
    std::ostringstream flujo;
    (flujo << ... << std::forward<Partes>(partes));
    imprimir(flujo.str());
}

}

#endif
