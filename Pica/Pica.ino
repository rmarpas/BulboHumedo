#include <DallasTemperature.h>
#include <OneWire.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <stdio.h>


#define oneWirePin 18
#define greenLED 5
#define redLED 4

const int blinkInterval = 500;
OneWire ourWire(oneWirePin);
DallasTemperature sensors(&ourWire);

DeviceAddress sondaAddr[5];
static int numSondas = 0;

//const char* ssid = "VIRGIN-telco_439F";
//const char* password = "mTQTYfFptMT7tQ";
const char* ssid = "IntroduceSSID";
const char* password = "IntroducePass";
const char* ta1 = "Temperatura A-1";
const char* ta2 = "Temperatura A-2";
const char* ta3 = "Temperatura A-3";
const char* ta4 = "Temperatura A-4";
const char* ta5 = "Temperatura A-5";

const char* mqtt_server = "IntroduceIPServidorTB";
unsigned long lastMsg;

/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);
ThingsBoard tb(espClient);
const char* tbTOKEN ="IntroduceTOKENTB";

int wstatus = WL_IDLE_STATUS;

TaskHandle_t TaskLed;

/*
 * 1Wire functions
 */
void registrarSondas(){
  unsigned char addr[8];
  int i, j;
  numSondas = 0;
  for(j=0;j<5;j++)
  {
  if(!ourWire.search(addr)){
      Serial.println("No más direcciones");
      break;
  }
  else {
    
    Serial.print("sonda ");
    Serial.print(j);
    Serial.print(": ");
    for(i=0;i<8;i++)
    {
      Serial.print(' ');
      Serial.print(addr[i],HEX);
    }
    Serial.println(" ");
    numSondas++;
    memcpy(sondaAddr[j],addr,8);
    }
  }
}

int getResolucion (DeviceAddress addr){
  return sensors.getResolution(addr);
}

/* resolucion por defecto de la sonda es de 12 bits.
 * resolucion configurada | precision obtenida | tiempo de lectura
 *         12 bits        |      0.0625ºC      |      750.00 ms
 *         11 bits        |      0.1250ºC      |      350.00 ms
 *         10 bits        |      0.2500ºC      |      187.50 ms
 *          9 bits        |      0.5000ºC      |       93.75 ms
 */
void setResolucion(DeviceAddress addr, int nivel){
  sensors.setResolution(addr,nivel);
}

void loopdetectprinttemp(){
  int i;
  registrarSondas();
  for(i=0;i<numSondas;i++)
  {
    sensors.requestTemperatures();
    Serial.print("Temperatura sonda ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(sensors.getTempC(sondaAddr[i]));
    //Serial.print(sensors.getTempCByIndex(0));
    delay(1000);

  }
}

/*
 * MQTT functions
 */
void info() {
  Serial.print("failed, status code =");
  Serial.print(client.state());
  Serial.print(", conected=");
  Serial.println(client.connected());
}
void mqttconnect() {
  /* Loop until reconnected */
  info();
  while (client.connected() == 0) {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "ESP32Client";
    /* connect now */
    info();
    if (client.connect("Temperatura A-1", tbTOKEN, NULL)) {
      Serial.println("Dentro if");
      Serial.println("connected");
      /* subscribe topic with default QoS 0*/
      //client.subscribe(LED_TOPIC);
      info();
    }
    else {
      info();
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.print("Estado WiFi");
      Serial.println(WiFi.status());
      if (client.state() < 0 )
      {
        Serial.print("Estado WiFi: ");
        Serial.println(WiFi.status());
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
      }
      Serial.println(" try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(5000);
    }
  }
}


void reconnect()
{
  while (!tb.connected())
  {
    Serial.print("Connecting to ThingsBoard...");
    if(tb.connect(mqtt_server, tbTOKEN))
    {
      Serial.println("Conectado");
    }
    else
    {
      Serial.println("Conexion failed, retrying in 2 seconds");
      delay(2000);
    }
    Serial.print("tb conectado = ");
    Serial.println(tb.connected());
  }
  Serial.println("fin reconectar");
}
void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  /* we got '1' -> on */

}

void sendMsg () {
  Serial.println("sendmsg");
  bool OKtemp = true;
  sensors.requestTemperatures();
Serial.print("Número de sondas: ");
Serial.println(numSondas);
if (numSondas==0)
  registrarSondas();
  //prepare string
  sensors.requestTemperatures();
    
  for(int i=0; i<=numSondas - 1;i++)
  {
    Serial.print("Temperatura sonda ");
    Serial.print (i);
    Serial.print(": ");
    float temp = sensors.getTempC(sondaAddr[i]);
    Serial.println(temp);
    if (temp < 70.0 && temp > 5.0)
    {
      switch (i)
      {
        case 0:
          tb.sendTelemetryFloat(ta1, temp);
          break;
        case 1:
          tb.sendTelemetryFloat(ta2, temp);
          break;
        case 2:
          tb.sendTelemetryFloat(ta3, temp);
          break;
        case 3:
          tb.sendTelemetryFloat(ta4, temp);
          break;
        case 4:
          tb.sendTelemetryFloat(ta5, temp);
          break;
        
      }
      
    }
  }
  
}
void blink(int led, int interval){
  digitalWrite(led,HIGH);
  delay(interval);
      digitalWrite(led,LOW);
      delay(interval);
}

void TaskLedcode(void * pvParameters){
  Serial.println("Run Task");
  while (1)
    {
      if (WiFi.status() != WL_CONNECTED)
        blink(redLED,blinkInterval);
      else
        blink(greenLED, blinkInterval);

      if(!tb.connected())
        blink(redLED,blinkInterval);
      else
        blink(greenLED, blinkInterval);

      delay (blinkInterval*2);
  }
}
void setup() {
  // put your setup code here, to run once:
  pinMode (5,OUTPUT);
  pinMode (4,OUTPUT);

  xTaskCreatePinnedToCore(
      TaskLedcode, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &TaskLed,  /* Task handle. */
      0); /* Core where the task should run */

  Serial.begin(115200);

  registrarSondas();
  
}

void loop() {
    /* if client was disconnected then try to reconnect again */
  wstatus = WiFi.status();
  if(wstatus != WL_CONNECTED)
  {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    while (wstatus != WL_CONNECTED)
    {  
      wstatus = WiFi.begin(ssid,password);
      //Serial.print(wstatus);Serial.print(" - ");
      //byte numSsid = WiFi.scanNetworks();
      //Serial.println(numSsid);
      delay(1500);
      switch(wstatus)
      {
        case WL_CONNECTED:
          Serial.println("WL_CONNECTED");
          break;
        case WL_NO_SHIELD:
          Serial.println("WL_NO_SHIELD");
          break;  
        case WL_IDLE_STATUS:
          Serial.println("WL_IDLE_STATUS");
          break;
        case WL_NO_SSID_AVAIL:
          Serial.println("WL_NO_SSID_AVAIL");
          break;
        case WL_SCAN_COMPLETED:
          Serial.println("WL_SCAN_COMPLETED");
          break;
        case WL_CONNECT_FAILED:
          Serial.println("WL_CONNECT_FAILED");
          break;
        case WL_CONNECTION_LOST:
          Serial.println("WL_CONNECTION_LOST");
          break;
        case WL_DISCONNECTED:
          Serial.println("WL_DISCONNECTED");
          break;
      }
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  if (!tb.connected()) {
    while(!tb.connected())
    {
      reconnect();
    }
  }
  /* this function will listen for incomming
    subscribed topic-process-invoke receivedCallback */
  tb.loop();
  /* we measure temperature every 3 secs
    we count until 15 secs reached to avoid blocking program if using delay()*/
  long now = millis();
  if (now - lastMsg > 1000 * 15) {
    lastMsg = now;
    sendMsg();
  }

  delay(1000);

}
