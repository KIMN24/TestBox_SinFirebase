#pragma once
#include <WebServer.h>
#include "ProcessInterface.hpp"
#include "ModoBarreraSimple.hpp"
#include "ModoBarreraLazo.hpp"
#include "Eventos.hpp"
#include <Preferences.h>
extern Preferences prefs;


// Variables globales
extern int tiempoAbrir;
extern int tiempoCerrar;
extern Eventos eventos;
extern int counter;
extern volatile bool flagtostart;
extern int modo;  // 0 = Simple, 1 = Lazo, 2 = Eventos
extern ProcessInterface* modoActual;
extern ModoBarreraSimple modoSimple;
extern ModoBarreraLazo modoLazo;

const char* www_username = "admin";
const char* www_password = "1234";

class WebServerHandler {
public:
  WebServerHandler() {}

  void begin() {
    server.on("/", HTTP_GET, std::bind(&WebServerHandler::handleRoot, this));

    server.on("/start", HTTP_GET, [this]() {
      flagtostart = true;
      server.send(200, "text/plain", "Sistema iniciado manualmente");
      addLogEvent("[START] ");
    });

    server.on("/setTimes", HTTP_POST, [this]() {
      handleSetTimes();
    });

    server.on("/get-counter", HTTP_GET, [this]() {
      server.send(200, "text/plain", String(counter));
    });

    server.on("/reset-counter", HTTP_POST, [this]() {
      if (!server.authenticate(www_username, www_password)) {
        return server.requestAuthentication();
      }

      counter = 0;

      // Guarda el valor reiniciado en memoria interna
      prefs.putInt("contador", counter);

      Serial.println("Contador reiniciado en memoria local");
      server.send(200, "text/plain", "Conteo reiniciado");
    });


    server.on("/event-log", HTTP_GET, [this]() {
      String response;
      for (const auto& entry : eventLog) {
        response += entry + "\n";
      }
      server.send(200, "text/plain", response);
    });

    server.on("/set-mode", HTTP_POST, [this]() {
      String modoStr;
      if (server.hasArg("modo")) {
        int nuevoModo = server.arg("modo").toInt();

        if (nuevoModo == 0) {
          modoActual = &modoSimple;
          modoStr = "Barrera Simple";
        } else if (nuevoModo == 1) {
          modoActual = &modoLazo;
          modoStr = "Barrera con Lazo";
        } else if (nuevoModo == 2) {
          modoActual = &eventos;
          modoStr = "Eventos (L0-L2)";
        }
        modo = nuevoModo;

        Serial.print("Modo cambiado a: ");
        Serial.println(nuevoModo);
        addLogEvent("[CAMBIO MODO] " + modoStr);
      }
      server.send(200, "text/plain", "OK");
    });

    // Endpoint para cambiar el evento dentro de "Eventos" (fetch)
    server.on("/set-evento", HTTP_POST, [this]() {
      if (server.hasArg("evento")) {
        int nuevoEvento = server.arg("evento").toInt();
        eventos.setEvento(nuevoEvento);
        Serial.print("Evento cambiado a: ");
        Serial.println(nuevoEvento);
      }
      server.send(200, "text/plain", "OK");
    });

    server.begin();
  }

  void handleClient() {
    server.handleClient();
  }

private:
  WebServer server;

  void handleRoot() {
    String html = "<!DOCTYPE html><html lang='es'><head>\
  <meta charset='UTF-8'>\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
  <title>Control de Barrera</title>\
  <style>\
    :root {\
      --primary-color: #141f42;\
      --secondary-color: #1e2e5c;\
      --accent-color: #66D43D;\
      --text-color: #ffffff;\
      --border-color: #34495e;\
      --button-color: #66D43D;\
      --button-hover: #66D43D;\
      --card-bg: rgba(255, 255, 255, 0.05);\
    }\
    * {\
      box-sizing: border-box;\
      margin: 0;\
      padding: 0;\
    }\
    body {\
      background: linear-gradient(135deg, var(--primary-color) 0%, var(--secondary-color) 100%);\
      color: var(--text-color);\
      font-family: 'Poppins', sans-serif;\
      line-height: 1.6;\
      padding: 20px;\
      min-height: 100vh;\
    }\
    .container {\
      max-width: 1200px;\
      margin: 0 auto;\
      background: rgba(0, 0, 0, 0.3);\
      border-radius: 15px;\
      padding: 25px;\
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);\
    }\
    header {\
      text-align: center;\
      margin-bottom: 30px;\
      padding-bottom: 15px;\
      border-bottom: 1px solid var(--border-color);\
      grid-column: 1 / -1;\
    }\
    h1 {\
      font-size: 2.2rem;\
      margin-bottom: 10px;\
      background: linear-gradient(90deg, #4facfe 0%, #00f2fe 100%);\
      -webkit-background-clip: text;\
      -webkit-text-fill-color: transparent;\
    }\
    h2 {\
      font-size: 1.3rem;\
      margin: 0 0 15px 0;\
      color: #c2d1e9;\
      border-left: 4px solid var(--accent-color);\
      padding-left: 10px;\
    }\
    .dashboard {\
      display: grid;\
      grid-template-columns: 1fr 1fr;\
      gap: 20px;\
    }\
    .card {\
      background: var(--card-bg);\
      border-radius: 10px;\
      padding: 20px;\
      margin-bottom: 0;\
      border: 1px solid var(--border-color);\
      height: fit-content;\
    }\
    .full-width {\
      grid-column: 1 / -1;\
    }\
    .form-group {\
      display: flex;\
      align-items: center;\
      margin-bottom: 15px;\
      flex-wrap: wrap;\
    }\
    label {\
      flex: 0 0 150px;\
      margin-right: 15px;\
    }\
    select, input[type='number'] {\
      background: #34495e;\
      border: 1px solid var(--border-color);\
      border-radius: 5px;\
      color: white;\
      padding: 8px 12px;\
      margin-right: 15px;\
      width: 200px;\
    }\
    .boton-update, .boton-start, input[type='submit'] {\
      background-color: var(--button-color);\
      color: white;\
      padding: 10px 15px;\
      border: none;\
      border-radius: 5px;\
      cursor: pointer;\
      font-size: 12px;\
      transition: all 0.3s ease;\
      margin-top: 5px;\
    }\
    .boton-update:hover, .boton-start:hover, input[type='submit']:hover {\
      background-color: var(--button-hover);\
      transform: translateY(-2px);\
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);\
    }\
    .boton-start {\
      background: linear-gradient(90deg, #4facfe 0%, #00f2fe 100%);\ 
      width: 100px;\
      font-weight: bold;\
      margin-top: 10px;\
    }\
    .counter-display {\
      font-size: 3rem;\
      text-align: center;\
      padding: 20px;\
      background: rgba(0, 0, 0, 0.2);\
      border-radius: 10px;\
      margin: 15px 0;\
      font-weight: bold;\
      border: 2px solid var(--accent-color);\
    }\
    .log-container {\
      background: rgba(0, 0, 0, 0.2);\
      border: 1px solid var(--border-color);\
      border-radius: 10px;\
      padding: 15px;\
      margin-bottom: 0;\
      height: 150px;\
      overflow-y: auto;\
      font-family: monospace;\
      font-size: 12px;\
    }\
    .status-indicator {\
      display: inline-block;\
      width: 12px;\
      height: 12px;\
      border-radius: 50%;\
      margin-right: 8px;\
    }\
    .status-online {\
      background-color: #2ecc71;\
      box-shadow: 0 0 8px #2ecc71;\
    }\
    .footer {\
      text-align: center;\
      margin-top: 30px;\
      font-size: 0.8rem;\
      color: #a0aec0;\
      grid-column: 1 / -1;\
    }\
    @media (max-width: 900px) {\
      .dashboard {\
        grid-template-columns: 1fr;\
      }\
    }\
    @media (max-width: 600px) {\
      .form-group {\
        flex-direction: column;\
        align-items: flex-start;\
      }\
      label {\
        margin-bottom: 5px;\
      }\
      select, input[type='number'] {\
        width: 100%;\
        margin-bottom: 10px;\
      }\
    }\
  </style>\
</head>\
<body>\
  <div class='container'>\
    <header>\
      <h1>Control de Test Sikkounter </h1>\
      <div><span class='status-indicator status-online'></span>Sistema conectado</div>\
    </header>\
    \
    <div class='dashboard'>\
      <div class='card'>\
        <h2>Modo de Operación</h2>\
        <div class='form-group'>\
          <label>Selecciona el modo:</label>\
          <select id='modoSelect'>\
            <option value='0'"
                  + String((modo == 0) ? " selected" : "") + ">Barrera Simple</option>\
            <option value='1'"
                  + String((modo == 1) ? " selected" : "") + ">Barrera con Lazo</option>\
            <option value='2'"
                  + String((modo == 2) ? " selected" : "") + ">Eventos (L0-L2)</option>\
          </select>\
          <button class='boton-update' onclick='cambiarModo()'>Cambiar Modo</button>\
        </div>";

    // Mostrar selección de eventos solo si el modo es Eventos
    if (modo == 2) {
      html += "<div class='form-group'>\
          <label>Evento:</label>\
          <select id='eventoSelect'>\
            <option value='0'"
              + String((eventos.getEvento() == 0) ? " selected" : "") + ">L0</option>\
            <option value='1'"
              + String((eventos.getEvento() == 1) ? " selected" : "") + ">L1</option>\
            <option value='2'"
              + String((eventos.getEvento() == 2) ? " selected" : "") + ">L2</option>\
          </select>\
          <button class='boton-update' onclick='cambiarEvento()'>Cambiar Evento</button>\
        </div>";
    }

    html += "    </div>\
      \
      <div class='card'>\
        <h2>Control de Prueba</h2>\
        <p>Presiona play para iniciar el test</p>\
        <button class='boton-start' onclick=\"fetch('/start')\">Iniciar</button>\
      </div>\
      \
      <div class='card'>\
        <h2>Configuración de Tiempos</h2>\
        <div class='form-group'>\
          <label>Tiempo para abrir (s):</label>\
          <input type='number' id='abrir' class='input-open' value='"
            + String(tiempoAbrir) + "' min='1'>\
        </div>\
        <div class='form-group'>\
          <label>Tiempo para cerrar (s):</label>\
          <input type='number' id='cerrar' class='input-close' value='"
            + String(tiempoCerrar) + "' min='1'>\
        </div>\
        <div class='form-group'>\
          <button class='boton-update' onclick='cambiarTiempos()'>Actualizar Tiempos</button>\
        </div>\
      </div>\
      \
      <div class='card'>\
        <h2>Contador de Eventos</h2>\
        <div class='counter-display'>\
          <span id='contador'>0</span>\
        </div>\
        <form method='POST' action='/reset-counter' onsubmit=\"return confirm('¿Estás seguro de que deseas reiniciar el contador?');\">\
          <input type='submit' value='Reiniciar Contador' />\
        </form>\
      </div>\
      \
      <div class='card full-width'>\
        <h2>Logs del sistema</h2>\
        <div class='log-container'>\
          <pre id='logContent' style='margin: 0;'></pre>\
        </div>\
      </div>\
    </div>\
    \
    <div class='footer'>\
      <p>Sistema de testing para contadores sikker • ESP32</p>\
    </div>\
  </div>\
  \
  <script>\
    function cambiarModo(){\
      const modo = document.getElementById('modoSelect').value;\
      fetch('/set-mode',{\
        method: 'POST',\
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},\
        body: 'modo=' + modo\
      })\
      .then(response => {\
        if(response.ok) {\
          location.reload();\
        }\
      });\
    }\
    \
    function cambiarEvento(){\
      const evento = document.getElementById('eventoSelect').value;\
      fetch('/set-evento',{\
        method: 'POST',\
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},\
        body: 'evento=' + evento\
      })\
      .then(response => {\
        if(response.ok) {\
          location.reload();\
        }\
      });\
    }\
    \
    function cambiarTiempos(){\
      const abrir = document.getElementById('abrir').value;\
      const cerrar = document.getElementById('cerrar').value;\
      if(abrir < 1 || cerrar < 1) {\\
        alert('Los tiempos deben ser al menos 1 segundo');\
        return;\
      }\
      fetch('/setTimes',{\
        method: 'POST',\
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},\
        body: 'abrir=' + abrir + '&cerrar=' + cerrar\
      })\
      .then(response => {\
        if(response.ok) {\
          location.reload();\
        }\
      });\
    }\
    \
    function actualizarConteo(){\
      fetch('/get-counter')\
        .then(r => r.text())\
        .then(d => {\
          document.getElementById('contador').innerText = d;\
        });\
    }\
    \
    function actualizarLogs(){\
      fetch('/event-log')\
        .then(r => r.text())\
        .then(d => {\
          document.getElementById('logContent').innerText = d;\
          const container = document.querySelector('.log-container');\
          container.scrollTop = container.scrollHeight;\
        });\
    }\
    \
    setInterval(actualizarConteo, 1000);\
    setInterval(actualizarLogs, 3000);\
    actualizarConteo();\
    actualizarLogs();\
  </script>\
</body></html>";
    server.send(200, "text/html", html);
  }

  void handleSetTimes() {
    if (server.hasArg("abrir") && server.hasArg("cerrar")) {
      tiempoAbrir = server.arg("abrir").toInt();
      tiempoCerrar = server.arg("cerrar").toInt();

      // Actualizar AMBOS modos
      modoSimple.setTiempoAbrir(tiempoAbrir);
      modoSimple.setTiempoCerrar(tiempoCerrar);

      modoLazo.setTiempoAbrir(tiempoAbrir);
      modoLazo.setTiempoCerrar(tiempoCerrar);

      Serial.println("Tiempos actualizados en ambos modos:");
      Serial.println("Abrir: " + String(tiempoAbrir));
      Serial.println("Cerrar: " + String(tiempoCerrar));
    }
    server.send(200, "text/plain", "OK");
  }
};
