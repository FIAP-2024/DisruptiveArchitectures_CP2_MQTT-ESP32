// Código para ESP32 enviando dados de temperatura e umidade usando DHT22 e MQTT

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// Define o pino do sensor DHT22
const int DHT_PIN = 15;

DHTesp dhtSensor;

// Configurações de Wi-Fi e MQTT
const char* ssid = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* password = ""; // Senha da rede Wi-Fi
const char* mqtt_server = "rm99466-mqttserver.eastus.cloudapp.azure.com"; // Endereço do servidor MQTT
int port = 1883; // Porta do servidor MQTT
String stMac; // Armazena o endereço MAC do ESP32
char mac[50];
char clientId[50];

// Objetos do cliente Wi-Fi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0; // Armazena o tempo da última mensagem enviada
#define MSG_BUFFER_SIZE (50) // Tamanho do buffer de mensagens
float temp = 0; // Variável para temperatura
float umi = 0; // Variável para umidade
int value = 0;

void setup_wifi() {
  // Função para conectar ao Wi-Fi
  delay(10);
  Serial.println();
  Serial.println("Conectando ao WiFi: ");
  Serial.println(ssid);
  Serial.println();

  WiFi.mode(WIFI_STA); // Configura o modo Wi-Fi do ESP32 como estação
  WiFi.begin(ssid, password); // Inicia a conexão com o Wi-Fi

  // Tenta conectar ao Wi-Fi até obter sucesso
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Falha ao conectar!");
  }

  randomSeed(micros()); // Gera uma semente aleatória para o cliente MQTT

  // Exibe informações de conexão
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  stMac = WiFi.macAddress(); // Obtém o endereço MAC do dispositivo
  stMac.replace(":", "_"); // Substitui os ':' por '_' no endereço MAC
  Serial.println(stMac); // Exibe o endereço MAC
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Função de callback chamada quando uma mensagem MQTT é recebida
  Serial.println("Mensagem enviada [");
  Serial.println(topic);
  Serial.println("]");
  Serial.println();

  // Imprime o payload recebido
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Controle simples para alterar o estado do DHT_PIN
  if ((char)payload[0] == '1') {
    digitalWrite(DHT_PIN, LOW); // Desliga o sensor
  } else {
    digitalWrite(DHT_PIN, HIGH); // Liga o sensor
  }
}

void reconnect() {
  // Função para reconectar ao broker MQTT
  while (!client.connected()) {
    Serial.println("Tentando a conexão com o MQTT");

    // Gera um ID de cliente aleatório
    String clientId = "ESP32CP2RM99466Client-";
    clientId += String(random(0xffff), HEX);

    // Tenta conectar ao broker MQTT
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado com sucesso!");
      client.publish("iotcp2rm99466/mqtt", "iotcp2rm99466"); // Publica uma mensagem inicial
      client.subscribe("iotcp2rm99466/mqtt"); // Subscreve ao tópico desejado
    } else {
      // Em caso de falha, imprime o código de erro e aguarda 5 segundos
      Serial.print("Falha, rc= ");
      Serial.print(client.state());
      Serial.println();
      Serial.println("Tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void setup() {
  // Configuração inicial do ESP32
  // pinMode(2, OUTPUT);
  Serial.begin(115200); // Inicia a comunicação serial
  setup_wifi(); // Conecta ao Wi-Fi
  client.setServer(mqtt_server, port); // Define o servidor MQTT
  client.setCallback(callback); // Define a função de callback
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22); // Configura o sensor DHT22
}

void loop() {
  // Loop principal do ESP32
  if (!client.connected()) {
    reconnect(); // Reconecta ao broker se desconectado
  }
  client.loop(); // Mantém a conexão MQTT ativa

  unsigned long now = millis(); // Obtém o tempo atual
  if (now - lastMsg > 2000) { // Verifica se 2 segundos se passaram
    lastMsg = now;
    TempAndHumidity data = dhtSensor.getTempAndHumidity(); // Lê os dados do sensor

    // Converte a temperatura para string e publica no tópico MQTT
    String temp = String(data.temperature, 2);
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println();
    client.publish("iotcp2rm99466/temperature", temp.c_str());

    // Converte a umidade para string e publica no tópico MQTT
    String umidade = String(data.humidity, 2);
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println();
    client.publish("iotcp2rm99466/humidity", umidade.c_str());
  }
}
