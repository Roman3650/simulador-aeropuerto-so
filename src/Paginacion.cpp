#include "Paginacion.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <string_view>
#include <vector>

namespace {

constexpr std::size_t cantidadPaginas = (std::size_t{1} << MemoriaRadar::bitsDireccion) /
                                        MemoriaRadar::tamanoPagina;

const std::vector<int> trazaInicial = {0, 1, 2, 3, 0, 1, 4, 2, 3, 5};
const std::vector<int> trazaLocalidad = {0, 1, 2, 3, 4, 0, 1, 2, 3, 5};
const std::vector<int> trazaCambio = {6, 7, 8, 9, 6, 7, 10, 8, 9, 11};

void imprimirDireccion(std::ostream& salida, std::uint16_t direccion) {
    salida << "0x" << std::hex << std::setw(4) << std::setfill('0') << direccion << std::dec;
}

}

std::vector<std::uint16_t> generarTraza(EscenarioCarga escenario,
                                        std::size_t cantidadPersonalizada) {
    std::vector<int> paginas;
    switch (escenario) {
        case EscenarioCarga::Vuelos10:
            paginas = trazaInicial;
            break;
        case EscenarioCarga::Vuelos50:
            for (std::size_t i = 0; i < 50; ++i) {
                if (i < 10) {
                    paginas.push_back(trazaInicial[i]);
                } else if (i < 30) {
                    paginas.push_back(trazaLocalidad[(i - 10) % trazaLocalidad.size()]);
                } else if (i < 40) {
                    paginas.push_back(trazaCambio[(i - 30) % trazaCambio.size()]);
                } else {
                    paginas.push_back(trazaInicial[(i - 40) % trazaInicial.size()]);
                }
            }
            break;
        case EscenarioCarga::Personalizado: {
            const std::size_t total = cantidadPersonalizada == 0 ? 10 : cantidadPersonalizada;
            for (std::size_t i = 0; i < total; ++i) {
                const std::vector<int>& origen = i < total / 2 ? trazaInicial : trazaLocalidad;
                paginas.push_back(origen[i % origen.size()]);
            }
            break;
        }
    }

    std::vector<std::uint16_t> traza;
    traza.reserve(paginas.size());
    for (std::size_t i = 0; i < paginas.size(); ++i) {
        const std::size_t desplazamiento = (i * 137) % MemoriaRadar::tamanoPagina;
        const std::size_t direccion = static_cast<std::size_t>(paginas[i]) *
                                      MemoriaRadar::tamanoPagina + desplazamiento;
        traza.push_back(static_cast<std::uint16_t>(direccion));
    }
    return traza;
}

std::string_view nombreTraza(EscenarioCarga escenario) {
    switch (escenario) {
        case EscenarioCarga::Vuelos10:
            return "ESTANDAR";
        case EscenarioCarga::Vuelos50:
            return "ESTRES";
        case EscenarioCarga::Personalizado:
            return "PERSONALIZADA";
    }
    return "DESCONOCIDA";
}

MemoriaRadar::MemoriaRadar(AlgoritmoReemplazo algoritmo, EscenarioCarga escenario,
                           std::size_t cantidadPersonalizada)
    : algoritmo_(algoritmo), escenario_(escenario),
      cantidadPersonalizada_(cantidadPersonalizada), tabla_(cantidadPaginas),
      marcos_(cantidadMarcos) {
}

void MemoriaRadar::reiniciar() {
    for (PTE& entrada : tabla_) {
        entrada = PTE{};
    }
    for (MarcoFisico& marco : marcos_) {
        marco = MarcoFisico{};
    }
    colaFifo_.clear();
    swap_.clear();
    manecilla_ = 0;
    reloj_ = 0;
    metricas_ = MetricasPaginacion{};
}

void MemoriaRadar::ejecutar(std::ostream& salida) {
    std::lock_guard<std::mutex> bloqueo(mutex_);
    reiniciar();

    const std::vector<std::uint16_t> traza = generarTraza(escenario_, cantidadPersonalizada_);
    salida << "[PAGINACION - " << nombreAlgoritmo(algoritmo_) << "] Algoritmo: "
           << nombreAlgoritmo(algoritmo_) << " | Escenario: " << nombreEscenario(escenario_)
           << " | Marcos: " << cantidadMarcos << " | Pagina: "
           << tamanoPagina << " B\n";
    imprimirTraza(traza, salida);

    for (std::size_t i = 0; i < traza.size(); ++i) {
        procesarReferencia(traza[i], i + 1, traza.size(), salida);
    }
    imprimirResumen(salida);
}

Traduccion MemoriaRadar::traducir(std::uint16_t direccionVirtual) const {
    std::lock_guard<std::mutex> bloqueo(mutex_);
    return traducirBloqueado(direccionVirtual);
}

Traduccion MemoriaRadar::traducirBloqueado(std::uint16_t direccionVirtual) const {
    const std::size_t numeroPagina = direccionVirtual / tamanoPagina;
    const std::size_t desplazamiento = direccionVirtual % tamanoPagina;
    Traduccion resultado;
    resultado.pagina = static_cast<int>(numeroPagina);
    resultado.desplazamiento = static_cast<int>(desplazamiento);

    if (numeroPagina < tabla_.size() && tabla_[numeroPagina].presente) {
        const PTE& entrada = tabla_[numeroPagina];
        resultado.direccionFisica = static_cast<std::uint16_t>(
            static_cast<std::size_t>(entrada.marcoFisico) * tamanoPagina + desplazamiento);
        resultado.acierto = true;
    }
    return resultado;
}

void MemoriaRadar::procesarReferencia(std::uint16_t direccionVirtual, std::size_t paso,
                                      std::size_t total, std::ostream& salida) {
    const Traduccion traduccion = traducirBloqueado(direccionVirtual);
    salida << "[PASO " << paso << '/' << total << "] Virtual=";
    imprimirDireccion(salida, direccionVirtual);
    salida << " | p=" << traduccion.pagina << " d=" << traduccion.desplazamiento << '\n';

    if (traduccion.acierto) {
        ++metricas_.pageHits;
        marcarAcceso(traduccion.pagina);
        const Traduccion final = traducirBloqueado(direccionVirtual);
        salida << "[MEMORIA - HIT] p=" << traduccion.pagina << " -> M"
               << tabla_[traduccion.pagina].marcoFisico << " | Fisica=";
        imprimirDireccion(salida, final.direccionFisica);
        salida << " | Ref=1 Mod=1\n";
    } else {
        ++metricas_.pageFaults;
        salida << "[MEMORIA - MISS] p=" << traduccion.pagina
               << " no residente, iniciando page fault\n";
        const ResultadoFallo fallo = atenderFallo(traduccion.pagina, salida);
        const Traduccion final = traducirBloqueado(direccionVirtual);
        salida << "[PAGE FAULT - SWAP] p=" << traduccion.pagina
               << " -> M" << fallo.indiceMarco;
        if (fallo.desdeSwap) {
            salida << " (IN desde swap)";
        } else {
            salida << " (IN nueva)";
        }
        if (fallo.paginaVictima >= 0) {
            salida << " | OUT p" << fallo.paginaVictima;
            if (fallo.escrituraSwap) {
                salida << " a swap (Mod=1)";
            } else {
                salida << " (Mod=0, swap omitido)";
            }
        }
        salida << " | Fisica=";
        imprimirDireccion(salida, final.direccionFisica);
        salida << '\n';
    }

    imprimirMarcos(salida);
    salida << '\n';
}

MemoriaRadar::ResultadoFallo MemoriaRadar::atenderFallo(int pagina, std::ostream& salida) {
    ResultadoFallo resultado;
    const std::size_t libre = seleccionarMarcoLibre();
    const bool marcoLibre = libre < cantidadMarcos;

    if (marcoLibre) {
        resultado.indiceMarco = libre;
    } else {
        resultado.indiceMarco = seleccionarVictima(salida);
        const MarcoFisico victima = marcos_[resultado.indiceMarco];
        resultado.paginaVictima = victima.numeroPagina;
        resultado.escrituraSwap = victima.modificado;

        if (victima.presente) {
            PTE& entradaVictima = tabla_[victima.numeroPagina];
            entradaVictima.presente = false;
            entradaVictima.marcoFisico = -1;
            entradaVictima.referenciado = false;
            entradaVictima.modificado = false;

            if (victima.modificado &&
                std::find(swap_.begin(), swap_.end(), victima.numeroPagina) == swap_.end()) {
                swap_.push_back(victima.numeroPagina);
            }
            const auto it = std::find(colaFifo_.begin(), colaFifo_.end(), victima.numeroPagina);
            if (it != colaFifo_.end()) {
                colaFifo_.erase(it);
            }
        }
    }

    const auto itSwap = std::find(swap_.begin(), swap_.end(), pagina);
    resultado.desdeSwap = itSwap != swap_.end();
    if (resultado.desdeSwap) {
        swap_.erase(itSwap);
    }

    PTE& entrada = tabla_[pagina];
    entrada.numeroPagina = pagina;
    entrada.presente = true;
    entrada.marcoFisico = static_cast<int>(resultado.indiceMarco);
    entrada.referenciado = true;
    entrada.modificado = true;
    entrada.orden = ++reloj_;

    MarcoFisico& marco = marcos_[resultado.indiceMarco];
    marco.numeroPagina = pagina;
    marco.presente = true;
    marco.referenciado = true;
    marco.modificado = true;
    marco.orden = entrada.orden;

    colaFifo_.push_back(pagina);
    if (marcoLibre && algoritmo_ == AlgoritmoReemplazo::Reloj) {
        manecilla_ = (resultado.indiceMarco + 1) % cantidadMarcos;
    }
    return resultado;
}

std::size_t MemoriaRadar::seleccionarMarcoLibre() const {
    for (std::size_t i = 0; i < marcos_.size(); ++i) {
        if (!marcos_[i].presente) {
            return i;
        }
    }
    return cantidadMarcos;
}

std::size_t MemoriaRadar::seleccionarVictima(std::ostream& salida) {
    switch (algoritmo_) {
        case AlgoritmoReemplazo::FIFO: {
            if (!colaFifo_.empty()) {
                const int pagina = colaFifo_.front();
                for (std::size_t i = 0; i < tabla_.size(); ++i) {
                    if (tabla_[i].presente && tabla_[i].numeroPagina == pagina) {
                        return static_cast<std::size_t>(tabla_[i].marcoFisico);
                    }
                }
            }
            break;
        }
        case AlgoritmoReemplazo::LRU: {
            bool encontrado = false;
            std::size_t mejor = 0;
            std::size_t menorOrden = 0;
            for (std::size_t i = 0; i < marcos_.size(); ++i) {
                if (marcos_[i].presente && (!encontrado || marcos_[i].orden < menorOrden)) {
                    mejor = i;
                    menorOrden = marcos_[i].orden;
                    encontrado = true;
                }
            }
            if (encontrado) {
                return mejor;
            }
            break;
        }
        case AlgoritmoReemplazo::Reloj: {
            for (std::size_t intentos = 0; intentos < cantidadMarcos; ++intentos) {
                MarcoFisico& marco = marcos_[manecilla_];
                if (marco.referenciado) {
                    marco.referenciado = false;
                    tabla_[marco.numeroPagina].referenciado = false;
                    salida << "[RELOJ - SEGUNDA OPORTUNIDAD] M" << manecilla_
                           << " Ref=1 -> 0\n";
                    manecilla_ = (manecilla_ + 1) % cantidadMarcos;
                } else {
                    const std::size_t victima = manecilla_;
                    manecilla_ = (manecilla_ + 1) % cantidadMarcos;
                    return victima;
                }
            }
            break;
        }
    }
    return manecilla_;
}

void MemoriaRadar::marcarAcceso(int pagina) {
    PTE& entrada = tabla_[pagina];
    entrada.referenciado = true;
    entrada.modificado = true;

    MarcoFisico& marco = marcos_[static_cast<std::size_t>(entrada.marcoFisico)];
    marco.referenciado = true;
    marco.modificado = true;

    if (algoritmo_ == AlgoritmoReemplazo::LRU) {
        entrada.orden = ++reloj_;
        marco.orden = entrada.orden;
    }
}

void MemoriaRadar::imprimirTraza(const std::vector<std::uint16_t>& traza,
                                 std::ostream& salida) const {
    salida << "[TRAZA - " << nombreTraza(escenario_) << "] Secuencia de paginas ("
           << traza.size() << "):";
    for (std::uint16_t direccion : traza) {
        salida << ' ' << direccion / tamanoPagina;
    }
    salida << '\n';

    salida << "[TRAZA - " << nombreTraza(escenario_) << "] Direcciones virtuales:";
    for (std::uint16_t direccion : traza) {
        salida << ' ';
        imprimirDireccion(salida, direccion);
    }
    salida << "\n\n";
}

void MemoriaRadar::imprimirMarcos(std::ostream& salida) const {
    salida << "[RAM]";
    for (std::size_t i = 0; i < marcos_.size(); ++i) {
        const MarcoFisico& marco = marcos_[i];
        salida << " M" << i << '[';
        if (algoritmo_ == AlgoritmoReemplazo::Reloj && i == manecilla_) {
            salida << "->";
        } else {
            salida << "  ";
        }
        salida << " P=" << (marco.presente ? '1' : '0')
               << " Ref=" << (marco.referenciado ? '1' : '0')
               << " Mod=" << (marco.modificado ? '1' : '0')
               << " Pag=";
        if (marco.presente) {
            salida << marco.numeroPagina;
        } else {
            salida << '-';
        }
        salida << " Ord=";
        if (marco.presente) {
            salida << marco.orden;
        } else {
            salida << '-';
        }
        salida << ']';
    }
    salida << '\n';
}

void MemoriaRadar::imprimirResumen(std::ostream& salida) const {
    salida << "[RESUMEN - " << nombreAlgoritmo(algoritmo_) << "] Page Hits: "
           << metricas_.pageHits << " | Page Faults: " << metricas_.pageFaults
           << " | Total Referencias: " << metricas_.pageHits + metricas_.pageFaults << '\n';
}

MetricasPaginacion MemoriaRadar::metricas() const {
    std::lock_guard<std::mutex> bloqueo(mutex_);
    return metricas_;
}

std::vector<PTE> MemoriaRadar::tablaDePaginas() const {
    std::lock_guard<std::mutex> bloqueo(mutex_);
    return tabla_;
}

std::vector<MarcoFisico> MemoriaRadar::marcos() const {
    std::lock_guard<std::mutex> bloqueo(mutex_);
    return marcos_;
}
