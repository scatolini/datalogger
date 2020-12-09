/* Falta comentar aqui
*/


#include "SDlog.h"
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>

SDlogClass::SDlogClass(){
  // Inicialização inicial com os nomes dos arquivos
  fileData = "data" +(String)fileNumber + ".csv";
  fileError = "error" +(String)fileNumber+ ".txt";
  fileData.toCharArray(fileData_char,SIZE_NAME_SDFILE);
  fileError.toCharArray(fileError_char,SIZE_NAME_SDFILE);

  // define o que seria não ter erros armazenados
  noError.id = 0;   /// CUIDADO NÃO PODE HAVER OUTRO COM ESSE ID'S
  noError.message = "";
  for(int i = 0; i < SIZE_STORED_ERROR; i++)
    storedError[i] = noError;
}

 /** \brief Inicialição do SDlog
 *
 *  \param quantitySensor  é a quantidade de sensores armazenados (determina o número de colunas no arquivo .csv)
 *
 *  \param chipSelect  pinos pela qual ocorrerá a comunicação SPI
 *
 *  \return  1  se não ocorrer erros e 0 caso o cartão não esteja instalado ou não seja compatível
 */

bool SDlogClass::begin(int quantitySensor, const int chipSelect){

  this->quantitySensor = quantitySensor;
  this->chipSelect = chipSelect;
  // verifica se o cartão SD está presente e foi inicializado:
  if (!SD.begin(BUILTIN_SDCARD)) {
    // não faça nada mais
    return 0;
  }

  nameUpdate();  // Atualiza os nomes dos arquivos para um novo nome
  return 1;

}

 /** \brief Retorna o nome do arquivo onde serão guardados os dados
 */

char* SDlogClass::getFileData(){
  return fileData_char;
}

 /** \brief Retorna o nome do arquivo onde serão guardados os erros
 */

char* SDlogClass::getFileError(){
  return fileError_char;
}

 /** \brief Retorna o índice utilizado para a criação dos nomes dos arquivos
 */

unsigned int SDlogClass::getFileNumber(){
  return fileNumber;
}

 /** \brief Retorna retorna os dados armazenados que ainda não foram gravados
 *   ou seja, ainda não estão no cartão
 */

String SDlogClass::getData(){
  if(data.length() == NULL)
    return ""; // Não há dados para armazenar
  return data;
}

 /** \brief Retorna retorna os erros armazenados que ainda não foram gravados
 */

errorMessage_t* SDlogClass::getStoredError(){
  return storedError;
}

 /** \brief Seta a data interna no Datalogger, provavelmente vinda do GPS.
 */

void SDlogClass::setDate(String date){
  this->date = date;
}

 /** \brief Seta o tempo interno no Datalogger, provavelmente vindo do GPS.
 *  Precisa de uma atualização constante para que não haja defasagem muito grande
 */

void SDlogClass::setTime(String time){
  this->time = time;
}

 /** \brief Atualiza os nomes dos arquivos, procurando o menor índice que esteja vago
 */

void SDlogClass::nameUpdate(){
  while(SD.exists(fileData_char) || SD.exists(fileError_char))
  {
    fileNumber++;
    fileData = "data" + (String)fileNumber + ".csv";    // Atualiza a String até que um novo nome seja encontrado
    fileError = "error" + (String)fileNumber + ".txt";
    fileData.toCharArray(fileData_char,SIZE_NAME_SDFILE);   //Atualiza o vetor de char com o novo nome
    fileError.toCharArray(fileError_char,SIZE_NAME_SDFILE);
  }
}

 /** \brief As próximas funções possuem a mesma utilidade, porém tem entradas diferentes
 *  para conseguir englobar o maior tipo de dados. Armazena o dado de maneira temporária
 *  no formato csv (comma separated virgula) até a informação esteja completa e pronta para
 *  ser gravada. Só lembrando que a função millis() devolve em unsigned long int.
 *
 *  \param value dado a ser armazenado
 */

void SDlogClass::dataPush(int value){
  if(data.length() == NULL)
    data += String(value);
  else
  {
    data += ",";
    data += String(value);
  }
  pushedSensor++;
}

void SDlogClass::dataPush(unsigned long int value){
  if(data.length() == NULL)
    data += String(value);
  else
  {
    data += ",";
    data += String(value);
  }
  pushedSensor++;
}

void SDlogClass::dataPush(float value, int decimalPlace){
  if(data.length() == NULL)
      data += String(value, decimalPlace);
  else
  {
    data += ",";
    data += String(value, decimalPlace);
  }
  pushedSensor++;
}

void SDlogClass::dataPush(double value, int decimalPlace){
  if(data.length() == NULL)
      data += String(value, decimalPlace);
  else
  {
    data += ",";
    data += String(value, decimalPlace);
  }
  pushedSensor++;
}

 /** \brief Grava os dados no cartão SD
 *
 *  \return 0 quando não foi possível abrir o arquivo com o nome desejado ou
 *  a quantidade de dados armazendos não é mesma da estipulada
 */

bool SDlogClass::recordData(){
  if(pushedSensor != quantitySensor)
  {
    pushedSensor = 0;
    data.remove(0); // Limpa toda a String para a proxima batelada
    return 0; // A quantidade sensores não é mesma que foi estipulada
  }
  pushedSensor = 0;
  File file = SD.open(fileData_char, FILE_WRITE);
  if (file) {
    file.println(data);
    file.close();
    data.remove(0); // Limpa toda a String para a proxima batelada
    return 1;
  }
  else{
    data.remove(0); // Limpa toda a String para a proxima batelada
    return 0; // arquivos não podem ser aberto
  }
}

 /** \brief O erro ao contrario do dado será tratado de maneira qualitativa, portanto a frequência
 *  de gravação será bem menor, logo podem acontecer vários iguais, que não fazem sentido serem armazenados,
 *  logo este push armazena de maneira temporária erros, sem repeti-los, até que o vetor seja apagado
 *
 *  \return 0 quando o tamanho do vetor de erros lotar, portanto seu tamnaho deve ser aumentado
 */

bool SDlogClass::errorPush(errorMessage_t newError){
  for(int i = 0; i < SIZE_STORED_ERROR; i++)
  {
    if (storedError[i].id == newError.id)
      return 1;
    else if (storedError[i].id == noError.id) // quando ja existe
    {
      storedError[i] = newError;
      return 1;
    }
    else
      continue;
  }
  return 0;
}

 /** \brief Grava os erros no cartão SD
 *
 *  \return 0 quando não foi possível abrir o arquivo com o nome desejado ou
 *  a quantidade de dados armazendos não é mesma da estipulada
 */

bool SDlogClass::recordError(){
  for(int i = 0; i < SIZE_STORED_ERROR; i++)
  {
    if (storedError[i].id != 0) // caso haja 1 erro, o laço é quebrado
      break;
    else if (i == SIZE_STORED_ERROR - 1) // Percorreu todo vetor porém não há erros salvos
      return 0;
  }
  File file = SD.open(fileError_char, FILE_WRITE);
  if (file) {
    if (date.length() != NULL){   // caso haja algum tempo armazenado
        file.print("Real time: ");
        file.println(time);         // grava o tempo real
    }
    file.println("Timestamp: "+(String)millis()+" ->");
    for(int i = 0; i < SIZE_STORED_ERROR; i++)
    {
      if (storedError[i].id != noError.id){
        file.print("\tErro "+(String)storedError[i].id+": ");
        file.println(storedError[i].message);
      }
    }
    file.close();
    for(int i = 0; i < SIZE_STORED_ERROR; i++)
      storedError[i] = noError;
    return 1;
  }
  else{
    for(int i = 0; i < SIZE_STORED_ERROR; i++)
      storedError[i] = noError;
    return 0; // arquivo não pode ser aberto
  }
}

 /** \brief Grava algumas informações preliminares como nome dos arquivos, quantidade
 *  de dados que serão armazenados e o nomes dos sensores
 *
 *  \param nameSensor  nomes dos sensores que serão, é so um guia pra saber o que é
 *  o que. Não existe um padrão que precisa ser seguido, mas como sugestão fica que sejam
 *  usando vírgulas para separar e com as unidades entre parênteses
 *
 *  \return 0 quando não foi possível abrir o arquivo com o nome desejado
 */

bool SDlogClass::initialLog(String nameSensor){
  File file = SD.open(fileError_char, FILE_WRITE);
  if (file) {
    file.println("===============================================================");
    file.println("Seja bem vindo, vou comecar, ok?");
    file.println("Os nomes dos arquivos são:");
    file.println(fileData_char);
    file.println(fileError_char);
    if (date.length() != NULL && time.length() != NULL){
      file.print("Data e tempo real: ");
      file.print(date);
      file.print(" ");
      file.println(time);
    } else if (date.length() == NULL && time.length() != NULL){
      file.print("Tempo real: ");
      file.println(time);
    } else if (date.length() != NULL && time.length() == NULL){
      file.print("Data real: ");
      file.println(date);
    }
    file.print("Há no momento "+(String)quantitySensor+ " sensores armazenados ");
    file.println("(corresponde ao número de colunas no arquivo data)");
    file.println("O nome dos sensores armazenados é:");
    file.println(nameSensor);
    file.close();
    return 1;
  }
  else{
    return 0; // arquivos não podem ser aberto
  }
}

 /** \brief Grava algum comentário adicional que se deseja colocar no arquivo
 *  de erros
 *
 *  \param comment  comentário
 *
 *  \return 0 quando não foi possível abrir o arquivo com o nome desejado
 */

bool SDlogClass::commentary(String comment){
  File file = SD.open(fileError_char, FILE_WRITE);
  if (file) {
    file.println("Comentário:");
    file.println(comment);
    file.close();
    return 1;
  }
  else{
    return 0; // arquivos não podem ser aberto
  }
}

SDlogClass SDlog;
