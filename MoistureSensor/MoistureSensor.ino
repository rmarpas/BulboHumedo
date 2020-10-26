#include <stdio.h>

#define FC28 34
#define Catalex 35

void setup() {
  // put your setup code here, to run once:
  pinMode(FC28, INPUT);
  pinMode(Catalex, INPUT);

  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

  int humedadFC28 = analogRead(FC28);
  int humedadCatalex = analogRead(Catalex);

  Serial.print("Humedad sensor FC-28:");
  Serial.print(humedadFC28);
  Serial.println();

  Serial.print("Humedad sensor Catalex:");
  Serial.print(humedadCatalex);
  Serial.println();

  delay(1000);

}
