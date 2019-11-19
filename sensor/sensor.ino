#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "DHT.h"

#define DHTTYPE DHT22

#define SENSOR_NAME "your-sensor-name"
#define TOPIC "your-topic-name"
#define MQTT_KEEPALIVE 10
#define MQTT_SOCKET_TIMEOUT 10
#define SLEEP_TIME_MS 30000

// change from 0 to 1 if you want to enable DEBUG_MODE
#define DEBUG_MODE 0

const char* ssid = "your-wifi-name";
const char* wifiPassword = "your-super-secure-wifi-password";
const char* mqttServer = "your-super-secure-mqtt-server";
const char* user = "your-user";
const char* password = "your-unbruteforceable-password"; 
const int port = 1883;

uint8_t DHTPin = D2; 
DHT dht(DHTPin, DHTTYPE);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// using IP to save DNS resolution time
IPAddress timeServer(129, 6, 15, 28);
byte packetBuffer[ NTP_PACKET_SIZE];

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP);

bool sent;

char message[100];
char payload[200];

float temperature;
float humidity;
float heatIndex;

unsigned long lastTimestamp;
unsigned long currentTimestamp;

void printConfig() {
  Serial.println("================== Configuration ==================");
  snprintf(message, sizeof(message), "Sensor name: '%s'", SENSOR_NAME);
  Serial.println(message);

  snprintf(message, sizeof(message), "MQTT Server: %s:%d'", mqttServer, port);
  Serial.println(message);
  
  snprintf(message, sizeof(message), "Posting to Topic: '%s'", TOPIC);
  Serial.println(message);
  Serial.println("===================================================");
}

void connectToWifi() {
  snprintf (message, sizeof(message), "Connecting to '%s'", ssid);
  Serial.println(message);

  WiFi.begin(ssid, wifiPassword);
  do{ 
    delay(500);
    Serial.print("Attempting to connect ..\n");
  } while (WiFi.status() != WL_CONNECTED);

  snprintf (message, sizeof(message), "Connected to '%s' with IP %s\n", ssid, WiFi.localIP().toString().c_str());
  Serial.println(message);
}

bool isConnectedToWifi(){
  return WiFi.status() == WL_CONNECTED;
}

void setup() {
  Serial.begin(115200);
  delay(100);

#if DEBUG_MODE
  printConfig();
#endif

  client.setServer(mqttServer, port);

  pinMode(DHTPin, INPUT);
  dht.begin();  
  timeClient.begin();
}

bool isConnectedToMqttServer() {
  return client.connected();
}

void connectToMqttServer(){
  if (client.connect(SENSOR_NAME, user, password)){
    Serial.println("Connected to MQTT Broker!");
  } else {
    Serial.println("Could not connected to MQTT Broker!");
  }
}

void loop() {
  if (!isConnectedToWifi()) {
    connectToWifi();
  }

  if (!isConnectedToMqttServer()) {
    connectToMqttServer();
  }
  
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  timeClient.update(); 
  currentTimestamp = timeClient.getEpochTime();

  // sometimes, 'currentTime' becomes a very low value such as 3 or 4.
  // when that happens, we want to reuse the last reading + the sleep time.
  // since I don't need super precise simes, a few seconds of inprecison is acceptable.
  if (currentTimestamp <= lastTimestamp) {
    currentTimestamp = lastTimestamp + SLEEP_TIME_MS;
  }

  snprintf (payload, sizeof(payload), "{\"timestamp\":%d,\"temperature\":%.2f,\"humidity\":%.2f,\"heatIndex\":%.2f}", 
            currentTimestamp, temperature, humidity, heatIndex);

  sent = false;
  while (!sent){
    Serial.println("Publish message ...");
    Serial.println(payload);
    if (client.publish(TOPIC, payload)){
      Serial.println("Published message");
      sent = true;
    } else {
      Serial.println("Publish failed");
      if (!isConnectedToMqttServer()) {
        Serial.println("Disconnected from MQTT server. Let's reconnect");
        connectToMqttServer();
      } else {
        Serial.println("We're connected to MQTT server. There's another issue");
      }
      delay(1000);
    }
  }
  client.disconnect();

  lastTimestamp = currentTimestamp;
  // ESP.deepSleep expects time in microseconds
  ESP.deepSleep(SLEEP_TIME_MS * 1000); 
}
