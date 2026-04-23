//VERSIÓN FINAL CÓDIGO TEST BOX CON FREERTOS

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

// ========================== VARIABLES FREERTOS =============================
TaskHandle_t webServerTaskHandle = NULL;
TaskHandle_t processTaskHandle = NULL;
SemaphoreHandle_t wifiMutex = NULL;


// ========================== TAREA WEB SERVER ==============================
void webServerTask(void *parameter) {
  Serial.println("[FREERTOS] Tarea Web Server iniciada");
  
  while (1) {
    webServer.handleClient();
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Permite que otras tareas se ejecuten
  }
  vTaskDelete(NULL);
}

// ========================== TAREA PROCESO ===============================
void processTask(void *parameter) {
  Serial.println("[FREERTOS] Tarea Proceso iniciada");
  
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      if (flagtostart) {
        if (modoActual != nullptr) {
          modoActual->execute();
        }
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);  // Permite que otras tareas se ejecuten
  }
  vTaskDelete(NULL);
}

// ========================== CONFIGURACIÓN ===============================
void setup() {
  Serial.begin(115200);
  prefs.begin("conteos", false);
  pinMode(ledPin, OUTPUT);
  pinMode(ledV, OUTPUT);
  pinMode(ledOn, OUTPUT);
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
  digitalWrite(ledOn, LOW);
  digitalWrite(WifiConn, LOW);
  
  modo = 0; 
  modoActual = &modoSimple;
  
  // Configurar tiempos iniciales
  modoSimple.setTiempoAbrir(tiempoAbrir);
  modoSimple.setTiempoCerrar(tiempoCerrar);
  
  modoLazo.setTiempoAbrir(tiempoAbrir);
  modoLazo.setTiempoCerrar(tiempoCerrar);
  
  //Configurar el modo actual
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
  
  // ========================== CREAR TAREAS FREERTOS ========================
  // Crear mutex para proteger acceso a recursos compartidos
  wifiMutex = xSemaphoreCreateMutex();
  
  // Crear tarea para Web Server (Core 1, Prioridad 1)
  xTaskCreatePinnedToCore(
    webServerTask,           // Función de la tarea
    "WebServerTask",         // Nombre
    4096,                    // Stack size
    NULL,                    // Parámetro
    1,                       // Prioridad
    &webServerTaskHandle,    // Task handle
    1                        // Core (1 = segundo núcleo)
  );
  
  // Crear tarea para Proceso (Core 0, Prioridad 2)
  xTaskCreatePinnedToCore(
    processTask,             // Función de la tarea
    "ProcessTask",           // Nombre
    4096,                    // Stack size
    NULL,                    // Parámetro
    2,                       // Prioridad (más alta que web server)
    &processTaskHandle,      // Task handle
    0                        // Core (0 = primer núcleo)
  );
  
  Serial.println("[FREERTOS] Tareas creadas y ejecutándose");
}

// ========================== LOOP ===============================
void loop() {
  digitalWrite(ledOn, HIGH);
  
  // Reconectar Wi-Fi si es necesario
  if (WiFi.status() != WL_CONNECTED) {
    wifiBegin();
  }
  // Pequeño delay para no saturar el loop
  delay(1000);
  
  // Imprimir información de depuración
  // Serial.print("[DEBUG] WiFi: ");
  // Serial.print(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado");
  // Serial.print(" | flagtostart: ");
  // Serial.print(flagtostart);
  // Serial.print(" | pausaActiva: ");
  // Serial.println(pausaActiva);
}
