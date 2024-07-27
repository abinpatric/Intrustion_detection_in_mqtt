#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Realme";
const char* password = "12334455";
const char* mqtt_server = "192.168.61.214";
const int mqtt_port = 1883;
const char* mqtt_username = "idsproject";
const char* mqtt_password = "Idsproject123";

WiFiClient espClient;
PubSubClient client(espClient);

bool sentInitialData = false;
bool initialMessageReceived = false;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  if (strcmp(topic, "confidential/topic") == 0) {
    String message = String((char*)payload).substring(0, length);
    int num = message.toInt();
    if (num >= 1 && num <= 50) {
      Serial.println("Intrusion detected in confidential topic with number: " + message);
      String intrusionMessage = "Intrusion detected in confidential topic with number: " + message;
      char intrusionMsg[intrusionMessage.length() + 1];
      intrusionMessage.toCharArray(intrusionMsg,intrusionMessage.length() + 1);
      client.publish("intrusion/topic",intrusionMsg);
      //client.publish("intrusion/topic", "Intrusion detected in confidential topic with number: " + message);
    }
  } else if (strcmp(topic, "intrusion/topic") == 0) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);

    if (doc.containsKey("key")) {
      initialMessageReceived=false;
      Serial.println("No Intrusion detected!.");
            
    }
    else{
      if(initialMessageReceived){
        Serial.println("Intrusion..");
        }
      else{
          initialMessageReceived=true;
          }
      //Serial.println("No intrusion");
      }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("confidential/topic");
      client.subscribe("intrusion/topic");

      // Send initial random number to the confidential topic
      int randomValue = random(1, 51); // Generate random number between 1 and 50
      client.publish("confidential/topic", String(randomValue).c_str());
      sentInitialData = true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  dht.begin();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (sentInitialData) {
    // Read and publish sensor data to intrusion topic
    StaticJsonDocument<256> doc;
    doc["humidity"] = dht.readHumidity();
    doc["temperature"] = dht.readTemperature();
    doc["key"] = "aab879fer5def34sj2adcek34";

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    client.publish("intrusion/topic", jsonBuffer);

    delay(5000);
}
}
