#include <SD.h>
#include <SPI.h>

#define SIZE_NAME_SDFILE 15     //Tamanho máximo do nome do arquivo
#define SIZE_STORED_ERROR 20    //Quantidade máxima de erros diferentes

/// Estrutura para representar os erros gerados
typedef struct errorMessage_t {
  uint8_t id;     // identificador único
  String message; // Mensagem que será guardada
} errorMessage_t;

class SDlogClass
{
  private:    
    int chipSelect;        // Seleciona em quais portas está instalado o módulo do cartão (para mais informações consulte um exemplo da biblioteca SD)
    unsigned int fileNumber = 0;          // número do arquivo utilizado

    // Nomes dos arquivos em formato String e char*
    // A função SD.open e SD.exists só aceita em char entretato é mais fácil de manipular Strings
    String fileData;
    String fileError;
    char fileData_char [SIZE_NAME_SDFILE];      
    char fileError_char [SIZE_NAME_SDFILE];

    errorMessage_t storedError[20]; // armazena os tipos de erros antes de envia-lo ao SD
    errorMessage_t noError;         // seria algo como NULL

    String data = "";       // armazena os dados antes de envia-lo ao SD
    int quantitySensor;   // indica o número de sensores que serão armazenados no arquivo .csv
    int pushedSensor = 0;     // indica a quatidade de sensores que já armazenadas por batelada (aumenta a cada push)

    String date = "";
    String time = "";

  public:
    SDlogClass();   //Construtor
    bool begin(int quantitySensor, const int chipSelect = BUILTIN_SDCARD);    // como padrão sempre seleciona o SD interno
    char* getFileData();              // retorna os nomes e o numéro dos arquivos
    char* getFileError();
    unsigned int getFileNumber();
    String getData();                 // retorna a string DATA
    errorMessage_t* getStoredError(); // retorna o vetor de estrutura dos erros

    void setDate(String date);
    void setTime(String time);

    void nameUpdate();              // aumenta em 1 o índice dos arquivos
    // Realiza o push com diferentes entradas de dados (P.S. se vc não sabe o q é push, da um google em filas c++)
    void dataPush(int value);
    void dataPush(unsigned long int value);   // necessário para armazenar os valores de millis()
    void dataPush(float value, int decimalPlace = 3);
    void dataPush(double value, int decimalPlace = 3);
    bool recordData();                        // grava os dados já selecionados no cartão
    bool errorPush(errorMessage_t newError);  //  realiza o push dos erros
    bool recordError();                      // grava os erros já selecionados no cartão
    bool initialLog(String nameSensor = "");  // grava algumas infromações iniciais no arquivo erros (obs: nao são erros)
    bool commentary(String comment);      // grava algum comentário caso se deseje

};

SDlogClass::SDlogClass(){
  // Inicialização inicial com os nomes dos arquivos
  fileData = "data" +(String)fileNumber + ".csv";
  fileError = "error" +(String)fileNumber+ ".txt";
  fileData.toCharArray(fileData_char,SIZE_NAME_SDFILE);
  fileError.toCharArray(fileError_char,SIZE_NAME_SDFILE);

  // define o que seria não ter erros armazenados
  noError.id = 0;   /// CUIDADO NÃO PODE HAVER ERROS COM ESSE ID'S
  noError.message = "";
  for(int i = 0; i < SIZE_STORED_ERROR; i++)
    storedError[i] = noError;
}

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

char* SDlogClass::getFileData(){
  return fileData_char;
}

char* SDlogClass::getFileError(){
  return fileError_char;
}

unsigned int SDlogClass::getFileNumber(){
  return fileNumber;
}

String SDlogClass::getData(){
  if(data.length() == NULL)
    return ""; // Não há dados para armazenar
  return data;
}

errorMessage_t* SDlogClass::getStoredError(){
  return storedError;
}

void SDlogClass::setDate(String date){
  this->date = date;
}

void SDlogClass::setTime(String time){
  this->time = time;
}

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

#ifndef __MK66FX1M0__
  #error "Teensy 3.6 is required to run this example"
#endif

SDlogClass SDlog;

const errorMessage_t deuRuim = { 1, "Erro teste"};
const errorMessage_t mundial = { 2, "Palmeiras nao tem mundial"};

const int chipSelect = BUILTIN_SDCARD; // Utilizando a SD interno, para outras opções consulte a biblioteca SD

elapsedMillis timerErrorLog = 0;
elapsedMillis timerErrorLog2 = 0;

void faltalBlink(int pin){ // sinaliza um erro fatal
  while(1){
  digitalWrite(pin, HIGH); // ativa o pino digital 13
  delay(250);            // espera por um segundo
  digitalWrite(pin, LOW);  // desativa o pino digital 13
  delay(250);            // espera por um segundo
  }
}

void setup() {
  pinMode(13,OUTPUT);
  
  Serial.begin(9600);
  delay(1000);
  Serial.print("Initializing SD card...");
  if (!SDlog.begin(4,chipSelect)) {   // Inicializa com a quantidade de dados de que será utilizada e onde está conectado o cartão SD
    Serial.println("Card failed, or not present");
    faltalBlink(13);
    return;
  }
  Serial.println("card initialized.");
  Serial.println(SDlog.getFileData());    // Imprimi o nome dos arquivos
  Serial.println(SDlog.getFileError());

  
  SDlog.setTime("18:30:42");
  SDlog.setDate("01/11/2020");
  SDlog.initialLog("Analog1,Analog2,Analog3,millis");
  
  SDlog.commentary("Rosas sao vermelhas");
  SDlog.commentary("Violetas sao azuis");
  
  delay(3000);
}


void loop() {
  
  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    SDlog.dataPush(sensor);
  }
  SDlog.dataPush(millis());
  Serial.println(SDlog.getData());
  if(SDlog.recordData())
    Serial.println("deu certo");
  else {
    Serial.println("error opening data#.txt");
  }
  SDlog.recordData();

  if(timerErrorLog2 > 200){
    if(!SDlog.errorPush(deuRuim))
      Serial.print("problemas com o push");
    SDlog.errorPush(mundial);
    timerErrorLog2 = 0;
  }

  errorMessage_t *storedError = SDlog.getStoredError();
  if(timerErrorLog > 1000){
    Serial.println(storedError[0].message);
    Serial.println(storedError[1].message);
    if(!SDlog.recordError())   // Grava as mesnsagens de erro no SD
          Serial.println("error opening error#.txt");
    timerErrorLog = 0;
  }


  delay(100);
}
