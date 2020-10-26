/* Here ESP32 will keep 2 roles:
  1/ read data from DHT11/DHT22 sensor
  2/ control led on-off
  So it willpublish temperature topic and scribe topic bulb on/off
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>

/* change it with your ssid-password */
const char* ssid = "IntroduceSSID";
const char* password = "IntroducePass";

const char* mqtt_server = "IntroduceIPServidorTB";
int sensor_pin = 33;
int sensorg_pin = 32;

unsigned long lastMsg;

/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);

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
    if (client.connect("pica", "oOu1IrgpIm6g9QgSuZFb", NULL)) {
      Serial.println("connected");
      /* subscribe topic with default QoS 0*/
      //client.subscribe(LED_TOPIC);
      info();
    }
    else {
      info();
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(5000);
    }
  }
}

void sendMsg () {
  float gsensor = analogRead(sensorg_pin);
  float sensor = analogRead(sensor_pin);

  Serial.print("sensor: ");
  Serial.print(sensor);
  //Serial.print(" %t");
  Serial.print("sensor oro");
  Serial.print(gsensor);

  String sensor1 = String(sensor);
  String sensor2 = String(gsensor);

  //prepare string
  String payload = "{";
  payload += "\"sensor\":";
  payload += sensor1;
  payload+= ",";
  payload += "\"sensorG\":";
  payload += sensor2;
  payload += "}";
  
  //payload
  char attributes[100];
  payload.toCharArray(attributes,100);
  client.publish("v1/devices/me/telemetry", attributes);
  Serial.println(attributes);
  
}

void setup() {
  Serial.begin(115200);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  /* configure the MQTT server with IPaddress and port */
  client.setServer(mqtt_server, 1883);
  /* this receivedCallback function will be invoked
    when client received subscribed topic */
  client.setCallback(receivedCallback);
}
void loop() {
  /* if client was disconnected then try to reconnect again */
  if (client.connected() == 0) {
  mqttconnect();
  }
  /* this function will listen for incomming
    subscribed topic-process-invoke receivedCallback */
  client.loop();
  /* we measure temperature every 3 secs
    we count until 3 secs reached to avoid blocking program if using delay()*/
  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;
    sendMsg();
  }

  delay(500);
}
