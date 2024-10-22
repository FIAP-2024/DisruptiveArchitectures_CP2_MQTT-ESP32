#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "rm99466-mqttserver.eastus.cloudapp.azure.com";
int port = 1883;
String stMac;
char mac[50];
char clientId[50];

// LEDs de Temperatura (Temperature)
int ledAzulTemp = 21;
int ledVerdeTemp = 19;
int ledVermelhoTemp = 18;

// LEDs de Umidade (Humidity)
int ledVermelhoHum = 4;
int ledAmareloHum = 2;
int ledVerdeHum = 15;

// Variáveis para dados recebidos
float temperature;
float umidade;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.println("Conectando ao WiFi: ");
  Serial.println(ssid);
  Serial.println();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
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
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  
  // Recebendo dados de temperatura
  if (String(topic) == "iotcp2rm99466/temperature") {
    temperature = message.toFloat();
    Serial.println("Temperatura: " + String(temperature));
    handleTemperature(temperature);
  }

  // Recebendo dados de umidade
  else if (String(topic) == "iotcp2rm99466/humidity") {
    umidade = message.toFloat();
    Serial.println("Umidade: " + String(umidade));
    handleHumidity(umidade);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentando conexão MQTT...");

    String clientId = "ESP32CP2RM99466Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado com sucesso!");
      client.subscribe("iotcp2rm99466/temperature");
      client.subscribe("iotcp2rm99466/humidity");
    } else {
      Serial.print("Falha, rc= ");
      Serial.print(client.state());
      Serial.println();
      Serial.println("Tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void handleTemperature(float temperature) {
  if (temperature >= 35) {
    digitalWrite(ledAzulTemp, LOW);
    digitalWrite(ledVerdeTemp, LOW);
    digitalWrite(ledVermelhoTemp, HIGH);
  } else if (temperature >= 18 && temperature < 35) {
    digitalWrite(ledAzulTemp, LOW);
    digitalWrite(ledVerdeTemp, HIGH);
    digitalWrite(ledVermelhoTemp, LOW);
  } else {
    digitalWrite(ledAzulTemp, HIGH);
    digitalWrite(ledVerdeTemp, LOW);
    digitalWrite(ledVermelhoTemp, LOW);
  }
}

void handleHumidity(float umidade) {
  if (umidade >= 100) {
    digitalWrite(ledVerdeHum, HIGH);
    digitalWrite(ledAmareloHum, LOW);
    digitalWrite(ledVermelhoHum, LOW);
  } else if (umidade >= 50 && umidade < 100) {
    digitalWrite(ledVerdeHum, LOW);
    digitalWrite(ledAmareloHum, HIGH);
    digitalWrite(ledVermelhoHum, LOW);
  } else {
    digitalWrite(ledVerdeHum, LOW);
    digitalWrite(ledAmareloHum, LOW);
    digitalWrite(ledVermelhoHum, HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  
  // Configuração dos LEDs
  pinMode(ledAzulTemp, OUTPUT);
  pinMode(ledVerdeTemp, OUTPUT);
  pinMode(ledVermelhoTemp, OUTPUT);
  pinMode(ledVermelhoHum, OUTPUT);
  pinMode(ledAmareloHum, OUTPUT);
  pinMode(ledVerdeHum, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
