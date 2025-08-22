// Codigo para conexao em HTTP
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHTesp.h"

DHTesp DHTSensor;
HttpClient http;

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* host = "https://api.thingspeak.com/update?api_key=6N5GRdgtSFGGHgbndD9A";

#define DHT_PIN 32

void setup() {
  Serial.begin(115200);
  DHTSensor.setup(DHT_PIN, DHTesp::DHT22);

  // Configuracao do WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando na rede WiFi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  Serial.println("Conectado ao WiFi");
}

void loop() {

  // Leitura do sensor de temperatura
  TempAndHumidity data = DHTSensor.getTempAndHumidity();


  // Definindo portas para a URL e definindo HTTP
  String field1 = "&field1=" + String(data.temperature);
  String field2 = "&field2=" + String(data.humidity);

  String url = host + field1 + field2;
  Serial.println(url);
  http.begin(url);

}
//--------------------------------------------------------
// Codigo para conexao em MQTT
/*#include <Arduino.h>
#include <WiFi.h>
#include <DHTesp.h>
#include <PubSubClient.h>

#define DHT_PIN 32

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;

DHTesp DHTSensor;
WiFiClient ESPClient;
PubSubClient client(ESPClient);


void setup() {
  Serial.begin(115200);
  DHTSensor.setup(DHT_PIN, DHTesp::DHT22);

  // Configuracao do WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando na rede WiFi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  Serial.println("Conectado ao WiFi");

  // Configuracao do Cliente MQTT
  client.setServer(mqtt_broker, mqtt_port);
  Serial.println("Conectando ao broker MQTT");
  while(!client.connected()){
    Serial.print(".");
    if(client.connect("ESP32Client-8787afd")){
      Serial.println("Conectado ao broker MQTT");
    }else{
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  // Leitura do sensor de temperatura e umidade
  TempAndHumidity data = DHTSensor.getTempAndHumidity();

  delay(200);

}*/
