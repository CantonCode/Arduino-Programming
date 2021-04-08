#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

const float accelerationThreshold = 2.5; // threshold of significant in G's
const int numSamples = 119;

int samplesRead = numSamples;
BLEService batteryService("1101");
BLEUnsignedCharCharacteristic gXChar("2101", BLERead | BLENotify);
BLEUnsignedCharCharacteristic gYChar("2102", BLERead | BLENotify);
BLEUnsignedCharCharacteristic gZChar("2103", BLERead | BLENotify);

void setup() {
Serial.begin(9600);
while (!Serial);

pinMode(LED_BUILTIN, OUTPUT);
if (!BLE.begin()) 
{
Serial.println("starting BLE failed!");
while (1);
}
 if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

BLE.setLocalName("BatteryMonitor");
BLE.setAdvertisedService(batteryService);
batteryService.addCharacteristic(gXChar);
batteryService.addCharacteristic(gYChar);
batteryService.addCharacteristic(gZChar);
BLE.addService(batteryService);

gXChar.setValue(0);
gYChar.setValue(0);
gZChar.setValue(0);





BLE.advertise();
Serial.println("Bluetooth device active, waiting for connections...");

}

void loop() 
{
BLEDevice central = BLE.central();

if (central) 
{
Serial.print("Connected to central: ");
Serial.println(central.address());
digitalWrite(LED_BUILTIN, HIGH);


while (central.connected()) {

 float aX, aY, aZ, gX, gY, gZ;

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
      Serial.print(aX, 3);
      Serial.print(',');
      Serial.print(aY, 3);
      Serial.print(',');
      Serial.print(aZ, 3);
      Serial.print(',');
      Serial.print(gX, 3);
      gXChar.writeValue(gX);
      Serial.print(',');
      Serial.print(gY, 3);
      gYChar.writeValue(gY);
      Serial.print(',');
      Serial.print(gZ, 3);
      gZChar.writeValue(gZ);
      Serial.println();

      if (samplesRead == numSamples) {
        // add an empty line if it's the last sample
        Serial.println();
      }
    }

}
}
}
digitalWrite(LED_BUILTIN, LOW);
Serial.print("Disconnected from central: ");
Serial.println(central.address());
}
