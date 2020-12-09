/* Se ainda esta com duvida, nao esquece que o README é seu amigo
*/
#include "DataECU.h"
#include <FlexCAN.h>
#include <cppQueue.h>
#include <Arduino.h>

DataECUClass::DataECUClass()
{
  perfectFrame.id = 0x7FB;
  perfectFrame.ext = 0;
  perfectFrame.len = 8;
  perfectFrame.rtr = 0;
  packages = new Queue (sizeof(CAN_message_t), numberOfPackages, implementation, overwrite); //Fila FIFO de tamanho 5

}

 /** \brief Inicialição do Listener para receber os pacotes da ECU
 *
 *  \param Can  determina por qual porta será feita a comunicação Can0 ou Can1
 */

void DataECUClass::begin (FlexCAN &Can){

  CAN_filter_t ECUFilter = getFilter();

  Can.begin(transmissionRate);  //inicializa com a taxa de transmissão - 1000 kbps
  Can.attachObj(&DataECU);

  //Filtro por hardware para pacotes com a ID selecionada
  for (uint8_t filterNum = 0; filterNum < 16; filterNum++) {
    Can.setFilter(ECUFilter, filterNum);
    Can.setMask(ECUFilter.id << 18, filterNum);
  }

  attachGeneralHandler();

}

 /** \brief Retorna o parametros do filtro que será usado para separar os pacotes com
 * ID selecinada, no caso da ECU (ID 0x7FB).
 */

CAN_filter_t DataECUClass::getFilter() {
  CAN_filter_t filter;
  filter.id = perfectFrame.id;
  filter.ext = perfectFrame.ext;
  filter.rtr = perfectFrame.rtr;
  return filter;
}

 /** \brief Retorna taxa de transmissão usada na ECU.
 */

uint32_t DataECUClass::getTransmissionRate() {
  return transmissionRate;
}

 /** \brief Retrona do vetor de dados da mensagem recebida sem processamento
 * na base hexadecimal.
 */

uint8_t* DataECUClass::getRawData() {
  return rawData;
}

 /** \brief Retorna os tamanho do rawData.
 */

uint8_t DataECUClass::getSizeRawData() {
  return sizeRawData;
}

 /** \brief Retorna os valores corrigidos e na respectivas unindades de cada sensor,
 *  caso não haja dado recebido, é atribuido o valor -1 a todos os elementos indicando
 *  que houve uma falha de comunicação.
 */

float* DataECUClass::getSensorData() {
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

 /** \brief Retorna os tamanho do sensorData.
 */

uint8_t DataECUClass::getSizeSensorData() {
  return sizeSensorData;
}

 /** \brief Retorna se está recebendo dados da ECU.
 *   A flag só atualiza com chamada de _getSensorData_ então é necessário que seja feita a
 *   chamada pelo menos uma vez antes de se obter o valor correto de ConectedECU
 */

bool DataECUClass::getConectedECU(){
    return flagConectedECU;
}

 /** \brief Função herdada da classe CANListener e chamada por interrupção toda vez que chega
 *  um pacote de dados.
 *
 * \param frame  Contém os valores do pacote recebido
 *
 * \param mailbox  Índice da caixa de correio do pacote recebido (inútil para essa aplicação
 *  já que todas caixas de correio possuem o mesmo filtro, portanto aceitam a mesma informação)
 *
 */

void DataECUClass::gotFrame(CAN_message_t &frame, int mailbox)
{
  if (frame.id != perfectFrame.id || frame.ext != perfectFrame.ext || frame.len != perfectFrame.len)
    return; // caso ID nao seja o correspondente com o esperado a função encerra (é redundante com o filto de hardware)

    //Descomentar a linha abaixo para começar a imprimir a mensagem exatamente como se recebe
  //   printFrame(frame, mailbox);
  flagCompletedMessage = armazenationFrame(frame, mailbox);

  if (flagCompletedMessage){
    timer = 0;
    translateMessage();
  }
}

 /** \brief Imprime o frame da forma como a mensagem é recebida
 */

void DataECUClass::printFrame(CAN_message_t &frame, int mailbox)
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

 /** \brief Armazena somente os valores de dados do pacotes CAN, e faz algumas verificações
 * para garantir que tudo foi recebido de maneira correta
 */

bool DataECUClass::armazenationFrame(CAN_message_t &frame, int mailbox)
{
  CAN_message_t temp;
  if (packages -> isFull() != 1)
  {
    packages -> push(&frame);   //adiciona um elemento a fila
  }
  if (frame.buf[0] == 0xFB && frame.buf[1] == 0xFA && packages -> isFull() == 1) /// Primeira etapa de verificação (bytes fixos)
  {
    for (int j = 0; j < numberOfPackages; j++)
    {
      packages -> pop(&temp);
      for (int i = 0; i < temp.len; i++)
      {
        rawData[i + 8 * j] = temp.buf[i];
      }
    }
    if (rawData[6 + 8 * 3] == 0x1E && rawData[7 + 8 * 3] == 0xFC) ///Segunda etapa de verificação (bytes fixos)
    {
      uint8_t sum = 0;
      for (int i = 0; i < adressCheckSum; i++)
      {
        sum += rawData[i];
      }
      if (sum == rawData[adressCheckSum])   /// Terceira etapa de verificação (soma dos bytes anteriores)
        return 1;
    }
  }
  return 0; //Caso a informação esteja de alguma maneira corrompida, a função retorna 0
}

 /** \brief Traduz da mensagem para valores reais de cada grandeza, cada uma com sua respectiva unidade.
 * A tabela de conversão foi fornecida pela INJEPRO e deve constar em algum lugar nessa biblioteca.
 */

void DataECUClass::translateMessage()
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

DataECUClass DataECU;
