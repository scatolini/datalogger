#include <DataECU.h>

void setup(void)
{
  delay(1000);
  Serial.println(F("Hello Teensy Single CAN Receiving Example With Objects."));

  DataECU.begin(Can0); // Selecina porta CAN 0 e inicializa a comunicação com a ECU
}

// -------------------------------------------------------------
void loop(void)
{
  delay(500);
  float *sensorDataECU = DataECU.getSensorData(); // Valores dos sensores
  uint8_t sizeSensorDataECU = DataECU.getSizeSensorData(); // Quantidade de sensores diferentes
  for (int i = 0; i < sizeSensorDataECU; i++)
  {
    Serial.print(sensorDataECU[i]);
    Serial.print(" ");
  }
  Serial.println();

}
