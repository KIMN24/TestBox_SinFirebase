#pragma once

#include "ProcessInterface.hpp"
#include "utils.hpp"

class ModoBarreraLazo : public ProcessInterface {
private:
  int tiempoAbrir;
  int tiempoCerrar;
  int tiempoLazo;
  int etapaInterna;         // Cambiado para evitar conflicto con variable global
  bool pausaActivaInterna;  // Cambiado para evitar conflicto

public:
  ModoBarreraLazo()
    : tiempoAbrir(), tiempoCerrar(), tiempoLazo(95), etapaInterna(-1), pausaActivaInterna(false) {}

void execute() override {
    digitalWrite(ledPin, HIGH);
    actualizarEstadoPausa();
    if (pausaActiva) return; // Usar la variable global de pausa

    switch (etapaInterna) {
    case -1:
        etapaInterna = 0;
        return;

    case 0: // R1 ON (LOW), R2 OFF (HIGH)
        RelaysCl();
        RelaysAct();
        Serial.println("ETAPA 0: R1 ON (activado), R2 OFF (desactivado)");
        esperar(tiempoAbrir, 1);
        break;

    case 1: // R1 ON, R2 ON (tiempoLazo)
        RelaysCl();
        RelaysDes();
        Serial.println("ETAPA 1: R2 activado por tiempoLazo");
        esperarMs(tiempoLazo, 2);
        break;

    case 2: // R1 ON, R2 OFF
        RelaysCl();
        RelaysAct();
        guardarConteoLocal();
        Serial.println("ETAPA 2: R2 apagado, R1 sigue activado");
        Serial.print("Cuentas actuales: ");
        Serial.println(counter);
        esperar(1, 3);
        break;

    case 3: // R1 OFF, R2 OFF - FIN DEL CICLO
        RelaysOp();
        RelaysAct();
        Serial.println("ETAPA 3: R1 y R2 desactivados - Fin de ciclo");
        esperar(tiempoCerrar, 0);
        break;
    }
  }

  void setTiempoAbrir(int tiempo) override {
    tiempoAbrir = tiempo;
  }
  
  void setTiempoCerrar(int tiempo) override {
    tiempoCerrar = tiempo;
  }
  
  void setTiempoLazo(int tiempo) override {
    tiempoLazo = tiempo;
  }

  int getTiempoAbrir() override {
    return tiempoAbrir;
  }
  
  int getTiempoCerrar() override {
    return tiempoCerrar;
  }
  
  int getTiempoLazo() override {
    return tiempoLazo;
  }

  String getModeName() override {
    return "Barrier with Loop";
  }

private:
  void esperar(int limite, int sigEtapa) {
    unsigned long startTime = millis();
    int segundosTranscurridos = 0;

    while (segundosTranscurridos < limite) {
      actualizarEstadoPausa();

      if (pausaActiva) { // Usar variable global
        Serial.println("Pausa detectada durante espera.");
        while (pausaActiva) {
          actualizarEstadoPausa();
          delay(100);
        }
        Serial.println("Reanudando espera...");
        startTime = millis(); // Reiniciar tiempo después de pausa
      }

      if (millis() - startTime >= 1000) {
        startTime = millis();
        segundosTranscurridos++;
        Serial.print("SEGUNDO: ");
        Serial.println(segundosTranscurridos);
      }
      
      delay(5);
    }

    etapaInterna = sigEtapa;
  }
  // ==================== ESPERA EN MILISEGUNDOS ====================
  void esperarMs(int limiteMs, int sigEtapa) {
    unsigned long startTime = millis();

    while (millis() - startTime < (unsigned long)limiteMs) {
      actualizarEstadoPausa();

      if (pausaActiva) {
        Serial.println("Pausa detectada durante espera (ms).");
        while (pausaActiva) {
          actualizarEstadoPausa();
          delay(50);
        }
        Serial.println("Reanudando espera...");
        startTime = millis(); // reinicia conteo tras la pausa
      }

      delay(1);
    }

    etapaInterna = sigEtapa;
  }
};