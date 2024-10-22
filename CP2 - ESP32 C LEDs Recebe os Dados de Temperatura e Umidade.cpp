// Código para ESP32 recebendo dados de temperatura e umidade via MQTT e acionando LEDs

#include <WiFi.h>
#include <PubSubClient.h>

// Configurações de Wi-Fi e MQTT
const char* ssid = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* password = ""; // Senha da rede Wi-Fi
const char* mqtt_server = "rm99466-mqttserver.eastus.cloudapp.azure.com"; // Endereço do servidor MQTT
int port = 1883; // Porta do servidor MQTT

// Definição dos LEDs para temperatura
int ledAzulTemp = 21; // LED azul para temperaturas abaixo de 18°C
int ledVerdeTemp = 19; // LED verde para temperaturas maior e igual a 18°C e menor que 35°C
int ledVermelhoTemp = 18; // LED vermelho para temperaturas maior e igual a 35°C

// Definição dos LEDs para umidade
int ledVermelhoHum = 4; // LED vermelho para umidade abaixo de 20%
int ledAmareloHum = 2; // LED amarelo para umidade maior e igual a 20% e menor que 100%
int ledVerdeHum = 15; // LED verde para umidade maior e igual a 100%

// Variáveis para armazenar os dados recebidos
float temperature; // Variável para temperatura
float umidade; // Variável para umidade

WiFiClient espClient;
PubSubClient client(espClient);

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

  // Exibe informações de conexão
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Função de callback chamada quando uma mensagem MQTT é recebida
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Imprime o payload recebido
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

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

// Função para reconectar ao broker MQTT
void reconnect() {
  // Enquanto não conectar, ficará tentando conectar no broker automáticamente
  while (!client.connected()) {
    Serial.println("Tentando a conexão com o MQTT");

    // Gera um ID de cliente aleatório
    String clientId = "ESP32CP2RM99466Client-";
    clientId += String(random(0xffff), HEX);

    // Tenta conectar ao broker MQTT
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado com sucesso!");
      client.subscribe("iotcp2rm99466/temperature"); // Subscreve ao tópico de temperatura
      client.subscribe("iotcp2rm99466/humidity"); // Subscreve ao tópico de umidade
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

// Função com a Lógica para acionar os LEDs de acordo com a temperatura recebida
void handleTemperature(float temperature) {
  if (temperature >= 35) {
    digitalWrite(ledAzulTemp, LOW);
    digitalWrite(ledVerdeTemp, LOW);
    digitalWrite(ledVermelhoTemp, HIGH); // Liga o LED Vermelho
  } else if (temperature >= 18 && temperature < 35) {
    digitalWrite(ledAzulTemp, LOW);
    digitalWrite(ledVerdeTemp, HIGH); // Liga o LED Verde
    digitalWrite(ledVermelhoTemp, LOW);
  } else {
    digitalWrite(ledAzulTemp, HIGH); // Liga o LED azul
    digitalWrite(ledVerdeTemp, LOW);
    digitalWrite(ledVermelhoTemp, LOW);
  }
}

// Função com a Lógica para acionar os LEDs de acordo com a umidade recebida
void handleHumidity(float umidade) {
  if (umidade >= 100) {
    digitalWrite(ledVerdeHum, HIGH); // Liga o LED Verde
    digitalWrite(ledAmareloHum, LOW);
    digitalWrite(ledVermelhoHum, LOW);
  } else if (umidade >= 20 && umidade < 100) {
    digitalWrite(ledVerdeHum, LOW);
    digitalWrite(ledAmareloHum, HIGH); // Liga o LED Amarelo
    digitalWrite(ledVermelhoHum, LOW);
  } else {
    digitalWrite(ledVerdeHum, LOW);
    digitalWrite(ledAmareloHum, LOW);
    digitalWrite(ledVermelhoHum, HIGH); // Liga o LED Vermelho
  }
}

void setup() {
  // Configuração inicial do ESP32
  Serial.begin(115200); // Inicia a comunicação serial

  // Configura os pinos dos LEDs como saída
  // Pinos de Temperatura
  pinMode(ledAzulTemp, OUTPUT);
  pinMode(ledVerdeTemp, OUTPUT);
  pinMode(ledVermelhoTemp, OUTPUT);

  // Pinos de Umidade
  pinMode(ledVermelhoHum, OUTPUT);
  pinMode(ledAmareloHum, OUTPUT);
  pinMode(ledVerdeHum, OUTPUT);

  
  setup_wifi(); // Conecta ao Wi-Fi
  client.setServer(mqtt_server, port); // Define o servidor MQTT
  client.setCallback(callback); // Define a função de callback
}

void loop() {
  // Loop principal do ESP32
  if (!client.connected()) {
    reconnect(); // Reconecta ao broker se desconectado
  }
  client.loop(); // Mantém a conexão MQTT ativa
}
