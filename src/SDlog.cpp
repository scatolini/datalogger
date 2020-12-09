/* Falta comentar aqui
*/


#include "SDlog.h"
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>

SDlogClass::SDlogClass(){
  // Inicializa��o inicial com os nomes dos arquivos
  fileData = "data" +(String)fileNumber + ".csv";
  fileError = "error" +(String)fileNumber+ ".txt";
  fileData.toCharArray(fileData_char,SIZE_NAME_SDFILE);
  fileError.toCharArray(fileError_char,SIZE_NAME_SDFILE);

  // define o que seria n�o ter erros armazenados
  noError.id = 0;   /// CUIDADO N�O PODE HAVER OUTRO COM ESSE ID'S
  noError.message = "";
  for(int i = 0; i < SIZE_STORED_ERROR; i++)
    storedError[i] = noError;
}

 /** \brief Iniciali��o do SDlog
 *
 *  \param quantitySensor  � a quantidade de sensores armazenados (determina o n�mero de colunas no arquivo .csv)
 *
 *  \param chipSelect  pinos pela qual ocorrer� a comunica��o SPI
 *
 *  \return  1  se n�o ocorrer erros e 0 caso o cart�o n�o esteja instalado ou n�o seja compat�vel
 */

bool SDlogClass::begin(int quantitySensor, const int chipSelect){

  this->quantitySensor = quantitySensor;
  this->chipSelect = chipSelect;
  // verifica se o cart�o SD est� presente e foi inicializado:
  if (!SD.begin(BUILTIN_SDCARD)) {
    // n�o fa�a nada mais
    return 0;
  }

  nameUpdate();  // Atualiza os nomes dos arquivos para um novo nome
  return 1;

}

 /** \brief Retorna o nome do arquivo onde ser�o guardados os dados
 */

char* SDlogClass::getFileData(){
  return fileData_char;
}

 /** \brief Retorna o nome do arquivo onde ser�o guardados os erros
 */

char* SDlogClass::getFileError(){
  return fileError_char;
}

 /** \brief Retorna o �ndice utilizado para a cria��o dos nomes dos arquivos
 */

unsigned int SDlogClass::getFileNumber(){
  return fileNumber;
}

 /** \brief Retorna retorna os dados armazenados que ainda n�o foram gravados
 *   ou seja, ainda n�o est�o no cart�o
 */

String SDlogClass::getData(){
  if(data.length() == NULL)
    return ""; // N�o h� dados para armazenar
  return data;
}

 /** \brief Retorna retorna os erros armazenados que ainda n�o foram gravados
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
 *  Precisa de uma atualiza��o constante para que n�o haja defasagem muito grande
 */

void SDlogClass::setTime(String time){
  this->time = time;
}

 /** \brief Atualiza os nomes dos arquivos, procurando o menor �ndice que esteja vago
 */

void SDlogClass::nameUpdate(){
  while(SD.exists(fileData_char) || SD.exists(fileError_char))
  {
    fileNumber++;
    fileData = "data" + (String)fileNumber + ".csv";    // Atualiza a String at� que um novo nome seja encontrado
    fileError = "error" + (String)fileNumber + ".txt";
    fileData.toCharArray(fileData_char,SIZE_NAME_SDFILE);   //Atualiza o vetor de char com o novo nome
    fileError.toCharArray(fileError_char,SIZE_NAME_SDFILE);
  }
}

 /** \brief As pr�ximas fun��es possuem a mesma utilidade, por�m tem entradas diferentes
 *  para conseguir englobar o maior tipo de dados. Armazena o dado de maneira tempor�ria
 *  no formato csv (comma separated virgula) at� a informa��o esteja completa e pronta para
 *  ser gravada. S� lembrando que a fun��o millis() devolve em unsigned long int.
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

 /** \brief Grava os dados no cart�o SD
 *
 *  \return 0 quando n�o foi poss�vel abrir o arquivo com o nome desejado ou
 *  a quantidade de dados armazendos n�o � mesma da estipulada
 */

bool SDlogClass::recordData(){
  if(pushedSensor != quantitySensor)
  {
    pushedSensor = 0;
    data.remove(0); // Limpa toda a String para a proxima batelada
    return 0; // A quantidade sensores n�o � mesma que foi estipulada
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
    return 0; // arquivos n�o podem ser aberto
  }
}

 /** \brief O erro ao contrario do dado ser� tratado de maneira qualitativa, portanto a frequ�ncia
 *  de grava��o ser� bem menor, logo podem acontecer v�rios iguais, que n�o fazem sentido serem armazenados,
 *  logo este push armazena de maneira tempor�ria erros, sem repeti-los, at� que o vetor seja apagado
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

 /** \brief Grava os erros no cart�o SD
 *
 *  \return 0 quando n�o foi poss�vel abrir o arquivo com o nome desejado ou
 *  a quantidade de dados armazendos n�o � mesma da estipulada
 */

bool SDlogClass::recordError(){
  for(int i = 0; i < SIZE_STORED_ERROR; i++)
  {
    if (storedError[i].id != 0) // caso haja 1 erro, o la�o � quebrado
      break;
    else if (i == SIZE_STORED_ERROR - 1) // Percorreu todo vetor por�m n�o h� erros salvos
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
    return 0; // arquivo n�o pode ser aberto
  }
}

 /** \brief Grava algumas informa��es preliminares como nome dos arquivos, quantidade
 *  de dados que ser�o armazenados e o nomes dos sensores
 *
 *  \param nameSensor  nomes dos sensores que ser�o, � so um guia pra saber o que �
 *  o que. N�o existe um padr�o que precisa ser seguido, mas como sugest�o fica que sejam
 *  usando v�rgulas para separar e com as unidades entre par�nteses
 *
 *  \return 0 quando n�o foi poss�vel abrir o arquivo com o nome desejado
 */

bool SDlogClass::initialLog(String nameSensor){
  File file = SD.open(fileError_char, FILE_WRITE);
  if (file) {
    file.println("===============================================================");
    file.println("Seja bem vindo, vou comecar, ok?");
    file.println("Os nomes dos arquivos s�o:");
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
    file.print("H� no momento "+(String)quantitySensor+ " sensores armazenados ");
    file.println("(corresponde ao n�mero de colunas no arquivo data)");
    file.println("O nome dos sensores armazenados �:");
    file.println(nameSensor);
    file.close();
    return 1;
  }
  else{
    return 0; // arquivos n�o podem ser aberto
  }
}

 /** \brief Grava algum coment�rio adicional que se deseja colocar no arquivo
 *  de erros
 *
 *  \param comment  coment�rio
 *
 *  \return 0 quando n�o foi poss�vel abrir o arquivo com o nome desejado
 */

bool SDlogClass::commentary(String comment){
  File file = SD.open(fileError_char, FILE_WRITE);
  if (file) {
    file.println("Coment�rio:");
    file.println(comment);
    file.close();
    return 1;
  }
  else{
    return 0; // arquivos n�o podem ser aberto
  }
}

SDlogClass SDlog;
