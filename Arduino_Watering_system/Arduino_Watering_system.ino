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

   RTC(Real Time Clock <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
    SDA  <-> A4
    SCL  <-> A5

   Relay <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
     IN  <-> D3

   LCD (128X64 .96') <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
    SDA  <-> SDA
    SCL  <-> SCL

   BUZZER <-> Arduino:
    + <-> D6
    - <-> GND

  LM-393 DRIVER (moisture sensor) <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
    OUT  <-> A0

    DHT11 (Temperature and Humidity) <-> Arduino:
    GND  <-> GND
    +5 V <-> +5 V
    OUT  <-> D4

  LDR (Light Mesure) <-> Arduino:
    + <-> A1
    - <-> GND

  HC-SR04 (Ultrason) <-> Arduino:
     GND     <-> GND
    +5 V     <-> +5 V
    EchoPin  <-> D8
    TrigPin  <-> D9


 SD card Module - Arduino UNO
    MOSI  <-> Pin 11
    MISO  <-> Pin 12
    CLK   <-> Pin 13
    CS    <-> Pin 10
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
#define triPin 9 //Pulse pin
#define echoPin 8 //Echo pin
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

//Inicial Image
const unsigned char plantImage [] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 
  0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x98, 0x00, 0x00, 0x00, 0x01, 0xe8, 0x00, 0x00, 0x00, 0x01, 
  0xf8, 0x00, 0x00, 0x00, 0x01, 0xf9, 0xc0, 0x00, 0x00, 0x01, 0xeb, 0xc0, 0x00, 0x00, 0x00, 0x1f, 
  0xc0, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x0b, 0x80, 0x00, 0x00, 0x00, 0x0c, 0x00, 
  0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 
  0x00, 0x01, 0x00, 0x40, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 
  0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 
  0xff, 0x80, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


dht11 DHT11;
RTC_DS1307 rtc;
Adafruit_SSD1306 display(OLED_RESET);


void setup() {

  Serial.begin (9600);

  pinMode(DHT11PIN, INPUT);
  pinMode(echoPin, INPUT);
  pinMode(triPin, OUTPUT);
  pinMode(audioPin, OUTPUT);
  pinMode(waterPump, OUTPUT);


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
    rtc.adjust(DateTime(2019, 8, 6, 17, 59 , 0));
  }

  
   // Display initial messages
  display.drawBitmap(20, 7, plantImage, 40, 27, WHITE); // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.drawBitmap(45, 7, plantImage, 40, 27, WHITE); // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.drawBitmap(65, 7, plantImage, 40, 27, WHITE); // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.setCursor(40, 0); 
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.print("Welcome!");
  display.display();

// plays the alarm sound
    for (int i = 0; i < 2; i++) {
      tone(audioPin, NOTE_C6, 200);
      delay(200);
      tone(audioPin, NOTE_C6, 200);
      delay(200);
      noTone(audioPin);
   }
  delay(8000);
  display.clearDisplay();
 
}

void loop() {

  long duration, waterLevel;

  int   ldrRawData;
  float resistorVoltage, ldrVoltage;
  float ldrResistance;
  float ldrLux;

  //Date and time definition


  
  DateTime now = rtc.now();
  char dateBuffer[12];;
  char timeBuffer[12];

  sprintf(dateBuffer,"%02u/%02u/%04u ",now.day(),now.month(),now.year());
  sprintf(timeBuffer,"%02u:%02u",now.hour(),now.minute());
  


  // reads the sensors
  int moistureRaw = analogRead(A0);
  int soilMoisture  = map(moistureRaw, 1023, 292, 0, 100);
  int chk = DHT11.read(DHT11PIN);
  // Perform the analog to digital conversion
  ldrRawData = analogRead(LDR_PIN);

  // RESISTOR VOLTAGE_CONVERSION
  resistorVoltage = (float)ldrRawData / MAX_ADC_READING * ADC_REF_VOLTAGE;

  // voltage across the LDR is the 5V supply minus the 5k resistor voltage
  ldrVoltage = ADC_REF_VOLTAGE - resistorVoltage;

  // LDR_RESISTANCE_CONVERSION
  // resistance that the LDR would have for that voltage
  ldrResistance = ldrVoltage / resistorVoltage * REF_RESISTANCE;

  // LDR_LUX
  ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);

  //HC-SR04 Readings
  digitalWrite(triPin, LOW);  //PULSE
  delayMicroseconds(2);
  digitalWrite(triPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  waterLevel = constrain (map(microsecondsToCentimeters(duration), 27, 3, 0, 100), 0, 100); //Covert water level to porcetage (set 26cm máx dist and 3 as min dist) in my case
  

  Serial.print(dateBuffer);
  Serial.print(" - ");
  Serial.print(timeBuffer);

  Serial.println();

  Serial.print("Soil Moisture(%): "); Serial.println(soilMoisture);
  Serial.print("Water level (%): "); Serial.println(waterLevel);
  Serial.print("Humidity (%): "); Serial.println((float)DHT11.humidity, 2);
  Serial.print("Temperature (C): "); Serial.println((float)DHT11.temperature, 2);
  Serial.print("LDR Illuminance: "); Serial.print(ldrLux); Serial.println(" lux");
  Serial.println(" ");

//Display water level on lcd
  display.setCursor(0, 0); 
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.print("Water Level: ");
  display.print(waterLevel);
  display.print("%");

  //Display time stamp on lcd
  display.setCursor(0, 8);
  display.setTextSize(0.3);
  display.setTextColor(WHITE);
  display.print(timeBuffer);
  display.print("  ");
  //Display date stamp on lcd
  display.print(dateBuffer);

  //Display soilmoisture on lcd
  display.setCursor(0, 16); 
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("Plant moisture: ");
  display.print(soilMoisture);
  display.print("%");
  
  //Display temperature on lcd
  display.setCursor(0, 24); 
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("Temp: ");
  display.print((float)DHT11.temperature, 0);
  display.drawCircle(49, 25, 1, WHITE);//xcenter, ycenter, radius, WHITE
  display.print(" C  ");

  //Display Relative humidity on lcd
  display.print("RH: ");
  display.print((float)DHT11.humidity, 0);
  display.print("%");
  display.display();

  //Clear lcd
  display.clearDisplay();
  delay(2000);

  // if low water level: plays the low level alarm ##CALIBRATE DISTANCE FOR THE WATER TANK##
  if ( waterLevel <= 5) {

    //Display Low Level on lcd
    display.setCursor(30, 0); //OLED Display
    display.setTextSize(1.5);
    display.setTextColor(WHITE);
    display.print("Water Level");
    display.display();
    display.setCursor(40, 10);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("LOW!");
    display.display();
    display.clearDisplay();


    // plays the alarm sound
    for (int i = 0; i < 2; i++) {
      tone(audioPin, NOTE_G3, 200);
      delay(200);
      tone(audioPin, NOTE_C3, 200);
      delay(200);
      noTone(audioPin);
    }
  }


 
  if ( soilMoisture < 50 && waterLevel > 5) {
 
    //Display pump on, watering
    display.setCursor(0, 0); //OLED Display
    display.setTextSize(1.5);
    display.setTextColor(WHITE);
    display.print("PUMP ON");
    display.display();

    display.setCursor(0, 10); //OLED Display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("WATERING!");
    display.display();
    
     // turn the pump on
    digitalWrite(waterPump, HIGH);
    
    delay(5000);
    
    // turn the pump off
    digitalWrite(waterPump, LOW);
    display.clearDisplay();

  }

  delay(2000);
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}
String fixZero(int i){
  String ret = String(i);
  if (i < 10){ 
    ret += "0";
    ret += i;
  }
  return ret;
}
