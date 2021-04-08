/*
  IMU Capture
  This example uses the on-board IMU to start reading acceleration and gyroscope
  data from on-board IMU and prints it to the Serial Monitor for one second
  when the significant motion is detected.
  You can also use the Serial Plotter to graph the data.
  The circuit:
  - Arduino Nano 33 BLE or Arduino Nano 33 BLE Sense board.
  Created by Don Coleman, Sandeep Mistry
  Modified by Dominic Pajak, Sandeep Mistry
  This example code is in the public domain.
*/

#include <Arduino_LSM9DS1.h>
#include <SD.h>                                      // used for SD Card storage
#include <SPI.h>   


 // imports Serial Peripheral Interface Bus (SPI)library

const float accelerationThreshold = 4.0; // threshold of significant in G's
const int numSamples = 119;
const int ledPin = 22;
const int ledPin2 = 23;
const int ledPin3 = 24;

int samplesRead = numSamples;
File dataFile;                                       // file I am trying to write?

/* Pin for SD Card reader */
const int chipSelect = 10;
String allD = "";


void setup() {

  Serial.begin(9600);

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // print the header
  Serial.println("aX,aY,aZ,gX,gY,gZ");

  File dataFile = SD.open("accel.txt", FILE_WRITE);
  if (dataFile) {
    Serial.println("FILE AVAILABLE");
    dataFile.close();
    
  }
    else {
    Serial.println("error opening datalog.txt");
    dataFile.println("ERROR");
    dataFile.close();
  }



  
}

void loop() {
  float aX, aY, aZ, gX, gY, gZ;
  String allData = "";

       
    digitalWrite(ledPin, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);

  // wait for significant motion
  while (samplesRead == numSamples) {

    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(aX, aY, aZ);

      // sum up the absolutes
      float aSum = fabs(aX) + fabs(aY) + fabs(aZ);

      // check if it's above the threshold
      if (aSum >= accelerationThreshold) {
        // reset the sample read count
        samplesRead = 0;
        break;
      }
    }
  }

  // check if the all the required samples have been read since
  // the last time the significant motion was detected
  while (samplesRead < numSamples) {
    // check if both new acceleration and gyroscope data is
    // available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {


      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      samplesRead++;

      // print the data in CSV format

      String dataString = String(aX) + "," + String(aY) + "," + String(aZ)+ "," + String(gX) + "," + String(gY) + "," + String(gZ) + "\n" ;
  
      allData += dataString;

     

      

       
      if (samplesRead == numSamples) {
         

        
        // add an empty line if it's the last sample
        dataFile = SD.open("accel.txt", FILE_WRITE);

        if (dataFile) {
          dataFile.println(allData);
          dataFile.close();
                 
    digitalWrite(ledPin, LOW);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, HIGH);

          Serial.println("READ TO CARD");
        }
          else {
    Serial.println("error opening datalog.txt");
  }

        
        Serial.println();
      }
    }
  }
}
