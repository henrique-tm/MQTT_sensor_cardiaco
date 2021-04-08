#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define UpperThreshold 555
#define LowerThreshold 550
//Limites para transição do pulso

bool BPMTiming=false;
bool BeatComplete=false;
float LastTime=0;
float BPM=0;
float BPMmed=0;
float interval=0;
int cont=0;
int sensor=0;
//variaveis do sistema

const char* SSID = "G7 ThinQ_9134"; //Seu SSID da Rede WIFI
const char* PASSWORD = "23456789"; // A Senha da Rede WIFI
const char* MQTT_SERVER = "test.mosquitto.org"; //Broker do Mosquitto.org
char msg[50];
WiFiClient CLIENT;
PubSubClient MQTT(CLIENT);


//CONFIGURAÇÃO DA INTERFACE DE REDE
void setupWIFI() {
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Conectando na rede: ");
  Serial.println(SSID);
  while (WiFi.status() != WL_CONNECTED) {
   Serial.print(".");
   delay(500);
  }
}
void setup(void) {
  Serial.begin(115200);
  setupWIFI(); 
  MQTT.setServer(MQTT_SERVER, 1883);
}
//definição da comunicação serial e server
void reconectar() {
  while (!MQTT.connected()) {
    Serial.println("Conectando ao Broker MQTT.");
    if (MQTT.connect("ESP8266")) {
      Serial.println("Conectado com Sucesso ao Broker");
    } else {
      Serial.print("Falha ao Conectador, rc=");
      Serial.print(MQTT.state());
      Serial.println(" tentando se reconectar...");
      delay(3000);
    }
  }
}
//rotina em caso de perda de sinal
void loop(void) {
  if (!MQTT.connected()) {
    reconectar();//conecta o controlador a rede em caso de queda 
  }
  MQTT.loop();
  Batimento();
  Serial.println(BPMmed);
  snprintf (msg, 75, "%f", BPMmed);
  Serial.print("Valor publicado: ");
  Serial.println(msg);
  MQTT.publish("sensorcardiac", msg);
}
void Batimento(){
  for(cont=0;cont<20;cont++){
    int value=analogRead(sensor);
    //Serial.println(value);
    // Leitura do sensor de frequencia cardiaca
    
    if(value>UpperThreshold){
      if(BeatComplete){
        interval=millis()-LastTime;
        BPM=float(60000/interval);
        BPMTiming=false;
        BeatComplete=false;
      }
      if(BPMTiming==false){
        LastTime=millis();
        BPMTiming=true;
      }
    }
    //Registro do intervalo entre os batimentos cardíacos
    
    if((value<LowerThreshold)&(BPMTiming))
      BeatComplete=true;
    //atualização da variável de estado
    
    BPMmed=BPM+BPMmed;
    delay(100);
    //Atualização da variavel para calculo do BPM a cada 0,1s
  }
  BPMmed=BPMmed/cont;
  cont=0;
  //Calculo da média de BPM
}
