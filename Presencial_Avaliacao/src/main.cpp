
// Este código é um exemplo de como conectar um ESP32 a uma rede Wi-Fi 
// e enviar dados de temperatura, umidade e presença para o ThingSpeak via MQTT. 

// Inclusao de bibliotecas
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <DHTesp.h>

// Configuracao de pinos
#define DHT_PIN 32
#define LED_BLUE 12
#define LED_RED 14
#define INPUT_PIR 33

// Instâncias de clientes
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
DHTesp DHTSensor;

// Configurações de Wi-Fi
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// Configurações do MQTT
const char *mqttServer = "mqtt3.thingspeak.com";
const char *mqttClientId = "CQkoNRg8NQw1LTkFFzIKJSs";
const char *mqttUser = "CQkoNRg8NQw1LTkFFzIKJSs";
const char *mqttPassword = "7BdFDbszFKkk3sKRoIPzwHyo";
const char *mqttChanelId = "2896126";
const int mqttPort = 1883;

// Configuracao inicial
void setup()
{
  Serial.begin(115200);

  // Configuracao do WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando na rede WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
  }
  Serial.println("Conectado ao WiFi");

  // Conexão com o MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  if (!mqttClient.connected())
  {
    Serial.println("");
    Serial.println("Conectando na rede MQTT...");
    if (mqttClient.connect(mqttClientId, mqttUser, mqttPassword))
    {
      Serial.println("Conectado ao MQTT");
      Serial.println("");
    }
    else
    {
      Serial.print("Falha na conexão com o MQTT");
      Serial.print("");
      delay(2000);
      return;
    }
  }

  // Configuração dos sensores
  DHTSensor.setup(DHT_PIN, DHTesp::DHT22);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(INPUT_PIR, INPUT);
}

void loop()
{
  // Leitura do sensor de temperatura
  TempAndHumidity data = DHTSensor.getTempAndHumidity();

  // Definição do tópico MQTT
  String topic = "channels/" + String(mqttChanelId) + "/publish";
  String payload = "field1=" + String(data.temperature, 2) + "&field2=" + String(data.humidity, 2) + "&field3=" + String(digitalRead(INPUT_PIR));
  Serial.print("Enviando dados para o MQTT: ");
  if (mqttClient.publish(topic.c_str(), payload.c_str()))
  {
    Serial.println("Dados enviados com sucesso!");
  }
  else
  {
    Serial.println("Falha ao enviar dados.");
  }

  // Controle do LED vermelho
  // O LED vermelho acende quando a temperatura está fora do intervalo de 20 a 30 graus Celsius
  // E apaga quando a temperatura está dentro do intervalo.
  if (data.temperature <= 20 || data.temperature >= 30) {
    digitalWrite(LED_RED, LOW); // Fora do intervalo
  } else {
    digitalWrite(LED_RED, HIGH); // Dentro do intervalo
  }

  // Leitura do sensor de presença & controle do LED azul
  // O LED azul acende quando o sensor de presença detecta movimento
  int valorPir = digitalRead(INPUT_PIR);
  if (valorPir == HIGH)
  {
    digitalWrite(LED_BLUE, HIGH);
  }
  else
  {
    digitalWrite(LED_BLUE, LOW);
  }

  delay(5000); // Aguarda 5 segundos antes de enviar os dados novamente
  mqttClient.loop(); // Mantém a conexão MQTT ativa
}