
class ReadingPin{
  protected:
    int pin;
  public:
    ReadingPin(int pin);
    void setPin(int pin);
    int getPin();
    virtual float getValue() = 0;
    
};

ReadingPin::ReadingPin(int pin){
  this-> pin = pin;
}

void ReadingPin::setPin(int pin){
  this-> pin = pin;
}

int ReadingPin::getPin(){
  return pin;
}

class Analog: public ReadingPin{
  private:
    float minValue = 0;
    float maxValue = 100;
    float minVoltage = 0;
    float maxVoltage = 5;     // maioria dos sensores variam de 0 a 5V  
    static int analogResolution; // por padrão a IDE assume que a resolução é 10 bits
    float operationalVoltage = 12;  //  tensão de operação máxima devido dado pelo ampop na entrada
    
  public:
    Analog (int pin);
    Analog (int pin, float operationalVoltage);
    void begin(float minValue, float maxValue, float minVoltage, float maxVoltage);
    
    void setValueLimits(float minValue, float maxValue);
    void setVolatgeLimits(float minVoltage, float maxVoltage);
    
    static void setAnalogResolution(int analogResol);       // as funções que mexem com a resolução analógica devem...
    static int getAnalogResolution();                     // ser static pois é o mesmo para todos os tipos de objetos
    void setOperationalVoltage(float operationalVoltage);
    float getOperationalVoltage();
    
    float getValue();
    
};

int Analog::analogResolution = 10; // por padrão a IDE assume que a resolução é 10 bits 

Analog::Analog(int pin):ReadingPin(pin){}

Analog::Analog(int pin, float operationalVoltage):ReadingPin(pin){
  this-> operationalVoltage = operationalVoltage;
}

void Analog::begin(float minValue, float maxValue, float minVoltage, float maxVoltage){
  this-> minValue = minValue;
  this-> maxValue = maxValue;
  this-> minVoltage = minVoltage;
  this-> maxVoltage = maxVoltage;
}

void Analog::setValueLimits(float minValue, float maxValue){
  this-> minValue = minValue;
  this-> maxValue = maxValue;
}

void Analog::setVolatgeLimits(float minVoltage, float maxVoltage){
  this-> minVoltage = minVoltage;
  this-> maxVoltage = maxVoltage;
}

void Analog::setAnalogResolution(int analogResol){
  Analog::analogResolution = analogResol;
}

int Analog::getAnalogResolution(){
  return Analog::analogResolution;
}

void Analog::setOperationalVoltage(float operationalVoltage){
  this -> operationalVoltage = operationalVoltage;
}

float Analog::getOperationalVoltage(){
  return operationalVoltage;
}

float Analog::getValue(){
  int discreteValue = analogRead(pin);
  float voltageValue = discreteValue/(pow(2,analogResolution)-1)*operationalVoltage;
  float value = (voltageValue - minVoltage)/(maxVoltage - minVoltage);
  value  = value*(maxValue - minValue) + minValue;
  return value;
}

class Digital: public ReadingPin{
  private:

  public:
    Digital(int pin);
    float getValue();
};

Digital::Digital(int pin) : ReadingPin(pin){}

float Digital::getValue(){
  return digitalRead(pin);
}

//Analog analogico1(A3, 5);
Analog analogico1(A1);
void setup() {
   analogico1.begin(0, 100, 0, 5);
  //analogico1.setValueLimits(5, 20);
  //analogico1.setVolatgeLimits(2,4);
  Serial.begin(9600);
  Serial.print("Pino analógico: ");
  Serial.println(analogico1.getPin());
  Serial.print("Resolução em bits escolhida: ");
  Serial.println(analogico1.getAnalogResolution());
  Serial.print("Tensão máxima de operação: ");
  Serial.println(analogico1.getOperationalVoltage());
  Serial.println();
  delay(1000);

}

void loop() {
  analogReference(EXTERNAL);
  Serial.print("Leitura Analógica: ");
  Serial.println(analogico1.getValue());
  //Serial.println(analogRead(A3));
  delay(50);

}
