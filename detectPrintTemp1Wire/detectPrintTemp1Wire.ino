#include <DallasTemperature.h>
#include <OneWire.h>

OneWire ourWire(5);
DallasTemperature sensors(&ourWire);

DeviceAddress sondaAddr[5];
static int numSondas = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  registrarSondas();
}
void registrarSondas(){
  unsigned char addr[8];
  int i, j;
  numSondas = 0;
  for(j=0;j<5;j++)
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
int getResolucion (DeviceAddress addr)
{
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
void printAddress(DeviceAddress da){
  for (int i = 0; i< 8; i++){
    Serial.print(" ");
    if (da[i] <16) Serial.print("0");
    Serial.print(da[i],HEX);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  int i;
  
  for(i=0;i<numSondas;i++)
  {
    sensors.requestTemperatures();
    Serial.print("Temperatura sonda ");
    printAddress(sondaAddr[i]);
    Serial.print(": ");
    Serial.println(sensors.getTempC(sondaAddr[i]));
    //Serial.print(sensors.getTempCByIndex(0));
    delay(1000);

  }
  

}
