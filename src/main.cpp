#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Arduino.h>
#include <Thing.h>
#include <WebThingAdapter.h>
#include <user_config.h>

#define DHTPIN            2         // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);

const char* deviceTypes[] = {"MultiLevelSensor", "Sensor", nullptr};
ThingDevice sensor("DHT22", "DHT22 Temperature & Humidity sensor", deviceTypes);
ThingProperty tempSensorProperty("temperature", "Temperature", NUMBER, "TemperatureProperty");
ThingProperty humiditySensorProperty("humidity", "Humidity", NUMBER, "LevelProperty");
WebThingAdapter* adapter = NULL;

uint32_t delayMS;
int cnt=10;

// Initialize device.
void setup() {
  Serial.begin(115200);
  // Let's connect to the WiFi
  Serial.println("");
  Serial.print("Connecting to \"");
  Serial.print(STA_SSID1);
  Serial.println("\"");
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(STA_SSID1, STA_PASS1);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(STA_SSID1);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  adapter = new WebThingAdapter("dht22", WiFi.localIP());
  sensor.addProperty(&tempSensorProperty);
  sensor.addProperty(&humiditySensorProperty);
  adapter->addDevice(&sensor);
  adapter->begin();
  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(sensor.id);

  //Let's initialize the DHT Sensor

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
}

void loop() {
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Serial.println(" *C");
    ThingPropertyValue tempValue;
    tempValue.number = event.temperature;
    tempSensorProperty.setValue(tempValue);
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
    ThingPropertyValue humidityValue;
    humidityValue.number = event.relative_humidity;
    humiditySensorProperty.setValue(humidityValue);
  }

  adapter->update();

  if(cnt==0){
    Serial.println("Reset..");
    ESP.restart();
  }

  cnt--;
  delay(1000);
}