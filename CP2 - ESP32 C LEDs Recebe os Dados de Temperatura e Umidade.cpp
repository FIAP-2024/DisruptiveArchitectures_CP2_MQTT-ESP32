// Código para ESP32 recebendo dados de temperatura e umidade via MQTT e acionando LEDs

#include <WiFi.h>
#include <PubSubClient.h>

// Configurações de Wi-Fi e MQTT
const char* ssid = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* password = ""; // Senha da rede Wi-Fi
const char* mqtt_server = "rm99466-mqttserver.eastus.cloudapp.azure.com"; // Endereço do servidor MQTT
int port = 1883; // Porta do servidor MQTT

// Definição dos LEDs para temperatura
int ledAzulTemp = 21; // LED azul para temperaturas abaixo de 20°C
int ledVerdeTemp = 19; // LED verde para temperaturas entre 20°C e 30°C
int ledVermelhoTemp = 18; // LED vermelho para temperaturas acima de 30°C

// Definição dos LEDs para umidade
int ledVermelhoHum = 4; // LED vermelho para umidade abaixo de 30%
int ledAmareloHum = 2; // LED amarelo para umidade entre 30% e 60%
int ledVerdeHum = 15; // LED verde para umidade acima de 60%

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

  // Verifica o tópico da mensagem e converte o payload para float
  if (String(topic) == "iotcp2rm99466/temperature") {
    temperature = atof((char*)payload); // Converte o payload para temperatura
  } else if (String(topic) == "iotcp2rm99466/humidity") {
    umidade = atof((char*)payload); // Converte o payload para umidade
  }

  // Lógica para acionar os LEDs de acordo com a temperatura recebida
  if (temperature < 20) {
    digitalWrite(ledAzulTemp, HIGH); // Liga o LED azul
    digitalWrite(ledVerdeTemp, LOW);
    digitalWrite(ledVermelhoTemp, LOW);
  } else if (temperature < 30) {
    digitalWrite(ledAzulTemp, LOW);
    digitalWrite(ledVerdeTemp, HIGH); // Liga o LED verde
    digitalWrite(ledVermelhoTemp, LOW);
  } else {
    digitalWrite(ledAzulTemp, LOW);
    digitalWrite(ledVerdeTemp, LOW);
    digitalWrite(ledVermelhoTemp, HIGH); // Liga o LED vermelho
  }

  // Lógica para acionar os LEDs de acordo com a umidade recebida
  if (umidade < 30) {
    digitalWrite(ledVermelhoHum, HIGH); // Liga o LED vermelho
    digitalWrite(ledAmareloHum, LOW);
    digitalWrite(ledVerdeHum, LOW);
  } else if (umidade < 60) {
    digitalWrite(ledVermelhoHum, LOW);
    digitalWrite(ledAmareloHum, HIGH); // Liga o LED amarelo
    digitalWrite(ledVerdeHum, LOW);
  } else {
    digitalWrite(ledVermelhoHum, LOW);
    digitalWrite(ledAmareloHum, LOW);
    digitalWrite(ledVerdeHum, HIGH); // Liga o LED verde
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

void setup() {
  // Configuração inicial do ESP32
  Serial.begin(115200); // Inicia a comunicação serial
  setup_wifi(); // Conecta ao Wi-Fi
  client.setServer(mqtt_server, port); // Define o servidor MQTT
  client.setCallback(callback); // Define a função de callback

  // Configura os pinos dos LEDs como saída
  pinMode(ledAzulTemp, OUTPUT);
  pinMode(ledVerdeTemp, OUTPUT);
  pinMode(ledVermelhoTemp, OUTPUT);
  pinMode(ledVermelhoHum, OUTPUT);
  pinMode(ledAmareloHum, OUTPUT);
  pinMode(ledVerdeHum, OUTPUT);
}

void loop() {
  // Loop principal do ESP32
  if (!client.connected()) {
    reconnect(); // Reconecta ao broker se desconectado
  }
  client.loop(); // Mantém a conexão MQTT ativa
}
