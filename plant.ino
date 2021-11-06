#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define SensorPin 36  //analog pin where the moisture sensor is connected
#define uS_TO_S_FACTOR 1000000ULL   /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3600        /* Time ESP32 will go to sleep (in seconds) 1h */

//wifi constants
const char *SSID = "wifiname";
const char *PWD = "wifipassword";

//mqtt constants
const char* mqttServer = "192.168.x.xxx"; //your MQTT server here
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtttopic = "plant/plant1"; //mqtt topic where the data will be received.

void setup() { 
  Serial.begin(9600); //for troubleshooting
  initWiFi();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop() {
  float sensorValue = analogRead(SensorPin); //read the sensor value
 
  Serial.println(sensorValue);
  delay(1000);
   //check WiFi connection and reconnect if needed
    if (WiFi.status() != WL_CONNECTED) { 
        //Serial.println("Reconnecting to WiFi...");
        WiFi.reconnect();
    }
    while (WiFi.status() != WL_CONNECTED) {
      //Serial.print('.');
      delay(1000);
    }

    //sending mqtt message
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject& JSONencoder = JSONbuffer.createObject();
    initMQTT();
    JSONencoder["sensordata"] = sensorValue;
    char JSONmessageBuffer[100];
    JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    //Serial.println("Sending message to MQTT topic..");
    //Serial.println(JSONmessageBuffer);
    if (client.publish(mqtttopic, JSONmessageBuffer) == true) {
      //Serial.println("Success sending message");
    } else {
      //Serial.println("Error sending message");
    }
 
    client.loop();
    //Serial.println("-------------");
    esp_deep_sleep_start();
} 

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  client.setServer(mqttServer, 1883);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client")) {
 
      Serial.println("connected");
 
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
    }
  }
}
