# 🔥 Sistema Inteligente de Detección de Fugas de Gas en Ambientes Domésticos

**ESP32 + Sensor MQ-2 + MQTT + Dashboard Web en Tiempo Real**

Martin Ortega · Alejandro Riveros · Camilo Torres  
Universidad de la Sabana — 2026

---

## 1. Introducción

El gas natural y el GLP (Gas Licuado de Petróleo) son fuentes de energía ampliamente utilizadas en hogares colombianos para cocción y calefacción. Las fugas de gas representan uno de los riesgos domésticos más graves: son invisibles, inodoras en su forma pura y en concentraciones críticas pueden provocar explosiones, incendios o intoxicaciones.

Este proyecto implementa un sistema de bajo costo basado en el microcontrolador ESP32 y el sensor MQ-2, capaz de monitorear en tiempo real la concentración de gases inflamables, generar alertas locales (LEDs y buzzer), publicar datos vía MQTT con encriptación TLS y mostrar un dashboard web accesible desde cualquier dispositivo en la misma red.

---

## 2. Arquitectura del Sistema (Diagrama de Bloques)
<img width="911" height="391" alt="Diagrama_Bloques_1_Proyecto_Final_IoT" src="https://github.com/user-attachments/assets/04732d10-3ac9-452c-bc6c-8764e857ca0f" />

---

## 3. Diagrama de Secuencia
<img width="508" height="1361" alt="Diagrama_de_flujo_IoT drawio" src="https://github.com/user-attachments/assets/b97daf58-3dfa-4425-ab25-9bc5279e1d24" />

---

## 4. Temas MQTT

| Topic | Tipo | Frecuencia | Descripción |
|---|---|---|---|
| `unisabana/gas/datos` | Publica | Cada 2s | PPM, ADC, estado, timestamp, IP |
| `unisabana/gas/status` | Publica | Al conectar | Estado online/offline |
| `unisabana/gas/healthcheck` | Publica | Cada 30s | Uptime, IP, timestamp |

**Broker:** `broker.hivemq.com`  
**Puerto MQTT:** `1883`  
**Puerto WebSocket:** `8080`

**Ejemplo de payload `unisabana/gas/datos`:**
```json
{
  "timestamp": "2026-05-25T19:03:36",
  "adc": 1911,
  "ppm": 63.1,
  "estado": "SEGURO",
  "ip": "10.223.72.164"
}
```

**Ejemplo de payload `unisabana/gas/healthcheck`:**
```json
{
  "status": "ok",
  "uptime": 3600,
  "ip": "10.223.72.164"
}
```

---

## 5. Endpoints API (Servidor Web ESP32)

| Endpoint | Método | Descripción | Respuesta |
|---|---|---|---|
| `http://<IP>/` | GET | Dashboard web en tiempo real | HTML |
| `http://<IP>/data` | GET | Datos actuales del sensor | JSON |
| `http://<IP>/health` | GET | Healthcheck del sistema | JSON |

**Ejemplo respuesta `/data`:**
```json
{
  "timestamp": "2026-05-25T19:03:36",
  "adc": 1911,
  "ppm": 63.1,
  "estado": "SEGURO",
  "ip": "10.223.72.164"
}
```

**Ejemplo respuesta `/health`:**
```json
{
  "status": "ok",
  "uptime": 3600,
  "timestamp": "2026-05-25T19:03:36",
  "ip": "10.223.72.164"
}
```

---

## 6. Esquema de Conexiones

| Componente | Pin ESP32 |
|---|---|
| MQ-2 VCC | 5V (VIN) |
| MQ-2 GND | GND |
| MQ-2 AOUT | GPIO34 |
| MQ-2 DOUT | GPIO35 |
| LED Rojo (+) | GPIO26 |
| LED Verde (+) | GPIO27 |
| LED Amarillo (+) | GPIO14 |
| Buzzer (+) | GPIO25 |
| Todos los (-) | GND |

> ⚠️ Los LEDs llevan resistencia de 220Ω en serie.

---

## 7. Librerías Utilizadas

| Librería | Versión | Uso |
|---|---|---|
| `WiFi.h` | built-in ESP32 | Conexión WiFi |
| `WebServer.h` | built-in ESP32 | Servidor web y endpoints |
| `PubSubClient` | 2.8 | Cliente MQTT |
| `WiFiClientSecure` | built-in ESP32 | TLS/SSL |
| `time.h` | built-in | Sincronización NTP |
| `Wire.h` | built-in | I2C |
| `Adafruit SSD1306` | 2.5.16 | Pantalla OLED |
| `Adafruit GFX Library` | 1.12.6 | Gráficos OLED |

---

## 8. Uso de Memoria

Generado por Arduino IDE al compilar:
RAM:   6.8%  — 22.132 bytes usados de 327.680 bytes
Flash: 23.5% — 308.273 bytes usados de 1.310.720 bytes

---

## 9. Sincronización NTP

El sistema sincroniza su reloj con servidores de tiempo de internet al iniciar:

```cpp
configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
```

- **Zona horaria:** UTC-5 (Colombia)
- **Servidores:** pool.ntp.org, time.nist.gov
- **Uso:** Cada mensaje MQTT y cada respuesta HTTP lleva timestamp real

---

## 10. Lógica de Estados

| PPM | Estado | LED | Buzzer |
|---|---|---|---|
| < 300 | SEGURO | 🟢 Verde | Apagado |
| 300 – 999 | ADVERTENCIA | 🟡 Amarillo | Apagado |
| ≥ 1000 | PELIGRO | 🔴 Rojo | Encendido |

---

## 11. Limitaciones

- El sensor MQ-2 no distingue entre tipos de gas; mide concentración total de gases inflamables.
- Requiere calibración periódica del valor R0 para mantener precisión.
- La lectura en PPM es aproximada mediante curva logarítmica; sirve como indicador relativo de riesgo.
- El broker MQTT público (HiveMQ) puede presentar intermitencia en redes universitarias con puertos bloqueados.
- La pantalla OLED fue reemplazada por el dashboard web por problemas de compatibilidad en el prototipo físico.
- El sistema depende de conectividad WiFi para MQTT y dashboard; en modo sin WiFi solo funcionan las alertas locales.

---

## 12. Posibilidades de Mejora

- Implementar autenticación MQTT con usuario y contraseña para mayor seguridad.
- Agregar almacenamiento de histórico en base de datos (InfluxDB o Firebase).
- Integrar notificaciones por Telegram o correo electrónico al detectar peligro.
- Implementar calibración automática del sensor al encendido usando NVS/EEPROM.
- Añadir modo de bajo consumo (deep sleep) para operación con batería.
- Escalar el sistema a múltiples sensores en distintas habitaciones.
- Implementar OTA (Over The Air) para actualización remota del firmware.

---

## 13. Lista de Materiales

| Componente | Descripción | Precio Aprox. (COP) | Cantidad |
|---|---|---|---|
| ESP32 (WROOM-32) | Microcontrolador principal con WiFi/Bluetooth | $25.000 – $40.000 | 1 |
| Sensor MQ-2 | Detecta GLP, butano, metano y humo | $8.000 – $15.000 | 1 |
| Buzzer activo 5V | Alarma sonora ante detección de fuga | $2.000 – $4.000 | 1 |
| LEDs (rojo/verde/amarillo) | Indicación visual del estado | $1.000 – $2.000 | 3 |
| Protoboard 830 puntos | Montaje y pruebas | $6.000 – $10.000 | 1 |
| Cables jumper | Conexiones del circuito | $5.000 – $8.000 | 1 set |
| Resistencias 220Ω | Protección LEDs | $1.000 – $2.000 | 3 |
| Fuente 5V / Cargador USB | Alimentación | $8.000 – $15.000 | 1 |

**Costo total estimado: $56.000 – $96.000 COP**

---

## 14. Referencias

- Espressif Systems. (2024). *ESP32 Technical Reference Manual*. https://docs.espressif.com
- Winsen Electronics. (2023). *MQ-2 Semiconductor Sensor for Combustible Gas Datasheet*.
- HiveMQ. (2025). *MQTT Essentials*. https://www.hivemq.com/mqtt-essentials/
- Adafruit. (2024). *Adafruit SSD1306 Library Documentation*. https://github.com/adafruit/Adafruit_SSD1306
- Knolleary. (2023). *PubSubClient Arduino Library*. https://github.com/knolleary/pubsubclient

---

## 15. Prototipo Físico

---

## 16. Dashboard web

