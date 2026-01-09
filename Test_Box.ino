//VERSIÓN FINAL CÓDIGO TEST BOX

#include <WiFi.h>
#include <HTTPClient.h>
#include <TimeLib.h>
#include "config.h"
#include "log.hpp"
#include "Webserver.hpp"
#include "Eventos.hpp"
#include "utils.hpp"
#include "ProcessInterface.hpp"
#include "ModoBarreraSimple.hpp"
#include "ModoBarreraLazo.hpp"
#include <Preferences.h>

Preferences prefs;


WebServerHandler webServer;
ProcessInterface* modoActual = nullptr;
ModoBarreraSimple modoSimple;
ModoBarreraLazo modoLazo;
Eventos eventos;

extern int tiempoAbrir;
extern int tiempoCerrar;
extern int counter;
extern volatile bool flagtostart;
extern bool pausaActiva;
extern int etapa;

void setup() {
  Serial.begin(115200);
  prefs.begin("conteos", false);
  pinMode(ledPin, OUTPUT);
  pinMode(ledV, OUTPUT);
  pinMode(Pause, INPUT_PULLUP);
  pinMode(WifiConn, OUTPUT);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, HIGH);
  digitalWrite(Relay4, HIGH);
  digitalWrite(ledPin, LOW);
  digitalWrite(ledV, LOW);
  digitalWrite(WifiConn, LOW);
  
  modo = 0; 
  modoActual = &modoSimple;
  
  // Configurar tiempos iniciales
  modoSimple.setTiempoAbrir(tiempoAbrir);
  modoSimple.setTiempoCerrar(tiempoCerrar);
  
  modoLazo.setTiempoAbrir(tiempoAbrir);
  modoLazo.setTiempoCerrar(tiempoCerrar);
  
  // También configurar el modo actual (redundante pero seguro)
  modoActual->setTiempoAbrir(tiempoAbrir);
  modoActual->setTiempoCerrar(tiempoCerrar);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case WIFI_EVENT_STA_DISCONNECTED:
      Serial.println("Wi-Fi desconectado. Intentando reconectar...");
      digitalWrite(WifiConn, LOW);
      wifiConnecting = false;  // Permite reintento
      break;

    case IP_EVENT_STA_GOT_IP:
      Serial.print("Wi-Fi reconectado. IP: ");
      Serial.println(WiFi.localIP());
      digitalWrite(WifiConn, HIGH);
      break;
  }
  });
  
  wifiBegin();
  webServer.begin();
  initTimer();
  cargarDatos();
}

void loop() {
    webServer.handleClient();

  if (WiFi.status() == WL_CONNECTED) {
    if (flagtostart) {

      if (modoActual != nullptr) {
        modoActual->execute();
      }
    }
  } else {
    wifiBegin();
  }
}
