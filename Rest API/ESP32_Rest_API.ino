#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>

//Wifi-Credentials  
const char *SSID = "Wifi_Name";
const char *PWD = "Wifi_Password";

//GPIO Pins
int ldrPin = 32;
//DHT11
#include "DHT.h"
#define DHTPIN 4 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// Web server running on port 80
WebServer server(80);

// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];

//env variable
float ldrVa;
float ldrAbs;
float temperature;
float humidity;

//Connection_To_Wifi
void connectToWiFi()
{
  Serial.print("Connecting to ");
  Serial.println(SSID);
  
  WiFi.begin(SSID, PWD);
  
  while (WiFi.status() != WL_CONNECTED)
    {
    Serial.print(".");
    delay(500);
    }
 
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

//Routing
void setup_routing()
{     
  server.on("/ldrVa", getldrValue);          
  server.on("/temp", getTemperature);          
  server.on("/humid", getHumidity);
  server.on("/env", getEnv);     
  
  // start server    
  server.begin();    
}

//Create_JSON
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

//Read_Sensor Data
void read_sensor_data(void * parameter)
{
     for (;;)
     {
     ldrVa = analogRead(ldrPin);
     ldrAbs = ldrVa/4;
     humidity = dht.readHumidity();
     temperature = dht.readTemperature();
     
     if (isnan(humidity) || isnan(temperature)) 
     {
     Serial.println("Failed to read from DHT sensor!");
     }

     Serial.println("Read sensor data");
     // delay the task
     //vTaskDelay(60000 / portTICK_PERIOD_MS);
     Serial.println(ldrAbs);
     Serial.println(temperature);
     Serial.println(humidity);
     delay(3000);
     }
}

//Get LDR_Values
void getldrValue()
{
  Serial.println("Get LDR Value");
  create_json("ldr", ldrAbs, "V");
  server.send(200, "application/json", buffer);
}
//Get Temperature
void getTemperature() {
  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
  server.send(200, "application/json", buffer);
}
//Get Humidity 
void getHumidity() {
  Serial.println("Get humidity");
  create_json("humidity", humidity, "%");
  server.send(200, "application/json", buffer);
}
//Environment
void getEnv() {
  Serial.println("Get env");
  jsonDocument.clear();
  add_json_object("temperature", temperature, "°C");
  add_json_object("humidity", humidity, "%");
  add_json_object("ldr", ldrAbs, "V");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
//Task
void setup_task() {    
  xTaskCreate(     
  read_sensor_data,      
  "Read sensor data",      
  3000,      
  NULL,      
  1,     
  NULL     
  );     
}
//Setup
void setup()
{
pinMode(ldrPin, OUTPUT);
Serial.begin(115200);
Serial.println(F("DHTxx test!"));
dht.begin();
connectToWiFi();     
setup_task();
setup_routing();
}
//Loop
void loop()
{
  server.handleClient();
}
