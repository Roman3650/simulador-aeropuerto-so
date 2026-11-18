#include "Sincronizacion.hpp"

#include "Logger.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

namespace {

constexpr auto intervaloAging = std::chrono::milliseconds(100);
constexpr auto duracionManiobra = std::chrono::milliseconds(60);
constexpr int incrementoAging = 1;

}

void TorreControl::solicitarPista(int idVuelo, int prioridad) {
    std::unique_lock<std::mutex> bloqueo(mutex_);

    esperando_.push_back(Solicitante{idVuelo, prioridad, prioridad, ordenLlegada_++});
    Logger::traza("[PISTA - MUTEX] Vuelo ", idVuelo, " solicita pista | prioridad base=",
                  prioridad, " | en espera=", esperando_.size());

    int prioridadActual = prioridad;
    bool bloqueado = false;
    while (pistaOcupada_ || !esTurno(idVuelo)) {
        if (!bloqueado) {
            bloqueado = true;
            Logger::traza("[PISTA - BLOQUEADO] Vuelo ", idVuelo,
                          " pasa a BLOQUEADO (mutex liberado mientras espera)");
        }
        prioridadActual = aplicarAging(idVuelo);
        condicion_.wait_for(bloqueo, intervaloAging, [this, idVuelo] {
            return !pistaOcupada_ && esTurno(idVuelo);
        });
    }

    retirarDeEspera(idVuelo);
    ocuparPista(idVuelo);
    autorizacionConcedida(idVuelo, prioridadActual);
}

void TorreControl::notificarPistaDespejada(int idVuelo) {
    std::unique_lock<std::mutex> bloqueo(mutex_);
    liberarPista(idVuelo);
    bloqueo.unlock();
    condicion_.notify_all();
    confirmacionFin(idVuelo);
}

bool TorreControl::esTurno(int idVuelo) const {
    const Solicitante* propio = nullptr;
    for (const Solicitante& candidato : esperando_) {
        if (candidato.idVuelo == idVuelo) {
            propio = &candidato;
        }
    }
    if (propio == nullptr) {
        return false;
    }

    for (const Solicitante& candidato : esperando_) {
        if (candidato.prioridadEfectiva > propio->prioridadEfectiva ||
            (candidato.prioridadEfectiva == propio->prioridadEfectiva &&
             candidato.ordenLlegada < propio->ordenLlegada)) {
            return false;
        }
    }
    return true;
}

int TorreControl::aplicarAging(int idVuelo) {
    const auto it = std::find_if(esperando_.begin(), esperando_.end(),
                                 [idVuelo](const Solicitante& candidato) {
                                     return candidato.idVuelo == idVuelo;
                                 });
    if (it == esperando_.end()) {
        return 0;
    }

    const int anterior = it->prioridadEfectiva;
    it->prioridadEfectiva += incrementoAging;
    Logger::traza("[TORRE - AGING] Vuelo ", idVuelo, " prioridad ", anterior, " -> ",
                  it->prioridadEfectiva);
    return it->prioridadEfectiva;
}

void TorreControl::retirarDeEspera(int idVuelo) {
    const auto it = std::find_if(esperando_.begin(), esperando_.end(),
                                 [idVuelo](const Solicitante& candidato) {
                                     return candidato.idVuelo == idVuelo;
                                 });
    if (it != esperando_.end()) {
        esperando_.erase(it);
    }
}

void TorreControl::ocuparPista(int idVuelo) {
    pistaOcupada_ = true;
    idEnPista_ = idVuelo;
    Logger::traza("[PISTA - MUTEX] Pista adquirida por Vuelo ", idVuelo);
}

void TorreControl::liberarPista(int idVuelo) {
    if (idEnPista_ != idVuelo) {
        Logger::traza("[PISTA - ERROR] Vuelo ", idVuelo, " no es el dueño de la pista");
        return;
    }

    pistaOcupada_ = false;
    idEnPista_ = -1;
    Logger::traza("[PISTA - LIBERADA] Vuelo ", idVuelo, " despejó la pista");
}

void TorreControl::autorizacionConcedida(int idVuelo, int prioridad) {
    Logger::traza("[TORRE - AUTORIZACION] Vuelo ", idVuelo,
                  " autorizado a entrar | prioridad final=", prioridad);
}

void TorreControl::confirmacionFin(int idVuelo) {
    Logger::traza("[TORRE - FIN] Confirmación de fin recibida de Vuelo ", idVuelo);
}

void hiloAvion(TorreControl& torre, const Vuelo& vuelo) {
    torre.solicitarPista(vuelo.id, vuelo.prioridad);
    ejecutarManiobra(vuelo);
    torre.notificarPistaDespejada(vuelo.id);
}

void ejecutarManiobra(const Vuelo& vuelo) {
    Logger::traza("[PISTA - EN PISTA] Vuelo ", vuelo.id,
                  " ejecutando maniobra (zona crítica)");
    std::this_thread::sleep_for(duracionManiobra);
    Logger::traza("[PISTA - EN PISTA] Vuelo ", vuelo.id, " maniobra finalizada");
}

void ejecutarSimulacionPistas(std::size_t cantidadVuelos) {
    if (cantidadVuelos == 0) {
        return;
    }

    TorreControl torre;
    std::vector<Vuelo> vuelos;
    vuelos.reserve(cantidadVuelos);
    for (std::size_t i = 0; i < cantidadVuelos; ++i) {
        vuelos.push_back(Vuelo{static_cast<int>(i + 1),
                               1 + static_cast<int>((i * 7) % 5)});
    }

    std::vector<std::thread> hilos;
    hilos.reserve(cantidadVuelos);
    Logger::traza("[TORRE - INICIO] Simulación de ", cantidadVuelos,
                  " vuelos con una pista");
    for (const Vuelo& vuelo : vuelos) {
        hilos.emplace_back([&torre, vuelo] { hiloAvion(torre, vuelo); });
    }
    for (std::thread& hilo : hilos) {
        hilo.join();
    }
    Logger::traza("[TORRE - RESUMEN] Vuelos atendidos: ", cantidadVuelos);
}
