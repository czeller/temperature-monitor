/************************************
  Sensor code credit
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com  

  Wifi/HTTP code credit
  https://randomnerdtutorials.com/esp8266-nodemcu-http-get-post-arduino/#http-post
************************************/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// get sensitive data from secrets file
#include <temperature-monitor-secrets.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

/************************************
  CONFIGURATION
************************************/

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
const char* freezerID = "foo"; // todo

// temperature API URL
const char* apiURL = "https://klafwmnkhnnsvilpdwzyuaviee0ghqnx.lambda-url.us-east-2.on.aws/";

// number of seconds to wait between temperature checks
const int temperatureCheckDelaySeconds = 3;

/************************************
  END CONFIGURATION
************************************/

void setup() {  
  Serial.println("Running setup..."); 
  
  // Start the Serial Monitor
  Serial.begin(115200);

  // Connect to Wifi
  connectWifi();

  // Start the DS18B20 sensor
  Serial.println("Starting DS18B20 sensor...");
  sensors.begin();
}

void loop() {
  float temperature = getFahrenheitFromSensor();
  Serial.print(temperature);
  Serial.println("ºF");

  Serial.println("Checking WiFi status...");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected!");

    // Try to reconnect (todo: test this)
    connectWifi();
  }
  else {
    sendTemperatureToAPI(temperature);
  }

  delay(temperatureCheckDelaySeconds * 1000);
}

float getFahrenheitFromSensor() {
  Serial.println("Requesting temperatures from sensor...");
  sensors.requestTemperatures(); 

  Serial.println("Getting Fahrenheit temperature from sensor...");  
  return sensors.getTempFByIndex(0);  
}

void sendTemperatureToAPI(float temperature) {
  BearSSL::WiFiClientSecure client;
  client.setInsecure(); /* we don't care about SSL certificates :) */

  HTTPClient http;
  
  String requestURL = apiURL + "?temperature=&freezerID=" + freezerID;
  requestURL += String(temperature);
    
  Serial.println("Begin HTTP...");      
  http.begin(client, requestURL.c_str());
  
  Serial.println("Send HTTP GET...");
  Serial.println(requestURL);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("HTTP failed. Error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void connectWifi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}
