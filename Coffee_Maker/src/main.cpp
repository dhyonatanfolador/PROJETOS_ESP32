// Projeto: Máquina de Café com MQTT e SMS via Twilio

// Este código demonstra como controlar uma máquina de café inteligente utilizando sensores ultrassônicos e um termistor.
// O sensor ultrassônico mede a altura do copo para garantir que ele esteja posicionado corretamente antes de servir o café.
// O termistor monitora a temperatura do café para assegurar que ele seja servido na temperatura ideal.
// Além disso, o código integra funcionalidades avançadas, como o envio de mensagens SMS via Twilio para notificações,
// e a conexão com um broker MQTT para comunicação em tempo real, permitindo monitoramento e controle remoto da máquina.

// Inclusão das bibliotecas necessárias
#include <Arduino.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <time.h>

// Configurações de pinos
#define PIN_TRIG_CAFE 5
#define PIN_ECHO_CAFE 17
#define PIN_TRIG_CUP 4
#define PIN_ECHO_CUP 16
#define PIN_BOMBA 15
#define PIN_BOTTON 12

// Instâncias de clientes
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
HTTPClient http;

// Configurações de Wi-Fi
const char* ssid = "Wokwi-GUEST";  // Nome da rede Wi-Fi
const char* password = "";         // Senha da rede Wi-Fi

// Constantes de temperatura
const float BETA = 3950;

// Configurações do Twilio
const char* accountSid = "AC9bc92c484d3ae00958313a1f30453efe"; // SID da conta Twilio
const char* auth_token = "dac51302c57052aaed15fc013459c625";   // Token de autenticação Twilio
const char* fromNumber = "+13862103530";                       // Número de origem (Twilio)
const char* toNumber = "+18777804236";                         // Número de destino para envio de SMS

// Configurações do MQTT
const char* mqttBroker = "broker.emqx.io"; // Endereço do broker MQTT
const int mqttPort = 1883;                 // Porta do broker MQTT
const char* topic = "Coffee/Broker";       // Tópico MQTT para publicação/assinatura

// Variáveis globais
long timezone = -4;          // Fuso horário
byte daysavetime = 1;        // Horário de verão (1 = ativo, 0 = inativo)

bool processoAtivo = false; // Variável para controle do processo
float alturaCopo = 0.0;      // Altura do copo medida pelo sensor ultrassônico
float alturaCafe = 0.0;      // Altura do café medida pelo sensor ultrassônico
float temperatura = 0.0;     // Temperatura medida pelo termistor

// Função de envio de SMS via Twilio
// Envia uma mensagem SMS usando a API do Twilio
int sendSms(String message, String toNumber, String fromNumber) {
  String url = "https://api.twilio.com/2010-04-01/Accounts/" + String(accountSid) + "/Messages.json";
  http.begin(url); // Inicializa a requisição HTTP
  http.setAuthorization(accountSid, auth_token); // Autenticação básica
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Cabeçalho da requisição
  String payload = "To=" + toNumber + "&From=" + fromNumber + "&Body=" + message;

  int httpCode = http.POST(payload); // Envia a requisição POST
  http.end(); // Finaliza a conexão HTTP
  return httpCode; // Retorna o código HTTP da resposta
}

// Função para obter a hora local
String getHour() {
  struct tm tmstruct;
  getLocalTime(&tmstruct);
  return String(tmstruct.tm_hour) + ":" + String(tmstruct.tm_min) + ":" + String(tmstruct.tm_sec);
}

// Função para obter a temperatura
// Calcula a temperatura com base no valor analógico lido do sensor
float getTemperature() {
  int analogValue = analogRead(34);
  return 1 / (log(1 / (4095.0 / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;
}

// Função para calcular a distância usando sensor ultrassônico
// Retorna a distância em centímetros
float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH); // Mede o tempo do pulso
  return (duration * 0.0343) / 2; // Converte o tempo em distância
}

// Callback para mensagens recebidas via MQTT
// Processa mensagens recebidas no tópico assinado
void receiveMessage(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
      message += (char)payload[i]; // Converte o payload em string
  }
  Serial.println("Mensagem recebida: " + message); // Exibe a mensagem no console
}

// Função para finalizar o processo
// Desliga a bomba e encerra o processo
void finalizaProcesso() {
  processoAtivo = false; // Encerra o processo
  digitalWrite(PIN_BOMBA, LOW); // Desliga a bomba
}

// Função principal do processo
// Controla a bomba d'água com base na altura do café, altura do copo e temperatura
void iniciarProcesso() {
  
  bool processoAtivo = true; // Variável para controlar o estado do processo

  while (processoAtivo) {
    temperatura = getTemperature(); // Obtém a temperatura
    alturaCopo = getDistance(PIN_TRIG_CUP, PIN_ECHO_CUP); // Mede a altura do copo
    alturaCafe = getDistance(PIN_TRIG_CAFE, PIN_ECHO_CAFE); // Mede a altura do café

    if ((alturaCopo - alturaCafe) > 10 && temperatura > 50) {
        digitalWrite(PIN_BOMBA, HIGH); // Liga a bomba se as condições forem atendidas
    } else {
        finalizaProcesso(); // Encerra o processo se as condições não forem atendidas
    }
  }
}

// Configuração inicial
void setup() {
  Serial.begin(115200);

  pinMode(PIN_BOTTON, INPUT); // Configura o pino do botão
  attachInterrupt(digitalPinToInterrupt(PIN_BOTTON), iniciarProcesso, RISING); // Configura interrupção para o botão

  // Configuracao do WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando na rede WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }
  Serial.println("Conectado ao WiFi");

  // Configuração do MQTT Broker
  mqttClient.setServer(mqttBroker, mqttPort);
  mqttClient.setCallback(receiveMessage);
  Serial.println("");
  Serial.println("Conectando no Broker...");
  while (!mqttClient.connected()) {
    String clientId = "nsifb-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Conectado ao Broker");
        Serial.println("");
        mqttClient.subscribe(topic);
    } else {
        Serial.print("Falha na conexão com o Broker");
        Serial.println("");
        delay(200);
        return;
    }
  }

  // Configuração de Timer
  configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

  // Configuração de Sensores
  pinMode(PIN_TRIG_CAFE, OUTPUT);
  pinMode(PIN_ECHO_CAFE, INPUT);
  pinMode(PIN_TRIG_CUP, OUTPUT);
  pinMode(PIN_ECHO_CUP, INPUT);

  // Configuração da Bomba d'Água
  pinMode(PIN_BOMBA, OUTPUT);
  digitalWrite(PIN_BOMBA, LOW); // Garante que a bomba esteja desligada inicialmente
}

// Loop principal
void loop() {
  mqttClient.loop(); // Mantém a conexão MQTT ativa
}