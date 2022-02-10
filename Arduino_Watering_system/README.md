# Arduino Watering System
Low-cost irrigation automation project with real-time analysis of environmental conditions

- **Objectives:** Watering System - Irrigation and Monitoring Water Level 
- **Behavior:** When the soil is dry and water level is low

## Files

The main code is on *Arduino_Watering_system.ino*
>To compile the file, first might be necessary to install required libraries.

## Required libraries

 - Adafruit_GFX_Library
 - Adafruit_SSD1306
 - DHTLib
 - RTClib
 - Wire

 
## Required hardware
 - Arduino UNO
 - Display Oled 0.96 I2c 
 - LM-393 DRIVER
 - Real Time Clock Module
 - DHT11 Sensor
 - Relay Module
 - HC-SR04 Module
 - LDR Sensor
 - BUZZER
 - LED
 -  12V water pump
 - 12V power  supply

## Conections

**RTC (Real Time Clock <-> Arduino:**	
	
	GND <-> GND
	+5 V <-> +5 V
	SDA <-> A4
	SCL <-> A5

**Relay <-> Arduino:**

	GND <-> GND
	+5 V <-> +5 V
	IN <-> D3

**LCD <-> Arduino:**

	GND <-> GND
	+5 V <-> +5 V
	SDA <-> SDA
	SCL <-> SCL

**BUZZER <-> Arduino:**

	VCC <-> D6
	GND <-> GND

**LED (Pump on indicator) <-> Arduino:**

	VCC <-> D8
	GND <-> GND

**LM-393 DRIVER (moisture sensor) <-> Arduino:**

	GND <-> GND
	+5 V <-> +5 V
	OUT <-> A0

**DHT11 (Temperature and Humidity) <-> Arduino:**

	GND <-> GND
	+5 V <-> +5 V
	OUT <-> D4

**LDR (Light Mesure) <-> Arduino:**

	VCC <-> A1
	GND <-> GND

**HC-SR04 (Ultrason) <-> Arduino:**

	GND <-> GND
	+5 V <-> +5 V
	EchoPin <-> D12
	TrigPin <-> D13



## Credits
 - Ricardo Silva Moreira 
 - Contact: silvaricardo1997@gmail.com 
 - Date: 23/07/2019 
 - Place: Brazil, Brasília 
 - Version: 19.07.23.Pre-alpha




