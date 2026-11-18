#ifndef SINCRONIZACION_HPP
#define SINCRONIZACION_HPP

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <vector>

struct Vuelo {
    int id = 0;
    int prioridad = 1;
};

class TorreControl {
public:
    void solicitarPista(int idVuelo, int prioridad);
    void notificarPistaDespejada(int idVuelo);

private:
    struct Solicitante {
        int idVuelo = 0;
        int prioridadBase = 0;
        int prioridadEfectiva = 0;
        std::size_t ordenLlegada = 0;
    };

    bool esTurno(int idVuelo) const;
    int aplicarAging(int idVuelo);
    void retirarDeEspera(int idVuelo);
    void ocuparPista(int idVuelo);
    void liberarPista(int idVuelo);
    static void autorizacionConcedida(int idVuelo, int prioridad);
    static void confirmacionFin(int idVuelo);

    std::mutex mutex_;
    std::condition_variable condicion_;
    std::vector<Solicitante> esperando_;
    std::size_t ordenLlegada_ = 0;
    bool pistaOcupada_ = false;
    int idEnPista_ = -1;
};

void hiloAvion(TorreControl& torre, const Vuelo& vuelo);
void ejecutarManiobra(const Vuelo& vuelo);
void ejecutarSimulacionPistas(std::size_t cantidadVuelos);

#endif
