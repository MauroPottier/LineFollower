  #include "SerialCommand.h"

#include <EEPROM.h>
#include "EEPROMAnything.h"

#define SerialPort Serial1

#define Baudrate 9600

SerialCommand sCmd(SerialPort);
bool debug;
bool start;
float lastErr;


unsigned long previous, calculationTime;
const int sensor [] = {A5, A4, A3, A2, A1, A0};         //x
struct param_t
{
  unsigned long cycleTime;
  int black[6];
  int white[6];
  float kp;
  float kd;
  float ki;
  float diff;
  int power;
  
} params;

void setup()
{
  SerialPort.begin(Baudrate);
  
  //pinnen H-brug
  pinMode(3, OUTPUT);  //A1
  pinMode(5, OUTPUT);  //A2 
  pinMode(9, OUTPUT);  //B2
  pinMode(10, OUTPUT); //B1

  sCmd.addCommand("set", onSet);
  sCmd.addCommand("start", onStart);
  sCmd.addCommand("stop", onStop);
  sCmd.addCommand("debug", onDebug);
  sCmd.addCommand("calibrate", onCalibrate);
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM_readAnything(0, params);
  float lastErr = 0;
  SerialPort.println("ready");
}

void loop()
{
  sCmd.readSerial();
 
  unsigned long current = micros();
  if (current - previous >= params.cycleTime)
  {
    
    previous = current;
       
    long normalised[6];
    for (int i = 0; i < 6; i++)
    {
      long value = analogRead(sensor[i]);
      normalised[i] = map(value, params.black[i], params.white[i], 0, 1000);
      normalised [i]=constrain(normalised [i],0,1000);
      
    }

    int index = 0;
    for (int i =1; i < 6 ;i++) if( normalised[i] < normalised[index]) index = i;
    
    int index2;
    index2= index;
    if (index ==0) index = 1;
    else if(index ==5) index = 4;
    
    
    long sNul = normalised[index];
    long sMinEen = normalised[index-1];
    long sPlusEen = normalised[index+1]; 


    float b = (sMinEen - sPlusEen ) / -2;
    float a = sPlusEen - b - sNul;
    if (a == 0) a = 1;


    float position = (-b / (2*a) * 10);
    position = position + (index*10-25);
 
    if (index2 == 0) position = -25;
    if (index2 == 5) position = 25;
 
  
    float error = -position; //error = setpoint - input

    /* proportioneel regelen */
    float output = error * params.kp;
    
    /* integrerend regelen */
    float iTerm;
    if ( error == 0) iTerm=0;
    iTerm += params.ki*error;
    iTerm = constrain(iTerm, -510, 510);
    output += iTerm;

    /* differentiërend regelen */
    output += params.kd * (error - lastErr);
    lastErr = error;
    
    output = constrain(output, -510, 510); 

   
    float powerLeft, powerRight;

    //SerialPort.print("output= ");
    //SerialPort.println(output);

    if (output >= 0)
      {
        powerLeft = constrain(params.power + params.diff * output, -255, 255);
        powerRight = constrain(powerLeft - output, -255, 255);
        powerLeft = powerRight + output;
      }
    else
      {
        powerRight = constrain(params.power - params.diff * output, -255, 255);
        powerLeft = constrain(powerRight + output, -255, 255);
        powerRight = powerLeft - output;
      }

    
    if(start==true){
      if (powerLeft >=0)  (analogWrite (3,abs (powerLeft)),digitalWrite (5,LOW));
      if (powerRight >=0)  (analogWrite (10,abs (powerRight)),digitalWrite(9,LOW));
      if (powerLeft <0)  (analogWrite (5,abs (powerLeft)),digitalWrite (3,LOW));
      if (powerRight <0)  (analogWrite (9,abs (powerRight)),digitalWrite(10,LOW));
 
    }else{
      digitalWrite(3, LOW);
      digitalWrite(5, LOW);
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
    }

  }
    
    unsigned long difference = micros() - current;
    if (difference > calculationTime) calculationTime = difference;
}

void onUnknownCommand(char *command)
{
  SerialPort.print("unknown command: \"");
  SerialPort.print(command);
  SerialPort.println("\"");
}

void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();  
  if (strcmp(param, "cycle") == 0)
  {
    long newCycleTime = atol(value);
    float ratio = ((float) newCycleTime) / ((float) params.cycleTime);

    params.ki *= ratio;
    params.kd /= ratio;

    params.cycleTime = newCycleTime;
  }
  else if (strcmp(param, "ki") == 0)
  {
    float sampleTimeInSec = ((float) params.cycleTime) / 1000000;
    params.ki = atof(value) * sampleTimeInSec;
  }
  else if (strcmp(param, "kd") == 0)
  {
    float sampleTimeInSec = ((float) params.cycleTime) / 1000000;
    params.kd = atof(value) / sampleTimeInSec;
  }
 
  if (strcmp(param, "power") == 0) params.power = atoi(value);
  if (strcmp(param, "kp") == 0) params.kp = atof(value);
  if (strcmp(param, "diff") == 0) params.diff = atof(value);
 

  EEPROM_writeAnything(0, params);
}

void onDebug()
{
  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);

  float sampleTimeInSec = ((float) params.cycleTime) / 1000000;
  float ki = params.ki / sampleTimeInSec;
  SerialPort.print("Ki: ");
  SerialPort.println(ki);

  float kd = params.kd * sampleTimeInSec;
  SerialPort.print("Kd: ");
  SerialPort.println(kd);
  SerialPort.print("Power : ");
  SerialPort.println(params.power);
  SerialPort.print("Kp : ");
  SerialPort.println(params.kp);
  SerialPort.print("Diff : ");
  SerialPort.println(params.diff);

  //waarden black
  SerialPort.print("black: ");
  for (int i = 0; i < 6; i++)
  {
      SerialPort.print(params.black[i]);
      SerialPort.print(" ");
  }

  //waarden white
  SerialPort.print("white: ");
  for (int i = 0; i < 6; i++)
  {
      SerialPort.print(params.white[i]);
      SerialPort.print(" ");
  }
  
  
  SerialPort.print("calculation time: ");
  SerialPort.println(calculationTime);
  calculationTime = 0;
}

void onCalibrate()
{
  char* param = sCmd.next();

  if (strcmp(param, "black") ==0 )
  {
    SerialPort.print("start calibrating black... ");
    for (int i = 0; i < 6; i++) params.black[i]=analogRead(sensor[i]);
    SerialPort.println("done");
  }
  else if (strcmp(param, "white") ==0 )
  {
    SerialPort.print("start calibrating white... ");    
    for (int i = 0; i < 6; i++) params.white[i]=analogRead(sensor[i]);  
    SerialPort.println("done");      
  }

  EEPROM_writeAnything(0, params);
}

void onStart()
{
    start = true; 
}

void onStop()
{
    start = false;
}
