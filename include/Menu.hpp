#ifndef MENU_HPP
#define MENU_HPP

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <string_view>

enum class AlgoritmoReemplazo {
    FIFO,
    LRU,
    Reloj
};

enum class EscenarioCarga {
    Vuelos10,
    Vuelos50,
    Personalizado
};

struct ConfiguracionMenu {
    AlgoritmoReemplazo algoritmo;
    EscenarioCarga escenario;
    std::size_t cantidadPersonalizada = 25;
};

class Menu {
public:
    void ejecutar(std::istream& entrada, std::ostream& salida);

private:
    ConfiguracionMenu configuracion_{AlgoritmoReemplazo::FIFO, EscenarioCarga::Vuelos10};

    bool ejecutarPrincipal(std::istream& entrada, std::ostream& salida);
    void configurarEscenario(std::istream& entrada, std::ostream& salida);
    void seleccionarAlgoritmo(std::istream& entrada, std::ostream& salida);
    void ejecutarSincronizacion(std::istream& entrada, std::ostream& salida);
    void ejecutarPaginacion(std::istream& entrada, std::ostream& salida);

    static std::optional<int> leerEntero(std::istream& entrada, std::ostream& salida,
                                         int minimo, int maximo);
};

std::string_view nombreAlgoritmo(AlgoritmoReemplazo algoritmo);
std::string_view nombreEscenario(EscenarioCarga escenario);
std::size_t cantidadVuelos(const ConfiguracionMenu& configuracion);

#endif
