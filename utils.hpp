// ========================== FUNCIONES PARA LOS PROCESOS ============================
#pragma once

#include "log.hpp"
#include <Preferences.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

extern Preferences prefs;
bool wifiConnecting = false;

// ========================== ALMACENAMIENTO LOCAL ============================

// Inicializa almacenamiento local y recupera conteo previo
void storageInit() {
  prefs.begin("conteos", false);  // false = modo lectura/escritura
  counter = prefs.getInt("contador", 0);
  Serial.print("Conteo recuperado: ");
  Serial.println(counter);
}

// Incrementa contador y guarda en memoria no volátil
void guardarConteoLocal() {
  counter++;
  prefs.putInt("contador", counter);
  Serial.print("Nuevo conteo guardado: ");
  Serial.println(counter);
  etapa = 0;
}

void cargarDatos() {
  counter = prefs.getInt("contador", 0);
  Serial.print("Contador cargado desde memoria: ");
  Serial.println(counter);
}


String getDateTimeString() {
  WiFiClientSecure client;
  client.setInsecure();  // Evita validar certificados (solo para testing)

  HTTPClient https;
  const char* url = "https://api.timezonedb.com/v2.1/get-time-zone?key=9W5GP4UHSQYY&format=json&by=zone&zone=America/Mexico_City";
  if (!https.begin(client, url)) {
    Serial.println("Error al iniciar conexión HTTPS");
    return "Fecha no disponible";
  }

  int httpCode = https.GET();
  if (httpCode != 200) {
    Serial.printf("Error HTTP: %d\n", httpCode);
    https.end();
    return "Fecha no disponible";
  }

  String payload = https.getString();
  https.end();

  // Parse JSON
  DynamicJsonDocument doc(1024);
  if (deserializeJson(doc, payload) != DeserializationError::Ok) {
    Serial.println("Error al parsear JSON");
    return "Fecha no disponible";
  }

  String dateTime = doc["formatted"];  // Ej: "2025-10-07 14:05:12"

  // Extraer partes
  int year = dateTime.substring(0, 4).toInt();
  int month = dateTime.substring(5, 7).toInt();
  int day = dateTime.substring(8, 10).toInt();
  int hour = dateTime.substring(11, 13).toInt();
  int minute = dateTime.substring(14, 16).toInt();
  int second = dateTime.substring(17, 19).toInt();

  // Llenar arreglos globales (si los usas)
  fechaArr[0] = day;
  fechaArr[1] = month;
  fechaArr[2] = year;
  horaArr[0] = hour;
  horaArr[1] = minute;
  horaArr[2] = second;

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d %02d:%02d:%02d",
           day, month, year, hour, minute, second);

  return String(buffer);
}

void RelaysOp() {
  digitalWrite(Relay1, true);
  digitalWrite(Relay3, true);
}

void RelaysCl() {
  digitalWrite(Relay1, false);
  digitalWrite(Relay3, false);
}

void RelaysAct() {
  digitalWrite(Relay2, true); //HIGH
  digitalWrite(Relay4, true); //HIGH
}

// Desactiva los cuatro relés.
void RelaysDes() {
  digitalWrite(Relay2, false); //LOW
  digitalWrite(Relay4, false); //LOW
}


// Verifica si el botón de pausa fue presionado y actualiza el estado del sistema
void actualizarEstadoPausa() {
  bool estadoActual = digitalRead(Pause);
  if (estadoActual == LOW && ultimoEstadoPausa == true) {
    pausaActiva = !pausaActiva;  // Cambia de estado al presionar el botón
    String timestamp = getDateTimeString();
    if (pausaActiva) {
      digitalWrite(ledV, HIGH);
      digitalWrite(ledPin, LOW);
      addLogEvent("[Pausado] " + timestamp);
      Serial.println("Proceso pausado.");
    } else {
      digitalWrite(ledV, LOW);
      digitalWrite(ledPin, HIGH);
      addLogEvent("[Reanudado] " + timestamp);
      Serial.println("Proceso reanudado.");
    }
    delay(200);  // Antirebote
  }
  ultimoEstadoPausa = estadoActual;
}

// Espera `limite` segundos con control de pausa y cambia a `sigEtapa` al finalizar
void esperar(int limite, int sigEtapa) {
  int segundosTranscurridos = 0;

  while (segundosTranscurridos <= limite) {
    actualizarEstadoPausa();  // Verifica si se presionó el botón de pausa

    if (pausaActiva) {
      Serial.println("Pausa detectada durante espera.");
      // Espera aquí hasta que se quite la pausa
      while (pausaActiva) {
        //webServer.handleClient();
        actualizarEstadoPausa();  // Sigue escuchando el botón para reanudar
        delay(100);
      }
      Serial.println("Reanudando espera...");
    }

    if (interruptCounter > 0) {
      Serial.print("SEGUNDO: ");
      Serial.println(segundosTranscurridos);
      portENTER_CRITICAL(&timerMux);
      interruptCounter = 0;
      portEXIT_CRITICAL(&timerMux);
      segundosTranscurridos++;
    }
    //webServer.handleClient();
    delay(5);  // Evita saturar la CPU
  }

  etapa = sigEtapa;
}

void wifiBegin() {
  if (wifiConnecting) return;  // Ya hay un intento en curso
  wifiConnecting = true;

  WiFi.mode(WIFI_STA);

  // IP fija
  IPAddress local_ip(192, 168, 200, 250);
  IPAddress gateway(192, 168, 200, 254);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);

  if (!WiFi.config(local_ip, gateway, subnet, dns)) {
    Serial.println("Error al configurar IP estática");
  }

  Serial.print("Conectando al Wi-Fi ");
  Serial.print(ssid);
  Serial.println("...");

  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long startAttemptTime = millis();

  // Esperar máximo 10 segundos para conectar
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(WifiConn, HIGH);
    Serial.println("\nConectado al Wi-Fi.");
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());
    String timestamp = getDateTimeString();
    addLogEvent("[Conectado] " + timestamp);
    wifiConnectedPreviously = true;
  } else {
    Serial.println("\nNo se pudo conectar al Wi-Fi.");
    digitalWrite(WifiConn, LOW);
    wifiConnectedPreviously = false;
  }

  wifiConnecting = false; // Marca fin del intento
}
