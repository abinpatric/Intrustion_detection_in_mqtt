#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

#define wifi_ssid "Realme"
#define wifi_password "12334455"
#define mqtt_server "192.168.61.214"

#define intrusion_topic "sensor/DHT11/intrusion"

WiFiClient espClient;
PubSubClient client(espClient);

float previousTemp;

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);

 
  previousTemp = dht.readTemperature();
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying to connect");
      delay(3000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("failed to read values!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");

  // Publish temperature to MQTT topic
  client.publish("sensor/DHT11/temperature", String(t).c_str(), true);

  if (abs(t - previousTemp) > 1) {
    Serial.println("Intrusion Detected! Data Manipulation");
    client.publish(intrusion_topic, "Intrusion Detected! Data Manipulation", true);
  }

  previousTemp = t;

delay(1000);
}
