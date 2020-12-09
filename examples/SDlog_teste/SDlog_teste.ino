#include <SDlog.h>


#ifndef __MK66FX1M0__
  #error "Teensy 3.6 is required to run this example"
#endif

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
  SDlog.recordData(); // é so pra testar, nao deve armazenar nada

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
