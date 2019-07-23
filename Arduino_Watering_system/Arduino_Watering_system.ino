/*
#####################################################################################
#   File:               Arduino_Watering_system.ino                                             
#   Processor:          Arduino UNO, MEGA       
#   Language:     Wiring / C /Fritzing / Arduino IDE          
#           
# Objectives:         Watering System - Irrigation and Monitoring
#                     
# Behavior:     When the soil is dry and water level is low
#                           
#  
#   Author:                 Ricardo Silva Moreira 
#   Contato:                silvaricardo1997@gmail.com
#   Date:                   23/07/2019  
#   place:                  Brazil, Brasília
#   Versão:                19.07.23.Pre-alpha
#         
#####################################################################################

*/

/*%%%% DESCRIPTION OF MAIN CONECTIONS %%%%%

 * RTC(Real Time Clock <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
    SDA  <-> A4
    SCL  <-> A5
 
 * Relay <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
     IN  <-> D3

 * LCD <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
    SDA  <-> SDA
    SCL  <-> SCL

  *BUZZER <-> Arduino:
    + <-> D6
    - <-> GND

  *LED (Pump on indicator) <-> Arduino:
    + <-> D8
    - <-> GND

 *LM-393 DRIVER (moisture sensor) <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
    OUT  <-> A0
 
 *  DHT11 (Temperature and Humidity) <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
    OUT  <-> D4

 *LDR (Light Mesure) <-> Arduino:
    + <-> A1
    - <-> GND

 *HC-SR04 (Ultrason) <-> Arduino:
     GND     <-> GND
    +5 V     <-> +5 V
    EchoPin  <-> D12
    TrigPin  <-> D13
 */


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h> //for this must add lib file download from Adfruit
#include <Adafruit_SSD1306.h> //for this must add lib file download from adfruit
#include <dht11.h> //For DHT11 sensor
#include "RTClib.h" //For real time module

#define SenseMetricSystem //Definition for HC-SR04 sensor
#define waterPump 3 //Definition water pump on Digital pin 3
#define DHT11PIN 4 //Definition for DHT11 sensor on Digital pin 4
#define audioPin  6 //Definition for Buzzer on Digital pin 6
#define pumpLEDPin 8 // Definition for Buzzer on Digital pin 8
#define triPin 13 //Pulse pin
#define echoPin 12 //Echo pin
#define OLED_RESET 4 //Clear LCD

//LDR Definitions
#define LDR_PIN               1   // Here Analog pin 0
#define MAX_ADC_READING       1023  // 10bit adc 2^10 == 1023
#define ADC_REF_VOLTAGE       5.0 // 5 volts
#define REF_RESISTANCE        10030 // 10k resistor 
#define LUX_CALC_SCALAR       12518931 // Formula 
#define LUX_CALC_EXPONENT    -1.405  // exponent first calculated with calculator 

// frequency musical notes
#define NOTE_C6  1047
#define NOTE_C3  131
#define NOTE_G3  196

dht11 DHT11;
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
  
  Serial.begin (9600);

  pinMode(DHT11PIN, INPUT);
  pinMode(echoPin, INPUT);
  pinMode(triPin, OUTPUT);
  pinMode(audioPin, OUTPUT);
  pinMode(waterPump, OUTPUT);
  pinMode(pumpLEDPin, OUTPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    rtc.adjust(DateTime(2019, 07, 22, 19, 39, 0));
  }

}

void loop() {

  long duration, waterLevel;

  int   ldrRawData;
  float resistorVoltage, ldrVoltage;
  float ldrResistance;
  float ldrLux;
  
  DateTime now = rtc.now();
  
  // reads the sensors
  int moistureRaw = analogRead(A0);
  int soilMoisture  = map(moistureRaw, 1030,258,0,100);
  int chk = DHT11.read(DHT11PIN);
  // Perform the analog to digital conversion  
  ldrRawData = analogRead(LDR_PIN);

  // RESISTOR VOLTAGE_CONVERSION
  resistorVoltage = (float)ldrRawData / MAX_ADC_READING * ADC_REF_VOLTAGE;
  
  // voltage across the LDR is the 5V supply minus the 5k resistor voltage
  ldrVoltage = ADC_REF_VOLTAGE - resistorVoltage;
  
  // LDR_RESISTANCE_CONVERSION
  // resistance that the LDR would have for that voltage  
  ldrResistance = ldrVoltage/resistorVoltage * REF_RESISTANCE; 
  
  // LDR_LUX
  ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);
  
  digitalWrite(triPin, LOW);  //PULSE
  delayMicroseconds(2); 
  digitalWrite(triPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(triPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  waterLevel = microsecondsToCentimeters(duration);

    Serial.print(now.day(), DEC); 
    Serial.print('/'); 
    Serial.print(now.month(), DEC); 
    Serial.print('/'); 
    Serial.print(now.year(), DEC);
    Serial.print(" ("); 
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]); 
    Serial.print(") ");
    Serial.print(now.hour(), DEC); 
    Serial.print(':'); 
    Serial.print(now.minute(), DEC); 
    Serial.print(':'); 
    Serial.print(now.second(), DEC);
    Serial.println();

  Serial.print("Soil Moisture(%): "); Serial.println(soilMoisture);
  Serial.print("Water level (cm): "); Serial.println(waterLevel);
  Serial.print("Humidity (%): "); Serial.println((float)DHT11.humidity, 2);
  Serial.print("Temperature (C): "); Serial.println((float)DHT11.temperature, 2);
  Serial.print("LDR Illuminance: "); Serial.print(ldrLux); Serial.println(" lux");
  Serial.println(" ");
  

// if low water level: plays the low level alarm ##CALIBRATE DISTANCE FOR THE WATER TANK##
  if( waterLevel < 50){

      //Display Low Level on lcd
    display.setCursor(30,0);  //OLED Display
    display.setTextSize(1.5);
    display.setTextColor(WHITE);
    display.print("Water Level");
    display.display();
    display.setCursor(40,10);  
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("LOW!");
    display.display();
    display.clearDisplay();
    
    // plays the alarm sound
    for(int i=0;i<2;i++){
      tone(audioPin, NOTE_G3, 200);
      delay(200);
      tone(audioPin, NOTE_C3, 200);
      delay(200);
      noTone(audioPin);
    }
  }

  if( soilMoisture < 50 && waterLevel > 50){
    // turn the pump on
      digitalWrite(waterPump,HIGH);
      digitalWrite(pumpLEDPin,HIGH);

      //Display pump on, watering
      display.setCursor(0,0);  //OLED Display
      display.setTextSize(1.5);
      display.setTextColor(WHITE);
      display.print("PUMP ON");
      display.display();

      display.setCursor(0,10);  //OLED Display
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.print("WATERING!");
      display.display();
  
      display.clearDisplay();
 
      delay(5000);
     // turn the pump off
      digitalWrite(waterPump, LOW);
      digitalWrite(pumpLEDPin,LOW);
  }

 //Display water level on lcd
  display.setCursor(0,0);  //OLED Display
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.print("Water Level: ");
  display.print(waterLevel);
  display.print("cm");

 //Display soilmoisture on lcd
  display.setCursor(0,10);  //OLED Display
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.print("Moisture: ");
  display.print(soilMoisture);
  display.print("%");
  display.display();

  //Display temperature on lcd
  display.setCursor(0,20);  //OLED Display
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.print("temperature: ");
  display.print((float)DHT11.temperature, 1);
  display.print("C");
  display.display();
  
  display.clearDisplay();

 delay(2000);
}

long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2;
}
