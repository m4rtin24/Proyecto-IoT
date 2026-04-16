# Sistema Inteligente de Detección de Fugas de Gas en Ambientes Domésticos

Usando ESP32 y Sensores MQ

Martin Ortega, 
Alejandro Riveros, 
Camilo Torres

Universidad de la Sabana 2026

**1. Introducción**

El gas natural y el GLP (Gas Licuado de Petróleo) son fuentes de energía ampliamente utilizadas en hogares colombianos para la cocción de alimentos y calefacción. Sin embargo, las fugas de gas representan uno de los riesgos domésticos más graves y silenciosos: son inodoras en su forma pura, invisibles y, en concentraciones críticas, pueden provocar explosiones, incendios o intoxicaciones que ponen en peligro la vida de las personas.

Este proyecto propone el diseño e implementación de un sistema de bajo costo basado en el microcontrolador ESP32 y sensores electroquímicos tipo MQ, capaz de monitorear en tiempo real la concentración de gases inflamables en ambientes domésticos como la cocina, generar alertas locales (sonoras y visuales) y, opcionalmente, enviar notificaciones remotas a través de WiFi.

**2. Importancia y Justificación**

**2.1 El Problema de las Fugas de Gas en el Hogar**

Cada año se registran en Colombia y América Latina cientos de accidentes domésticos relacionados con fugas de gas. La mayoría ocurre porque:

- Las fugas pequeñas o lentas son imperceptibles para el olfato humano durante horas.
- Muchos hogares carecen de ventilación adecuada, lo que permite la acumulación de gas.
- Las personas mayores, niños o quienes padecen problemas respiratorios son especialmente vulnerables a la intoxicación por monóxido de carbono asociado.
- La detección tardía convierte una fuga menor en una tragedia que pudo evitarse.

**2.2 Riesgos para la Salud**

La exposición a gases como el metano, butano o propano en concentraciones elevadas puede causar: mareos, náuseas, pérdida de consciencia, y en casos extremos, la muerte por asfixia o intoxicación. Adicionalmente, la presencia de GLP en el aire entre el 1.8% y el 9.5% de concentración forma una mezcla explosiva que puede detonarse con una simple chispa eléctrica.

**2.3 Relevancia Tecnológica y Social**

Un sistema de detección automatizado y asequible puede marcar la diferencia en hogares de bajos y medianos recursos donde no se cuenta con detectores comerciales. Este proyecto demuestra cómo la electrónica de bajo costo y el Internet de las Cosas (IoT) pueden tener un impacto directo y positivo en la seguridad doméstica, alineándose con los Objetivos de Desarrollo Sostenible relacionados con salud, seguridad y ciudades sostenibles.

**3. Objetivos del Proyecto**

**3.1 Objetivo General**

Diseñar e implementar un sistema de detección de fugas de gas de bajo costo utilizando el microcontrolador ESP32 y sensores MQ, capaz de emitir alertas locales y remotas ante niveles peligrosos de concentración de gas en ambientes domésticos.

**3.2 Objetivos Específicos**

- Integrar el sensor MQ-2 con el ESP32 para la lectura analógica de concentración de gases inflamables.
- Establecer umbrales de alerta (advertencia y peligro) programados en el firmware del ESP32.
- Implementar un sistema de alerta local mediante buzzer y LEDs indicadores.
- Mostrar las lecturas en tiempo real a través de una pantalla OLED.
- Enviar notificaciones remotas vía WiFi (protocolo MQTT o servidor HTTP simple).
- Validar el sistema en un entorno controlado simulando condiciones de fuga.

**4. Descripción Técnica del Sistema**

**4.1 Arquitectura General**

El sistema se compone de tres bloques funcionales principales: (1) Adquisición de datos, a cargo del sensor MQ-2 que convierte la concentración de gas en una señal analógica; (2) Procesamiento y decisión, realizado por el ESP32 que lee la señal ADC, la interpreta y evalúa frente a los umbrales configurados; y (3) Actuación y comunicación, que incluye las alertas locales (buzzer, LEDs, pantalla OLED) y la notificación remota vía WiFi.

**4.2 Flujo de Operación**

- El sensor MQ-2 calienta su elemento sensible (requiere ~30 segundos de precalentamiento).
- El ESP32 lee la señal analógica en el pin ADC y la convierte a un valor de concentración estimado en ppm (partes por millón).
- El valor se compara con dos umbrales: Advertencia (ej. 300 ppm) y Peligro (ej. 1000 ppm).
- Según el nivel detectado, se activan los indicadores: LED verde (seguro), LED amarillo (advertencia) o LED rojo + buzzer (peligro).
- La pantalla OLED muestra el nivel actual y el estado del sistema.
- En nivel de peligro, el ESP32 envía una alerta por WiFi (notificación a app o correo).

**4.3 Consideraciones de Calibración**

Los sensores MQ requieren calibración en aire limpio para determinar el valor de resistencia de referencia (R0). El ESP32 puede realizar esta calibración automáticamente al encenderse en un ambiente ventilado, almacenando el valor en su memoria no volátil (NVS/EEPROM).

**5. Lista de Materiales y Componentes**

A continuación se detalla el listado de materiales necesarios para el prototipo funcional. Los precios son estimados para el mercado colombiano (Mercado Libre, MercadoExpress, tiendas como Electronilab o Didácticas Electrónicas):

| Componente                  | Descripción                                             | Precio Aprox. (COP) | Cantidad     |
| --------------------------- | ------------------------------------------------------- | ------------------- | ------------ |
| ESP32 (WROOM-32)            | Microcontrolador principal con WiFi/Bluetooth integrado | $25.000 – $40.000   | 1            |
| Sensor MQ-2                 | Detecta GLP, butano, metano y humo                      | $8.000 – $15.000    | 1            |
| Sensor MQ-5                 | Específico para GLP y gas natural (complementario)      | $8.000 – $15.000    | 1 (opcional) |
| Buzzer activo 5V            | Alarma sonora ante detección de fuga                    | $2.000 – $4.000     | 1            |
| LED RGB o LEDs (rojo/verde) | Indicación visual del estado del sistema                | $1.000 – $2.000     | 2–3          |
| Módulo OLED 0.96" (I2C)     | Pantalla para mostrar lecturas en tiempo real           | $8.000 – $12.000    | 1            |
| Protoboard 830 puntos       | Para montaje y pruebas del circuito                     | $6.000 – $10.000    | 1            |
| Cables jumper (M-M, M-H)    | Conexiones del circuito                                 | $5.000 – $8.000     | 1 set        |
| Resistencias (220Ω, 10kΩ)   | Para divisores de voltaje y protección                  | $1.000 – $2.000     | varios       |
| Fuente 5V / Cargador USB    | Alimentación del sistema                                | $8.000 – $15.000    | 1            |

Costo total estimado del prototipo: $72.000 – $123.000 COP

**6. Esquema de Conexiones**

**6.1 Pines ESP32 – MQ-2**

- VCC del MQ-2 → 5V del ESP32 (o fuente externa)
- GND del MQ-2 → GND del ESP32
- AO → GPIO34
- DO → GPIO35 (opcional)

**6.2 Pines ESP32 – Pantalla OLED (I2C)**

- SDA → GPIO21
- SCL → GPIO22
- VCC → 3.3V
- GND → GND

**6.3 Pines ESP32 – Actuadores**

- Buzzer → GPIO25
- LED Rojo → GPIO26
- LED Verde → GPIO27
- LED Amarillo → GPIO14

Nota importante: El ESP32 opera a 3.3V en sus pines GPIO. El sensor MQ-2 necesita 5V para su elemento calefactor, pero su salida analógica puede superar 3.3V — se recomienda usar un divisor de voltaje (2 resistencias) para proteger el pin ADC del ESP32.

**7. Lineamientos del Firmware**

El firmware se desarrollará en el entorno Arduino IDE o PlatformIO usando el framework de Arduino para ESP32.

Librerías:
- Adafruit_SSD1306
- Adafruit_GFX
- WiFi.h
- PubSubClient
- Preferences.h

El ciclo principal del programa leerá el ADC cada 500 ms, calculará la concentración en ppm mediante la fórmula de la curva de sensibilidad del MQ-2, actualizará la pantalla y evaluará los umbrales de alerta.

**8. Alcance y Limitaciones**

**8.1 Alcance**

- Prototipo funcional en entorno de laboratorio / cocina doméstica controlada.
- Detección de GLP, gas natural (metano), butano y propano.
- Alerta local y notificación remota básica (vía WiFi).
- Dashboard simple o notificación por Telegram Bot (extensión opcional).

**8.2 Limitaciones**

- El sensor MQ-2 no distingue entre tipos de gas; solo mide concentración total.
- Requiere calibración periódica para mantener precisión.
- La precisión de la lectura en ppm es aproximada; sirve como indicador relativo de riesgo.

**9. Conclusiones**

Este proyecto integra de forma práctica conceptos de electrónica, programación embebida e IoT para resolver un problema real de seguridad doméstica. El uso del ESP32 como plataforma principal ofrece una solución potente, versátil y económica que permite tanto el procesamiento local como la conectividad remota. La implementación de este tipo de sistemas contribuye directamente a la prevención de accidentes domésticos y puede escalar a soluciones más robustas en el futuro.

La viabilidad económica del prototipo (menos de $120.000 COP) y la disponibilidad de los componentes en el mercado local lo convierten en un proyecto completamente realizable dentro del contexto académico propuesto.
