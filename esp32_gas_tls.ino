#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>   // <-- antes era WiFiClient
#include <WebServer.h>
#include <PubSubClient.h>
#include <time.h>

// -------------------- WiFi --------------------
const char* ssid     = "No se aceptan petristas";
const char* password = "compredatospobre";

// -------------------- MQTT (TLS) --------------------
const char* mqtt_server  = "broker.hivemq.com";
const int   mqtt_port    = 8883;                  // <-- puerto TLS
const char* mqtt_client  = "ESP32_GasDetector_UniSabana";
const char* topic_datos  = "unisabana/gas/datos";
const char* topic_status = "unisabana/gas/status";
const char* topic_health = "unisabana/gas/healthcheck";

// -------------------- Servidor Web --------------------
WebServer server(80);

// -------------------- PINES --------------------
#define GAS_AOUT  34
#define GAS_DOUT  35
#define LED_ROJO  26
#define LED_VERDE 27
#define LED_AMAR  14
#define BUZZER    25

// -------------------- UMBRALES --------------------
#define UMBRAL_ADVERTENCIA 300.0
#define UMBRAL_PELIGRO     1000.0

// -------------------- OBJETOS --------------------
WiFiClientSecure espClient;          // <-- cliente seguro
PubSubClient     mqttClient(espClient);

// -------------------- VARIABLES --------------------
float  ppm    = 0;
int    rawADC = 0;
String estado = "SEGURO";
bool   wifiOK = false;
bool   mqttOK = false;
unsigned long lastMsg    = 0;
unsigned long lastHealth = 0;

// -------------------- FUNCIONES --------------------
float adcToPPM(int adc) {
  adc = constrain(adc, 0, 4095);
  float normalized = adc / 4095.0;
  float logPPM = log10(0.1) + normalized * (log10(100000.0) - log10(0.1));
  return pow(10, logPPM);
}

void buzzerOn()  { digitalWrite(BUZZER, HIGH); }
void buzzerOff() { digitalWrite(BUZZER, LOW);  }

void aplicarEstado() {
  if (ppm >= UMBRAL_PELIGRO) {
    estado = "PELIGRO";
    digitalWrite(LED_ROJO,  HIGH);
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMAR,  LOW);
    buzzerOn();
  } else if (ppm >= UMBRAL_ADVERTENCIA) {
    estado = "ADVERTENCIA";
    digitalWrite(LED_ROJO,  LOW);
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMAR,  HIGH);
    buzzerOff();
  } else {
    estado = "SEGURO";
    digitalWrite(LED_ROJO,  LOW);
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_AMAR,  LOW);
    buzzerOff();
  }
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "sin-hora";
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buf);
}

void conectarWiFi() {
  Serial.print("Conectando WiFi");
  WiFi.begin(ssid, password);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiOK = true;
    Serial.println("\nWiFi OK. IP: " + WiFi.localIP().toString());
  } else {
    wifiOK = false;
    Serial.println("\nWiFi FALLO. Modo sin conexion.");
  }
}

void conectarMQTT() {
  if (!wifiOK) return;

  // --- TLS: encripta la conexion sin validar certificado ---
  // Los datos viajan cifrados (igual que HTTPS), simplemente
  // no se verifica la identidad del servidor.
  espClient.setInsecure();

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(512);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(15);

  Serial.print("Conectando MQTT (TLS)...");
  String clientId = "ESP32_Gas_" + String(random(0xffff), HEX) + String(millis());
  if (mqttClient.connect(clientId.c_str())) {
    mqttOK = true;
    Serial.println("OK (encriptado)");
    mqttClient.publish(topic_status, "{\"status\":\"online\",\"tls\":true}", true);
  } else {
    mqttOK = false;
    Serial.println("FALLO rc=" + String(mqttClient.state()));
  }
}

// -------------------- ENDPOINTS WEB --------------------
void handleHealth() {
  String json = "{";
  json += "\"status\":\"ok\",";
  json += "\"uptime\":"      + String(millis() / 1000) + ",";
  json += "\"timestamp\":\"" + getTimestamp() + "\",";
  json += "\"ip\":\""        + WiFi.localIP().toString() + "\",";
  json += "\"tls\":true";
  json += "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Detector de Gas - UniSabana</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { font-family: 'Segoe UI', sans-serif; background: #0f0f1a; color: #fff; min-height: 100vh; padding: 20px; }
    h1 { text-align: center; font-size: 1.8em; margin-bottom: 5px; color: #00d4ff; }
    .subtitle { text-align: center; color: #888; margin-bottom: 30px; font-size: 0.9em; }
    .cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; max-width: 900px; margin: 0 auto 30px auto; }
    .card { background: #1a1a2e; border-radius: 16px; padding: 24px; text-align: center; border: 1px solid #2a2a4a; }
    .card .label { font-size: 0.85em; color: #888; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1px; }
    .card .value { font-size: 2.2em; font-weight: bold; color: #00d4ff; }
    .card .unit { font-size: 0.8em; color: #666; margin-top: 4px; }
    .estado-card { max-width: 900px; margin: 0 auto 30px auto; background: #1a1a2e; border-radius: 16px; padding: 30px; text-align: center; border: 2px solid #2a2a4a; transition: all 0.3s; }
    .estado-card .estado-label { font-size: 0.9em; color: #888; margin-bottom: 10px; text-transform: uppercase; }
    .estado-card .estado-valor { font-size: 3em; font-weight: bold; }
    .SEGURO { border-color: #00ff88; }
    .SEGURO .estado-valor { color: #00ff88; }
    .ADVERTENCIA { border-color: #ffaa00; }
    .ADVERTENCIA .estado-valor { color: #ffaa00; }
    .PELIGRO { border-color: #ff3333; animation: pulse 1s infinite; }
    .PELIGRO .estado-valor { color: #ff3333; }
    @keyframes pulse { 0% { box-shadow: 0 0 0 0 rgba(255,51,51,0.4); } 70% { box-shadow: 0 0 0 15px rgba(255,51,51,0); } 100% { box-shadow: 0 0 0 0 rgba(255,51,51,0); } }
    .chart-container { max-width: 900px; margin: 0 auto 30px auto; background: #1a1a2e; border-radius: 16px; padding: 24px; border: 1px solid #2a2a4a; }
    .chart-title { font-size: 0.9em; color: #888; margin-bottom: 16px; text-transform: uppercase; }
    .leds { display: flex; justify-content: center; gap: 20px; margin: 20px 0; }
    .led { width: 30px; height: 30px; border-radius: 50%; opacity: 0.2; transition: all 0.3s; }
    .led.on { opacity: 1; box-shadow: 0 0 15px currentColor; }
    .led-r { background: #ff3333; color: #ff3333; }
    .led-y { background: #ffaa00; color: #ffaa00; }
    .led-g { background: #00ff88; color: #00ff88; }
    .mqtt-status { max-width: 900px; margin: 0 auto 20px auto; background: #1a1a2e; border-radius: 12px; padding: 16px 24px; border: 1px solid #2a2a4a; display: flex; justify-content: space-between; align-items: center; }
    .mqtt-dot { width: 12px; height: 12px; border-radius: 50%; display: inline-block; margin-right: 8px; transition: background 0.3s; }
    .dot-verde { background: #00ff88; }
    .dot-rojo  { background: #ff3333; }
    .dot-amar  { background: #ffaa00; }
    .timestamp { text-align: center; color: #555; font-size: 0.8em; margin-top: 10px; }
    .lock { color: #00ff88; font-weight: bold; }
  </style>
</head>
<body>
  <h1>🔥 Detector de Gas MQ-2</h1>
  <p class="subtitle">Universidad de la Sabana 2026 — Sistema IoT <span class="lock">🔒 TLS</span></p>

  <div class="mqtt-status">
    <span><span class="mqtt-dot dot-amar" id="mqttDot"></span><span id="mqttLabel">Conectando a broker MQTT (WSS)...</span></span>
    <span id="uptime">Uptime: --</span>
  </div>

  <div class="estado-card SEGURO" id="estadoCard">
    <div class="estado-label">Estado del sistema</div>
    <div class="estado-valor" id="estadoValor">SEGURO</div>
    <div class="leds">
      <div class="led led-r" id="ledR"></div>
      <div class="led led-y" id="ledY"></div>
      <div class="led led-g on" id="ledG"></div>
    </div>
  </div>

  <div class="cards">
    <div class="card">
      <div class="label">Concentración</div>
      <div class="value" id="ppm">--</div>
      <div class="unit">PPM</div>
    </div>
    <div class="card">
      <div class="label">ADC Raw</div>
      <div class="value" id="adc">--</div>
      <div class="unit">0-4095</div>
    </div>
    <div class="card">
      <div class="label">IP del sensor</div>
      <div class="value" style="font-size:1em;margin-top:10px" id="ip">--</div>
      <div class="unit">ESP32</div>
    </div>
  </div>

  <div class="chart-container">
    <div class="chart-title">📈 Historial PPM desde broker MQTT (últimos 30 valores)</div>
    <canvas id="chart" height="80"></canvas>
  </div>

  <p class="timestamp" id="ts">Esperando datos del broker...</p>

  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script src="https://unpkg.com/mqtt/dist/mqtt.min.js"></script>
  <script>
    const MAX = 30;
    const labels = Array(MAX).fill('');
    const data   = Array(MAX).fill(0);

    const ctx = document.getElementById('chart').getContext('2d');
    const chart = new Chart(ctx, {
      type: 'line',
      data: {
        labels,
        datasets: [{
          label: 'PPM',
          data,
          borderColor: '#00d4ff',
          backgroundColor: 'rgba(0,212,255,0.1)',
          borderWidth: 2,
          pointRadius: 0,
          fill: true,
          tension: 0.4
        }]
      },
      options: {
        animation: false,
        scales: {
          x: { display: false },
          y: { beginAtZero: true, grid: { color: '#2a2a4a' }, ticks: { color: '#888' } }
        },
        plugins: { legend: { display: false } }
      }
    });

    const mqttDot   = document.getElementById('mqttDot');
    const mqttLabel = document.getElementById('mqttLabel');

    // WSS = WebSockets Seguro (encriptado TLS) en el puerto 8884
    const client = mqtt.connect('wss://broker.hivemq.com:8884/mqtt');

    client.on('connect', function () {
      mqttDot.className   = 'mqtt-dot dot-verde';
      mqttLabel.textContent = '🔒 Conectado a broker.hivemq.com (MQTT/WSS - TLS)';
      client.subscribe('unisabana/gas/datos');
      client.subscribe('unisabana/gas/healthcheck');
    });

    client.on('error', function (err) {
      mqttDot.className   = 'mqtt-dot dot-rojo';
      mqttLabel.textContent = 'Error broker MQTT: ' + err.message;
    });

    client.on('offline', function () {
      mqttDot.className   = 'mqtt-dot dot-rojo';
      mqttLabel.textContent = 'Desconectado del broker';
    });

    client.on('message', function (topic, message) {
      try {
        const d = JSON.parse(message.toString());

        if (topic === 'unisabana/gas/datos') {
          document.getElementById('ppm').textContent = parseFloat(d.ppm).toFixed(1);
          document.getElementById('adc').textContent = d.adc;
          document.getElementById('ip').textContent  = d.ip;
          document.getElementById('ts').textContent  = 'Última actualización (vía MQTT/TLS): ' + d.timestamp;

          const card = document.getElementById('estadoCard');
          card.className = 'estado-card ' + d.estado;
          document.getElementById('estadoValor').textContent = d.estado;

          document.getElementById('ledR').className = 'led led-r' + (d.estado === 'PELIGRO'     ? ' on' : '');
          document.getElementById('ledY').className = 'led led-y' + (d.estado === 'ADVERTENCIA' ? ' on' : '');
          document.getElementById('ledG').className = 'led led-g' + (d.estado === 'SEGURO'      ? ' on' : '');

          data.shift();   data.push(parseFloat(d.ppm));
          labels.shift(); labels.push(d.timestamp.slice(11,19));
          chart.update();
        }

        if (topic === 'unisabana/gas/healthcheck') {
          document.getElementById('uptime').textContent = 'Uptime: ' + d.uptime + 's';
        }

      } catch(e) { console.error('Error parsing MQTT message:', e); }
    });

    async function fetchHealth() {
      try {
        const res = await fetch('/health');
        const d   = await res.json();
        document.getElementById('uptime').textContent = 'Uptime: ' + d.uptime + 's';
      } catch(e) {}
    }
    setInterval(fetchHealth, 10000);
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_ROJO,  OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMAR,  OUTPUT);
  pinMode(BUZZER,    OUTPUT);
  pinMode(GAS_DOUT,  INPUT);

  digitalWrite(LED_ROJO,  LOW);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMAR,  LOW);
  buzzerOff();

  analogReadResolution(12);
  analogSetPinAttenuation(GAS_AOUT, ADC_11db);

  Serial.println("Sensor iniciado.");
  conectarWiFi();

  if (wifiOK) {
    configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    delay(2000);
    Serial.println("Hora: " + getTimestamp());
    conectarMQTT();
    server.on("/",       handleRoot);
    server.on("/health", handleHealth);
    server.begin();
    Serial.println("Dashboard en: http://" + WiFi.localIP().toString());
  }

  Serial.println("Sistema listo.");
}

// -------------------- LOOP --------------------
void loop() {
  rawADC = analogRead(GAS_AOUT);
  ppm    = adcToPPM(rawADC);
  aplicarEstado();

  Serial.printf("ADC: %d | PPM: %.1f | Estado: %s\n", rawADC, ppm, estado.c_str());

  if (wifiOK) {
    server.handleClient();

    if (!mqttClient.connected()) conectarMQTT();
    mqttClient.loop();

    unsigned long now = millis();

    if (now - lastMsg > 2000) {
      lastMsg = now;
      String payload = "{";
      payload += "\"timestamp\":\"" + getTimestamp() + "\",";
      payload += "\"adc\":"         + String(rawADC) + ",";
      payload += "\"ppm\":"         + String(ppm, 1) + ",";
      payload += "\"estado\":\""    + estado + "\",";
      payload += "\"ip\":\""        + WiFi.localIP().toString() + "\"";
      payload += "}";
      mqttClient.publish(topic_datos, payload.c_str());
    }

    if (now - lastHealth > 30000) {
      lastHealth = now;
      String health = "{\"status\":\"ok\",\"uptime\":" + String(millis()/1000) + ",\"ip\":\"" + WiFi.localIP().toString() + "\",\"tls\":true}";
      mqttClient.publish(topic_health, health.c_str());
    }
  }

  delay(500);
}
