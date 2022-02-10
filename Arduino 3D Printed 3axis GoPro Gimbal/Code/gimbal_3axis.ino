/* Program: 3-axis camera stabilizer (GIMBAL)
  * Author: Ricardo Silva Moreira e-mail: silvaricardo1997@gmail.com
  * Copyright© 2018 Ricardo, Brasília, BR
  *
  * Version: alpha
  * Last modified: 09/17/2018
  * Brief description:
       This program reads the values of Rotation and Acceleration x-y-z from the MPU6050 and applies the corrections in the opposite direction to the servos */


// ------------------------------------------------------------------------------------------

// ==========================================================
// ===             PHYSICAL CONNECTIONS				            ===
// ==========================================================
/*
##SERVOS##
	rollServo -> DigitalPin 10
		VCC -> +5V
		GND -> GND
		
	yawServo -> DigitalPin 9
		VCC -> +5V
		GND -> GND
		
	pitchServo -> DigitalPin 8
		VCC -> +5V 
		GND -> GND

##MPU6050##
	SCL -> AnalogPin 4
	SDA -> AnalogPin 5
	VCC -> +5V
	GND -> GND

*/


#include <Servo.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

Servo rollServo;  // create an object of the type used to control the servo roll; 
Servo pitchServo;  // create an object of the type used to control the servo pitch; 
Servo yawServo;  // create an object of the type used to control the servo yaw;



MPU6050 mpu;

#define OUTPUT_READABLE_YAWPITCHROLL

#define LED_PIN 13 
bool blinkState = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // keep current MPU interrupt status byte
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all current bytes in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

//orientation / movement variables
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            Acceleration sensor measurements
VectorInt16 aaReal;     // [x, y, z]            Gravity-free acceleration sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            Global frame acceleration sensor measurements
VectorFloat gravity;    // [x, y, z]            Gravity vector
float euler[3];         // [psi, theta, phi]    Container for Euler angle
float ypr[3];           // [yaw, pitch, roll]   yaw / pitch / roll container and gravity vector



// ================================================================
// ===             INTERRUPTION DETECTION ROUTINE               ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether the MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}

byte rollStart = 90;  //rollServoervo init position
byte pitchStart = 90; //pitchServoervo init position
byte yawStart  = 90;  //yawServo init position

// ================================================================
// ===                   INITIAL  SETUP                         ===
// ================================================================

unsigned long time;
unsigned long prevUpdate;

void setup() {
    Wire.begin();
    TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)

    Serial.begin(9600);
    
    mpu.initialize();

    // load abd config DMP
    Serial.println(F("initiating DMP..."));
    devStatus = mpu.dmpInitialize();

    // provide your own gyro offsets here, scaled to minimum sensitivity
    mpu.setXGyroOffset(260);  
    mpu.setYGyroOffset(76);   
    mpu.setZGyroOffset(-85);   
    mpu.setZAccelOffset(1788 ); // 1788 factory default for my test chip
    
    // check if it worked (returns 0 if so)
    if (devStatus == 0) {
        // Turn on the MPU6050, since here it is ready
        Serial.println(F("Activating MPU..."));
        mpu.setDMPEnabled(true);

        // turn on Arduino interrupt detection
        Serial.println(F("Enabling Interrupt Detection (Arduino external interrupt)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set the DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("MPU Ready! Wainting interuption ..."));
        dmpReady = true;

        // get the expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
    }

    rollServo.attach(10); 	// rollServo connected to Digital 10 port on arduino NANO
    rollServo.write(rollStart);

    yawServo.attach(9);		//yawServo connected to Digital 9 port on arduino NANO
    yawServo.write(yawStart);
    
    pitchServo.attach(8);	//pitchServo connected to Digital 9 port on arduino NANO
    pitchServo.write(pitchStart);
    
    prevUpdate = millis();
    
}

// ================================================================
// ===                   PROGRAM MAIN LOOP                      ===
// ================================================================

float iRoll; // Store the angle applied to the MPU on the x axis (roll)
float iPitch; // Store the angle applied to the MPU on the Y axis (pitch)
float iYaw;	// Store the angle applied to the MPU on the z axis (yaw)

void loop() {
    // if programming failed, try again!
    if (!dmpReady) return;

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

   // get the current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless the code is very inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so it can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));

    // otherwise check for DMP data interruption (this should happen often)
    } else if (mpuIntStatus & 0x02) {
        // wait for the correct length of available data, must be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from the FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here if >1 packet available
        fifoCount -= packetSize;

       // display Euler angles in degrees
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        
        iRoll = ypr[2] * 180/M_PI;
        iPitch = ypr[1] * 180/M_PI;
        iYaw = ypr[0]* 180/M_PI;
       
                
        time = millis();
        
        if (time - prevUpdate > 10) {
          rollServo.write(rollStart - iRoll);
          pitchServo.write(pitchStart - iPitch);
          yawServo.write(yawStart - iYaw);
          prevUpdate = time;
        }  
        
    }
}
