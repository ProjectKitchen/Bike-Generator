#include <SoftwareSerial.h>
//Global definitions
SoftwareSerial ESPserial(2, 3); // RX | TX
int targetOutputVoltage = 582;
int hysteresisInSteps = 5;
int t1GatePin = 6;
int t2GatePin = 5;
int outputVoltagePin = A0;
int inputVoltagePin = A4;

int inputCurrentPin = A1;
int batteryCurrentPin = A2;
int rectifierCurrentPin = A3;

int outputVoltage = 0;
int inputVoltage = 0;
int inputCurrent = 0;
int batteryCurrent = 0;
int rectifierCurrent = 0;

int mode = 2; // 0 = off, 1 = buck, 2 = boost //buck= t1 pulsing, t2 off //boost= t1 on, t2 pulsing // off =  t1 off, t2 off

int pwmValueT1 = 1;
int pwmValueT2 = 0;

int count1 = 0;

int incomingByte = 0;

void setup()
{
  pinMode(t2GatePin, OUTPUT);  
  digitalWrite(t2GatePin, LOW);
  pinMode(t1GatePin, OUTPUT);  
  digitalWrite(t1GatePin, LOW);
  
  // Analog pins would not have to be declared
  pinMode(outputVoltagePin, INPUT);
  pinMode(inputVoltagePin , INPUT);
  pinMode(inputCurrentPin, INPUT);
  pinMode(batteryCurrentPin, INPUT);
  pinMode(rectifierCurrentPin, INPUT);

  TCCR0B = TCCR0B & 0b11111000 | 0x01;

  Serial.begin(115200);     // communication speed with the host
}


void loop()
{
    //This section checks if there is data in the rx buffer. 
    //If there is and it equals "1" the saved data is transfered to the ESP8266
    incomingByte = Serial.read();
    if(incomingByte == '1')
    {
      String serialSend;
      serialSend += String(inputVoltage);
      serialSend += String(',');
      serialSend += String(inputCurrent);
      serialSend += String(',');
      serialSend += String(outputVoltage);
      serialSend += String(',');
      serialSend += String(batteryCurrent);
      serialSend += String(',');
      serialSend += String(rectifierCurrent);
      serialSend += String(',');
      serialSend += String(mode);
      serialSend += String(',');
      serialSend += String(pwmValueT1);
      serialSend += String(',');
      serialSend += String(pwmValueT2);
      serialSend += String(',');
      serialSend += String(count1);
      serialSend += String('#');
      Serial.print(serialSend);
      count1++;
    }
    //Here data is read out from all the ADC pins and saved to be used later
  outputVoltage    = analogRead(outputVoltagePin);
  inputVoltage     = analogRead(inputVoltagePin);
  inputCurrent     = analogRead(inputCurrentPin);
  batteryCurrent   = analogRead(batteryCurrentPin);
  rectifierCurrent = analogRead(rectifierCurrentPin);
  
  choosemode();// see function
  pwmSafety(); // see function
  //The current pwm values are transfered to the ADCs which in turn power the mosfet gates accordingly
  analogWrite(t2GatePin, pwmValueT2);
  analogWrite(t1GatePin, pwmValueT1); 
  delay(5);
}


//One of the two main modes that makes sure that T2 is off and changes the pwmValue of T1 depending if the duty cycle should be reduced or not.
void buckmode(void)
{
  digitalWrite(pwmValueT2, LOW);
  pwmValueT2 = 0;
  if (outputVoltage < (targetOutputVoltage - hysteresisInSteps))
  {
    pwmValueT1 += 1;

  }
  else if (outputVoltage > (targetOutputVoltage + hysteresisInSteps))
  {
    pwmValueT1 -= 1;
  }
  else
  {
    ;
  }
}
//The other main mode that makes sure that T1 is on and changes the pwmValue of T2 depending if the duty cycle should be reduced or not.
void boostmode(void)
{
  digitalWrite(pwmValueT1, HIGH);
  pwmValueT1 = 255;
  if (outputVoltage <= (targetOutputVoltage - hysteresisInSteps))
  {
    pwmValueT2 -= 1;

  }
  else if (outputVoltage >= (targetOutputVoltage + hysteresisInSteps))
  {
    pwmValueT2 += 1;
  }
  else
  {
    ;
  }
}
//Off mode is called if an emergency stop is necessary.
void offmode(void)
{
  pwmValueT1 = 0;
  pwmValueT2 = 0;

}

//This function sets the mode that is to be executed, depending on the relationship of the input voltage to the target output voltage
void choosemode(void)
{
  if (inputVoltage <= targetOutputVoltage) // set boost mode
    {
      mode = 2;
      boostmode();
    }

  else if(inputVoltage > targetOutputVoltage) // set buck mode
    {
      mode = 1;
      buckmode();
    }
  else if(true) //set off mode 
    {
      mode = 0;
      offmode();
    }
  else
    {
      ;
    }
      
}

//as the pwm values can go below 0 or above 255 this function is called to stop error signals reaching the ADC
void pwmSafety(void)
{
  if (pwmValueT1 > 255)
  {
    pwmValueT1 = 255;
  }

  if (pwmValueT1 < 0)
  {
    pwmValueT1 = 0;
  }

  /*if(mode == 2)
  {
    if (pwmValueT2 > 255  )  //and voltage is higher than 9V  // || pwmValueT2 <= 0)
    {
      pwmValueT2 = 128;
    }
    if (pwmValueT2 < -1 && outputVoltage>300);
    {
      pwmValueT2 = 0;
    }
  }*/

  //else
  {
    if (pwmValueT2 > 255)
    {
      pwmValueT2 = 255;
    }
  
    if (pwmValueT2 < 0)
    {
      pwmValueT2 = 0;
    }
  } 
}
