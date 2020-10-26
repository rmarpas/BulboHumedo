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
  int humedadFC28 = map(analogRead(FC28), 2800, 4096, 0, 100);
  int humedadCatalex = map(analogRead(Catalex), 0, 2048, 0, 100);

  Serial.print("Humedad sensor FC-28:");
  Serial.print(100-humedadFC28);
  Serial.print("%");
  Serial.println();

  Serial.print("Humedad sensor Catalex:");
  Serial.print(humedadCatalex);
  Serial.print("%");
  Serial.println();

  delay(1000);

}
