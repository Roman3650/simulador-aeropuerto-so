#ifndef PAGINACION_HPP
#define PAGINACION_HPP

#include "Menu.hpp"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <mutex>
#include <string_view>
#include <vector>

struct PTE {
    int numeroPagina = -1;
    bool presente = false;
    int marcoFisico = -1;
    bool referenciado = false;
    bool modificado = false;
    std::size_t orden = 0;
};

struct MarcoFisico {
    int numeroPagina = -1;
    bool presente = false;
    bool referenciado = false;
    bool modificado = false;
    std::size_t orden = 0;
};

struct Traduccion {
    int pagina = 0;
    int desplazamiento = 0;
    std::uint16_t direccionFisica = 0;
    bool acierto = false;
};

struct MetricasPaginacion {
    std::size_t pageHits = 0;
    std::size_t pageFaults = 0;
};

std::vector<std::uint16_t> generarTraza(EscenarioCarga escenario,
                                        std::size_t cantidadPersonalizada = 0);
std::string_view nombreTraza(EscenarioCarga escenario);

class MemoriaRadar {
public:
    static constexpr std::size_t tamanoPagina = 1024;
    static constexpr std::size_t cantidadMarcos = 4;
    static constexpr unsigned bitsDireccion = 16;

    MemoriaRadar(AlgoritmoReemplazo algoritmo, EscenarioCarga escenario,
                 std::size_t cantidadPersonalizada = 0);

    void ejecutar(std::ostream& salida);
    Traduccion traducir(std::uint16_t direccionVirtual) const;
    MetricasPaginacion metricas() const;
    std::vector<PTE> tablaDePaginas() const;
    std::vector<MarcoFisico> marcos() const;

private:
    struct ResultadoFallo {
        std::size_t indiceMarco = 0;
        bool desdeSwap = false;
        bool escrituraSwap = false;
        int paginaVictima = -1;
    };

    void reiniciar();
    void procesarReferencia(std::uint16_t direccionVirtual, std::size_t paso,
                            std::size_t total, std::ostream& salida);
    Traduccion traducirBloqueado(std::uint16_t direccionVirtual) const;
    ResultadoFallo atenderFallo(int pagina, std::ostream& salida);
    std::size_t seleccionarMarcoLibre() const;
    std::size_t seleccionarVictima(std::ostream& salida);
    void marcarAcceso(int pagina);
    void imprimirTraza(const std::vector<std::uint16_t>& traza, std::ostream& salida) const;
    void imprimirMarcos(std::ostream& salida) const;
    void imprimirResumen(std::ostream& salida) const;

    AlgoritmoReemplazo algoritmo_;
    EscenarioCarga escenario_;
    std::size_t cantidadPersonalizada_ = 0;
    std::vector<PTE> tabla_;
    std::vector<MarcoFisico> marcos_;
    std::vector<int> colaFifo_;
    std::vector<int> swap_;
    std::size_t manecilla_ = 0;
    std::size_t reloj_ = 0;
    MetricasPaginacion metricas_;
    mutable std::mutex mutex_;
};

#endif
