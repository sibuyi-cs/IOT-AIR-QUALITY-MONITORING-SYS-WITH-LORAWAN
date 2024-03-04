#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TTN_esp32.h>
#include <TTN_CayenneLPP.h>

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_BME280 bme;

const char* devEui = "70B3D57ED8002623";
const char* appEui = "0000000000000000";
const char* appKey = "21C85D2B570030DD3D5F05BB1B770706";

TTN_esp32 ttn;
TTN_CayenneLPP lpp;

String line = "";


float smokeValue = 0.0;
float gasValue = 0.0;


void message(const uint8_t* payload, size_t size, uint8_t port, int rssi) {
  Serial.println("-- MESSAGE");
  Serial.printf("Received %d bytes on port %d (RSSI=%ddB) :", size, port, rssi);
  for (int i = 0; i < size; i++) {
    Serial.printf(" %02X", payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!bme.begin(0x76, &Wire)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("TTGO LoRa32 BME280 Sensor Example");

  ttn.begin();
  ttn.onMessage(message);  
  ttn.join(devEui, appEui, appKey);
  Serial.print("Joining TTN ");
  while (!ttn.isJoined()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\njoined !");
  ttn.showStatus();
}

void loop() {

  while(Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n'); 
    int commaIndex = receivedData.indexOf(',');
    
    if (commaIndex != -1) {
      String value1_str = receivedData.substring(0, commaIndex);
      String value2_str = receivedData.substring(commaIndex + 1);

      smokeValue = value1_str.toFloat();
      gasValue = value2_str.toFloat();
    }
  }

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  // Send BME280 data over LoRa
  lpp.reset();
  lpp.addTemperature(1, temperature);
  lpp.addRelativeHumidity(2, humidity);
  lpp.addBarometricPressure(3, pressure);
  lpp.addAnalogInput(1,gasValue);
  lpp.addAnalogInput(2,smokeValue);

  if (ttn.sendBytes(lpp.getBuffer(), lpp.getSize())) {
      Serial.println(" ");    
      Serial.println(" Sent Data To TTN");
      Serial.print("temperature: ");
      Serial.println(temperature);
      Serial.print("humidity: ");
      Serial.println(humidity);
      Serial.print("pressure: ");
      Serial.println(pressure);
      Serial.print("gasValue: ");
      Serial.println(gasValue);
      Serial.print("smokeValue: ");
      Serial.println(smokeValue);
             
  }

  delay(10000); 
}
