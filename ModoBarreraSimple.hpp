#pragma once
#include "ProcessInterface.hpp"
#include "utils.hpp"

class ModoBarreraSimple : public ProcessInterface {
private:
  int tiempoAbrir;
  int tiempoCerrar;
  int tiempoLazo;
  int etapaInterna;         // Cambiado para evitar conflicto con variable global
  bool pausaActivaInterna;  // Cambiado para evitar conflicto
public:
  ModoBarreraSimple()
    : tiempoAbrir(), tiempoCerrar(), tiempoLazo(1), etapaInterna(0), pausaActivaInterna(false) {}

  void execute() override {
    digitalWrite(ledPin, HIGH);
    actualizarEstadoPausa();

    if (pausaActiva) return;

    switch (etapaInterna) {
      case 0:
        Serial.println("MODO SIMPLE - ETAPA 1: Cerrando...");
        RelaysOp();
        esperar(tiempoAbrir, 1);
        break;

      case 1:
        Serial.println("MODO SIMPLE - ETAPA 0: Abriendo...");
        RelaysCl();
        esperar(tiempoCerrar, 2);
        break;

      case 2:
        Serial.println("MODO SIMPLE - ETAPA 2: Subiendo datos...");
        //subiryVerificarInfo();
        guardarConteoLocal();
        Serial.print("Cuentas actuales: ");
        Serial.println(counter);
        etapaInterna = 0;
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
    return "Simple Barrier";
  }

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
};
