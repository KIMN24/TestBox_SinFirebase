#pragma once
#include "ProcessInterface.hpp"
#include "utils.hpp"

class Eventos : public ProcessInterface {
private:
    int etapaInterna;      // Etapa de ejecución interna
    int eventoSeleccionado; // 0 = L0, 1 = L1, 2 = L2
    int repeticiones;       // Para contar ON/OFF de R2 en L1

public:
    Eventos() : etapaInterna(0), eventoSeleccionado(0), repeticiones(0) {}

    void setEvento(int ev) {
        eventoSeleccionado = ev;
        etapaInterna = 0; // Reinicia ejecución
        repeticiones = 0;
    }

    int getEvento() { return eventoSeleccionado; }

    void execute() override {
        if (pausaActiva) return;

        switch(eventoSeleccionado) {
            case 0: ejecutarL0(); break;
            case 1: ejecutarL1(); break;
            case 2: ejecutarL2(); break;
        }
    }

    // ==================== EVENTOS =====================
    void ejecutarL0() {
        switch (etapaInterna) {
            case 0:
                digitalWrite(Relay1, LOW);
                //digitalWrite(Relay2, LOW);
                Serial.println("[L0] R1 ON, R2 OFF");
                delay(4000); // Aquí x segundos (ej: 2s)
                etapaInterna = 1;
                break;
            case 1:
                digitalWrite(Relay1, HIGH);
                delay(4000); 
                Serial.println("[L0] Reinicio");
                //subiryVerificarInfo();
                etapaInterna = 0;
                break;
        }
    }

    void ejecutarL1() {
        switch (etapaInterna) {
            case 0:
                digitalWrite(Relay1, LOW);
                Serial.println("[L1] R1 ON fijo");
                etapaInterna = 1;
                break;
            case 1:
                if (repeticiones < 4) {
                    digitalWrite(Relay1, LOW);
                    digitalWrite(Relay2, LOW);
                    Serial.println("[L1] R2 ON");
                    delay(1000);
                    digitalWrite(Relay2, HIGH);
                    Serial.println("[L1] R2 OFF");
                    delay(1000);
                    repeticiones++;
                } else {
                    etapaInterna = 2;
                }
                break;
            case 2:
                digitalWrite(Relay1, HIGH);
                Serial.println("[L1] Reinicio");
                delay(1000);
                repeticiones = 0;
                guardarConteoLocal();
                etapaInterna = 0;
                break;
        }
    }

    void ejecutarL2() {
        switch (etapaInterna) {
            case 0:
                digitalWrite(Relay1, HIGH);
                digitalWrite(Relay2, LOW);
                Serial.println("[L2] R2 ON 1s, R1 OFF");
                delay(2000);
                etapaInterna = 1;
                break;
            case 1:
                digitalWrite(Relay2, HIGH);
                delay(1000); 
                Serial.println("[L2] Reinicio");
                //subiryVerificarInfo();
                etapaInterna = 0;
                break;
        }
    }

    // ==================== INTERFAZ =====================
    void setTiempoAbrir(int) override {}
    void setTiempoCerrar(int) override {}
    void setTiempoLazo(int) override {}

    int getTiempoAbrir() override { return 0; }
    int getTiempoCerrar() override { return 0; }
    int getTiempoLazo() override { return 0; }

    String getModeName() override { return "Eventos L0-L2"; }
};
