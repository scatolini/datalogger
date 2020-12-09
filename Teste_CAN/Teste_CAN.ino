/*
   Object Oriented CAN example for Teensy 3.6 with Dual CAN buses
   By Collin Kidder. Based upon the work of Pawelsky and Teachop

   Both buses are set to 500k to show things with a faster bus.
   The reception of frames in this example is done via callbacks
   to an object rather than polling. Frames are delivered as they come in.
*/

#include <FlexCAN.h>
#include <cppQueue.h>


class DataECU : public CANListener
{
  private:
    const uint8_t numberOfPackages = 5; // numero de pacotes para uma determinada ID
    const uint32_t transmissionRate = 1000000; // 1000 kbps taxa de transmissão da ECU
    const uint8_t adressCheckSum = 34; // posição do byte responsavel pelo check sum 

    // Definindo a fila de pacotes
    Queue *packages;      //fila para armazenar os pacotes
    const QueueType implementation = FIFO;
    const bool overwrite = true;

    //Vetores para armazenar as mensagens
    uint8_t rawData [40]; // 40 bytes (5 packages x 8 lenth)
    float sensorData [9]; // 9 sensores diferentes
    const uint8_t sizeRawData = 40;
    const uint8_t sizeSensorData = 9;

    bool flagCompletedMessage = 0;  // indica qua mensagem foi recebida corretamente
    bool flagConectedECU = 0;       // indica se ECU está conectada
    elapsedMillis timer = 0;        // contador em ms do tempo que não se recebe pacotes
    const uint32_t timout = 500;    // tempo em máximo para definir se a ECU está conectada
    CAN_message_t perfectFrame;


  public:
    DataECU();

    CAN_filter_t getFilter();
    uint32_t getTransmissionRate();
    uint8_t* getRawData();
    float* getSensorData();
    uint8_t getSizeRawData();
    uint8_t getSizeSensorData();

    void translateMessage();
    bool armazenationFrame(CAN_message_t &frame, int mailbox);
    void printFrame(CAN_message_t &frame, int mailbox);
    void gotFrame(CAN_message_t &frame, int mailbox); //overrides the parent version so we can actually do something
};

DataECU::DataECU()
{
  perfectFrame.id = 0x7FB;
  perfectFrame.ext = 0;
  perfectFrame.len = 8;
  perfectFrame.rtr = 0;
  packages = new Queue (sizeof(CAN_message_t), numberOfPackages, implementation, overwrite);

}
CAN_filter_t DataECU::getFilter() {
  CAN_filter_t filter;
  filter.id = perfectFrame.id;
  filter.ext = perfectFrame.ext;
  filter.rtr = perfectFrame.rtr;
  return filter;
}
uint32_t DataECU::getTransmissionRate() {
  return transmissionRate;
}

uint8_t* DataECU::getRawData() {
  return rawData;
}

uint8_t DataECU::getSizeRawData() {
  return sizeRawData;
}

float* DataECU::getSensorData() {
  if (timer > timout){
    flagConectedECU = 0;
      for (int i = 0; i < sizeSensorData; i++)
        sensorData[i] = -1;
  }
  else if (timer <= timout){
    flagConectedECU = 1;
  }
  return sensorData;
}

uint8_t DataECU::getSizeSensorData() {
  return sizeSensorData;
}

void DataECU::printFrame(CAN_message_t &frame, int mailbox)
{
  Serial.print(" ID: ");
  Serial.print(frame.id, HEX);
  Serial.print(" Data: ");
  for (int c = 0; c < frame.len; c++)
  {
    Serial.print(frame.buf[c], HEX);
    Serial.write(' ');
  }
  Serial.print(" Time: ");
  Serial.print(timer);
  timer = 0;
  Serial.write('\r');
  Serial.write('\n');
}

void DataECU::gotFrame(CAN_message_t &frame, int mailbox)
{
  if (frame.id != perfectFrame.id || frame.ext != perfectFrame.ext || frame.len != perfectFrame.len)
    return 0; // caso ID nao seja o correspondente com o esperado a função encerra (é redundante com o filto de hardware)

  //   printFrame(frame, mailbox);
  flagCompletedMessage = armazenationFrame(frame, mailbox);
  
  if (flagCompletedMessage){
    timer = 0;
    translateMessage();
  }
}

bool DataECU::armazenationFrame(CAN_message_t &frame, int mailbox)
{
  CAN_message_t temp;
  if (packages -> isFull() != 1)
  {
    packages -> push(&frame);   //adiciona um elemento a fila
  }
  if (frame.buf[0] == 0xFB && frame.buf[1] == 0xFA && packages -> isFull() == 1) // Primeira etapa de verificação (bytes fixos)
  {
    for (int j = 0; j < numberOfPackages; j++)
    {
      packages -> pop(&temp);
      for (int i = 0; i < temp.len; i++)
      {
        rawData[i + 8 * j] = temp.buf[i];
      }
    }
    if (rawData[6 + 8 * 3] == 0x1E && rawData[7 + 8 * 3] == 0xFC) //Segunda etapa de verificação (bytes fixos)
    {
      uint8_t sum = 0;
      for (int i = 0; i < adressCheckSum; i++)
      {
        sum += rawData[i];
      }
      if (sum == rawData[adressCheckSum])   // Terceira etapa de verificação (soma dos bytes anteriores)
        return 1;
    }
  }
  return 0;
}
void DataECU::translateMessage()
{
  float rotacaoDoMotor       = (float)((int16_t)((rawData[0 + 8 * 0] << 8) + rawData[1 + 8 * 0])); // [rpm]
  float velocidade           = (float)((int16_t)((rawData[2 + 8 * 0] << 8) + rawData[3 + 8 * 0])); // [km/h]
  float pressaoDoOleo        = (float)((int16_t)((rawData[4 + 8 * 0] << 8) + rawData[5 + 8 * 0])) / 10; // [bar]

  float temperaturaDoMotor   = (float)((int16_t)((rawData[0 + 8 * 1] << 8) + rawData[1 + 8 * 1])) / 10; // [ºC]
  float pressaoDeCombustivel = (float)((int16_t)((rawData[2 + 8 * 1] << 8) + rawData[3 + 8 * 1])) / 10; // [bar]
  float tps                  = (float)((int16_t)((rawData[6 + 8 * 1] << 8) + rawData[7 + 8 * 1])) / 10; // [%]

  float map_                 = (float)((int16_t)((rawData[0 + 8 * 2] << 8) + rawData[1 + 8 * 2])) / 1000; // [bar]
  float temperaturaDoAr      = (float)((int16_t)((rawData[2 + 8 * 2] << 8) + rawData[3 + 8 * 2])) / 10; // [ºC]
  float sondaWB              = (float)((int16_t)((rawData[6 + 8 * 2] << 8) + rawData[7 + 8 * 2])) / 1000; // [lambda]

  sensorData[0] = rotacaoDoMotor;
  sensorData[1] = velocidade;
  sensorData[2] = pressaoDoOleo;
  sensorData[3] = temperaturaDoMotor;
  sensorData[4] = pressaoDeCombustivel;
  sensorData[5] = tps;
  sensorData[6] = map_;
  sensorData[7] = temperaturaDoAr;
  sensorData[8] = sondaWB;
}

DataECU dataECU;

// -------------------------------------------------------------
void setup(void)
{
  delay(1000);
  Serial.println(F("Hello Teensy Single CAN Receiving Example With Objects."));

  CAN_filter_t ECUFilter = dataECU.getFilter();

  Can0.begin(dataECU.getTransmissionRate());  //inicializa com a taxa de transmissão - 1000 kbps
  Can0.attachObj(&dataECU);

  //if using enable pins on a transceiver they need to be set on
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  //Filtro por hardware para pacotes com a ID selecionada
  for (uint8_t filterNum = 0; filterNum < 16; filterNum++) {
    Can0.setFilter(ECUFilter, filterNum);
    Can0.setMask(ECUFilter.id << 18, filterNum);
  }

  dataECU.attachGeneralHandler();
}


// -------------------------------------------------------------
void loop(void)
{
  delay(500);
  float *sensorDataECU = dataECU.getSensorData();
  uint8_t sizeSensorDataECU = dataECU.getSizeSensorData();
  for (int i = 0; i < sizeSensorDataECU; i++)
  {
    Serial.print(sensorDataECU[i]);
    Serial.print(" ");
  }
  Serial.println();

}
