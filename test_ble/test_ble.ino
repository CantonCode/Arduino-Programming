
#include <ArduinoBLE.h>

//----------------------------------------------------------------------------------------------------------------------
// BLE UUIDs
//----------------------------------------------------------------------------------------------------------------------

#define BLE_UUID_SENSOR_DATA_SERVICE              "2BEEF31A-B10D-271C-C9EA-35D865C1F48A"
#define BLE_UUID_MULTI_SENSOR_DATA                "4664E7A1-5A13-BFFF-4636-7D0A4B16496C"

#define NUMBER_OF_SENSORS 4
#define SAMPLES 5

union multi_sensor_data
{
  struct __attribute__( ( packed ) )
  {
    float values[SAMPLES][NUMBER_OF_SENSORS];
  };
  uint8_t bytes[SAMPLES][ NUMBER_OF_SENSORS * sizeof( float ) ];
};

union multi_sensor_data multiSensorData;


//----------------------------------------------------------------------------------------------------------------------
// BLE
//----------------------------------------------------------------------------------------------------------------------

BLEService sensorDataService( BLE_UUID_SENSOR_DATA_SERVICE );
BLECharacteristic multiSensorDataCharacteristic( BLE_UUID_MULTI_SENSOR_DATA, BLERead | BLENotify, sizeof multiSensorData.bytes );


const int BLE_LED_PIN = LED_BUILTIN;
const int RSSI_LED_PIN = LED_PWR;


void setup()
{
  Serial.begin( 9600 );
  //while ( !Serial );

  pinMode( BLE_LED_PIN, OUTPUT );
  pinMode( RSSI_LED_PIN, OUTPUT );

  if ( setupBleMode() )
  {
    digitalWrite( BLE_LED_PIN, HIGH );
  }

  for ( int j = 0; j < SAMPLES; j++ )
    for ( int i = 0; i < NUMBER_OF_SENSORS; i++ )
    {
      multiSensorData.values[j][i] = i;
    }
}


void loop()
{
#define UPDATE_INTERVALL 50
  static long previousMillis = 0;

  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if ( central )
  {
    Serial.print( "Connected to central: " );
    Serial.println( central.address() );

    while ( central.connected() )
    {
      unsigned long currentMillis = millis();
      if ( currentMillis - previousMillis > UPDATE_INTERVALL )
      {
        previousMillis = currentMillis;

        Serial.print( "Central RSSI: " );
        Serial.println( central.rssi() );

        if ( central.rssi() != 0 )
        {
          digitalWrite( RSSI_LED_PIN, LOW );

          for ( int j = 0; j < SAMPLES; j++ )
            for ( int i = 0; i < NUMBER_OF_SENSORS; i++ )
            {
              multiSensorData.values[j][i] = multiSensorData.values[j][i] + 0.1;
            }

          multiSensorDataCharacteristic.writeValue( multiSensorData.bytes, sizeof multiSensorData.bytes );
        }
        else
        {
          digitalWrite( RSSI_LED_PIN, HIGH );
        }
      }
    }

    Serial.print( F( "Disconnected from central: " ) );
    Serial.println( central.address() );
  }
}



bool setupBleMode()
{
  if ( !BLE.begin() )
  {
    return false;
  }

  // set advertised local name and service UUID:
  BLE.setDeviceName( "Arduino Nano 33 BLE" );
  BLE.setLocalName( "Arduino Nano 33 BLE" );
  BLE.setAdvertisedService( sensorDataService );

  // BLE add characteristics
  sensorDataService.addCharacteristic( multiSensorDataCharacteristic );

  // add service
  BLE.addService( sensorDataService );

  // set the initial value for the characeristic:
  multiSensorDataCharacteristic.writeValue( multiSensorData.bytes, sizeof multiSensorData.bytes );

  // start advertising
  BLE.advertise();

  return true;
}
