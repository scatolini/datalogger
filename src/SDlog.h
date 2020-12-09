/* FAlta comentar aqui
*/
#ifndef SDLOG_H
#define SDLOG_H

#include <SD.h>
#include <SPI.h>
#include <Arduino.h>

#define SIZE_NAME_SDFILE 15     //Tamanho m�ximo do nome do arquivo
#define SIZE_STORED_ERROR 20    //Quantidade m�xima de erros diferentes

/// Estrutura para representar os erros gerados
typedef struct errorMessage_t {
  uint8_t id;     // identificador �nico
  String message; // Mensagem que ser� guardada
} errorMessage_t;

class SDlogClass
{
  private:
    int chipSelect;        // Seleciona em quais portas est� instalado o m�dulo do cart�o (para mais informa��es consulte um exemplo da biblioteca SD)
    unsigned int fileNumber = 0;          // n�mero do arquivo utilizado

    // Nomes dos arquivos em formato String e char*
    // A fun��o SD.open e SD.exists s� aceita em char entretato � mais f�cil de manipular Strings
    String fileData;
    String fileError;
    char fileData_char [SIZE_NAME_SDFILE];
    char fileError_char [SIZE_NAME_SDFILE];

    errorMessage_t storedError[20]; // armazena os tipos de erros antes de envia-lo ao SD
    errorMessage_t noError;         // seria algo como NULL

    String data = "";       // armazena os dados antes de envia-lo ao SD
    int quantitySensor;   // indica o n�mero de sensores que ser�o armazenados no arquivo .csv
    int pushedSensor = 0;     // indica a quatidade de sensores que j� armazenadas por batelada (aumenta a cada push)

    String date = "";   // data provavelmente vinda do GPS
    String time = "";   // tempo provavelmente vinda do GPS

  public:
    SDlogClass();   //Construtor
    bool begin(int quantitySensor, const int chipSelect = BUILTIN_SDCARD);    // como padr�o sempre seleciona o SD interno
    char* getFileData();              // retorna os nomes e o num�ro dos arquivos
    char* getFileError();
    unsigned int getFileNumber();
    String getData();                 // retorna a string DATA
    errorMessage_t* getStoredError(); // retorna o vetor de estrutura dos erros

    void setDate(String date);
    void setTime(String time);

    void nameUpdate();              // aumenta em 1 o �ndice dos arquivos
    // Realiza o push com diferentes entradas de dados (P.S. se vc n�o sabe o q � push, da um google em filas c++)
    void dataPush(int value);
    void dataPush(unsigned long int value);   // necess�rio para armazenar os valores de millis()
    void dataPush(float value, int decimalPlace = 3);
    void dataPush(double value, int decimalPlace = 3);
    bool recordData();                        // grava os dados j� selecionados no cart�o
    bool errorPush(errorMessage_t newError);  //  realiza o push dos erros
    bool recordError();                      // grava os erros j� selecionados no cart�o
    bool initialLog(String nameSensor = "");  // grava algumas infroma��es iniciais no arquivo erros (obs: nao s�o erros)
    bool commentary(String comment);      // grava algum coment�rio caso se deseje

};

extern SDlogClass SDlog;

#endif // SDLOG_H
