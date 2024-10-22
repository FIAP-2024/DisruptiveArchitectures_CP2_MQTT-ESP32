

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

const int DHT_PIN = 15;

DHTesp dhtSensor;

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "rm99466-mqttserver.eastus.cloudapp.azure.com";
int port = 1883;
String stMac;
char mac[50];
char clientId[50];

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE (50)
float temp = 0;
float umi = 0;
int value = 0;


void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.println("Conectando ao WiFi: ");
  Serial.println(ssid);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Falha ao conectar!");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  stMac = WiFi.macAddress();
  stMac.replace(":", "_");
  Serial.println(stMac);

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Mensagem enviada [");
  Serial.println(topic);
  Serial.println("]");
  Serial.println();

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();

  if ((char)payload[0] == '1') {
    digitalWrite(DHT_PIN, LOW);
  }
  else {
    digitalWrite(DHT_PIN, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentando a conexão com o MQTT");

    String clientId = "ESP32CP2RM99466Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado com sucesso!");
      client.publish("iotcp2rm99466/mqtt", "iotcp2rm99466");
      client.subscribe("iotcp2rm99466/mqtt");
    }
    else {
      Serial.print("Falha, rc= ");
      Serial.print(client.state());
      Serial.println();
      Serial.println("Tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void setup() {
  // pinMode(2, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, port);
  client.setCallback(callback);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    TempAndHumidity data = dhtSensor.getTempAndHumidity();

    String temp = String(data.temperature, 2);
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println();
    client.publish("iotcp2rm99466/temperature", temp.c_str());

    String umidade = String(data.humidity, 2);
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println();
    client.publish("iotcp2rm99466/humidity", umidade.c_str());
  }
}
