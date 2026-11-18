#include "Logger.hpp"

#include <iostream>
#include <mutex>

namespace Logger {

namespace {

std::mutex mutexSalida;

}

void imprimir(std::string_view mensaje) {
    std::lock_guard<std::mutex> bloqueo(mutexSalida);
    std::cout << mensaje << '\n';
    std::cout.flush();
}

}
