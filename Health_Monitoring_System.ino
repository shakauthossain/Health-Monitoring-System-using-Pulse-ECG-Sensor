#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "MAX30105.h"           //https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library
#include "heartRate.h"

#define OLED_SDA 21
#define OLED_SCL 22
Adafruit_SH1106 display(21, 22);

int LO_pos_pin = 15;
int LO_neg_pin = 13;
int Output_pin = 14;

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.print("Heartbeat");
  display.setCursor(20, 40);
  display.print("Monitor");
  display.display();
  delay(2000);
  display.clearDisplay();
  Serial.println("Sensor Initializing...");


  //Use default I2C port, 400kHz speed
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    //    display.clearDisplay();
    //    display.setTextSize(2);
    //    display.setTextColor(WHITE);
    //    display.setCursor(30, 0);
    //    display.print("Sensor");
    //    display.setCursor(30, 20);
    //    display.print("is not");
    //    display.setCursor(10, 40);
    //    display.print("Connected.");
    //    display.display();
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED


  //ECG
  pinMode(LO_pos_pin, INPUT); // Setup for leads off detection LO +
  pinMode(LO_neg_pin, INPUT); // Setup for leads off detection LO -
  pinMode(Output_pin, INPUT); // Setup for output pin.
}

void ECG(){
    if ((digitalRead(LO_pos_pin) == 1) || (digitalRead(LO_neg_pin) == 1)) {
    Serial.println('!');
  }
  else {
    Serial.println(analogRead(Output_pin));
  }
  //Wait for a bit to keep serial data from saturating
  delay(2000);
  }

void loop() {

  ECG();
  Blynk.run();
  timer.run();
  
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable
      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  display.print("Avg BPM");
  display.setCursor(60, 30);
  display.print(beatAvg);
  display.display();


  if (irValue < 50000) {
    Serial.print(" No finger?");
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(50, 10);
        display.print("No");
        display.setCursor(30, 30);
        display.print("Finger");
        display.display();
  }
    else {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(30, 10);
      display.print("Avg BPM");
      display.setCursor(60, 30);
      display.print(beatAvg);
      display.display();
    }
  Serial.println();
}
