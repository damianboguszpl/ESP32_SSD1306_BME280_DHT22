
// Provided by Damian Bogusz & Paweł Ciszewski
//


// Choose sensor ... (comment one of the two following lines)
//#define USING_BME280
#define USING_DHT22


// --- used libraries ---------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include <SPI.h>              // display
#include <Wire.h>             // display
#include <Adafruit_GFX.h>     // display
#include <Adafruit_SSD1306.h> // display

#ifdef USING_BME280
  #include <Adafruit_BME280.h>  // BME280 sensor
#elif defined USING_DHT22
  #include "DHT.h"              // DHT22 sensor
#endif

// --- server -----------------------------------------------------------
AsyncWebServer server(80);

// fill the following ssid and password with your network's credentials
const char *ssid = "super"; 
const char *password = "trudnehaslo123";
StaticJsonDocument<250> jsonDocument;
char buffer[250];

// --- sensor -----------------------------------------------------------
#ifdef USING_BME280
  //I2C    BME280
  Adafruit_BME280 bme;
#elif defined USING_DHT22
  #define DHTPIN 4      // pin which DHT22 is connected to
  #define DHTTYPE DHT22 //our sensor is DHT22 type
  DHT dht(DHTPIN, DHTTYPE); //create an instance of DHT sensor
#endif

// --- display ----------------------------------------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- const & variables ------------------------------------------------
float temperature;
float humidity;
float pressure;

int interval = 2000;

String t_string = "Temp: ";
String h_string = "Hum:  ";
String interval_string = "Read interval: " + String(interval/1000) + "s";
String ip_string = "IP: ";

// ----------------------------------------------------------------------

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}

void setup_wifi() {
  // WIFI setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup_server_routing() {
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("GET => /data");
    
    jsonDocument.clear();
    add_json_object("temperature", temperature, "°C");
    add_json_object("humidity", humidity, "%");
    add_json_object("pressure", pressure, "hPa");
    serializeJson(jsonDocument, buffer);
    request->send(200, "application/json", buffer);
  });
  
  server.onNotFound(notFound);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); 
  server.begin();
}

void setup_display() {
  // display setup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.display();
}

void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}

void readSensorData() {
  #ifdef USING_BME280           // for BME280 sensor
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure()/100.0;
    Serial.println("Sensor data read:  " + String(temperature) + "*C" + "   Hum:  " + String(humidity) + "%"+ "   Pressure:  " + String(humidity) + "hPa");
  #elif defined USING_DHT22     // for DHT sensor
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    Serial.println("Sensor data read:  " + String(temperature) + "*C" + "   Hum:  " + String(humidity) + "%");
  #endif
}

void refreshDisplayInfo() {
  Serial.print("Display refreshed  <==  ");
  readSensorData();
  
  display.clearDisplay();
  
  display.setCursor(0,0);
  display.print(String(temperature) +" " + (char)247 +"C  " + String(pressure) + " hPa");
  display.setCursor(0,8);
  display.println("Humidity: " + String(humidity) +" %");
  display.setCursor(0,16);
  display.println("Interval: " + String(interval/1000) + "s");
  display.setCursor(0,24);
  display.println("IP: " + WiFi.localIP().toString());
  display.display();
}

void setup()
{
  Serial.begin(115200);

  setup_display();

  #ifdef USING_BME280
    // BME280 sensor setup
    delay(1000);
    if (!bme.begin()) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
    }
  #elif defined USING_DHT22
      // DHT22 sensor setup
      Serial.println("DHT22 sensor!");
      dht.begin(); //call begin to start sensor
  #endif

  setup_wifi();
  setup_server_routing();
}
void loop()
{
  refreshDisplayInfo();
  delay(interval);
}
