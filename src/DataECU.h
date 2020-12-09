/* Se ainda esta com duvida, nao esquece que o README é seu amigo
*/
#ifndef DATAECU_H
#define DATAECU_H

#include <FlexCAN.h>
#include <cppQueue.h>
#include <Arduino.h>

class DataECUClass : public CANListener
{
  private:
    const uint8_t numberOfPackages = 5; // numero de pacotes para uma determinada ID
    const uint32_t transmissionRate = 1000000; // 1000 kbps taxa de transmissão da ECU
    const uint8_t adressCheckSum = 34; // posição do byte responsavel pelo check sum

    /// Definindo a fila de pacotes
    Queue *packages;      //fila para armazenar os pacotes
    const QueueType implementation = FIFO;
    const bool overwrite = true;

    ///Vetores para armazenar os dados
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
    DataECUClass();

    /// Funções para ter acesso a parâmetros privados
    CAN_filter_t getFilter();
    uint32_t getTransmissionRate();
    uint8_t* getRawData();
    float* getSensorData();
    uint8_t getSizeRawData();
    uint8_t getSizeSensorData();
    bool getConectedECU();

    void begin (FlexCAN &Can = Can0);
    void gotFrame(CAN_message_t &frame, int mailbox); //sobrescreve a versão parental para que alguma coisa possa ser feita
    void printFrame(CAN_message_t &frame, int mailbox);
    bool armazenationFrame(CAN_message_t &frame, int mailbox);
    void translateMessage();
};

extern DataECUClass DataECU;

#endif /* DataECU_h */
