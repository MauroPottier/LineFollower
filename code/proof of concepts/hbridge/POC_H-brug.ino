#include "SerialCommand.h"

#include <EEPROM.h>
#include "EEPROMAnything.h"

#define SerialPort Serial1

#define Baudrate 9600

SerialCommand sCmd(SerialPort);
bool debug;
unsigned long previous, calculationTime;
struct param_t
{
  bool start;
  unsigned long cycleTime;
  int power;
  bool left;
  bool right;
  bool reverse;
  
} params;

void setup()
{
  SerialPort.begin(Baudrate);

  pinMode(3, OUTPUT);  //A1
  pinMode(5, OUTPUT);  //A2
  pinMode(9, OUTPUT);  //B2
  pinMode(10, OUTPUT); //B1

  sCmd.addCommand("set", onSet);
  sCmd.addCommand("start", onStart);
  sCmd.addCommand("stop", onStop);
  sCmd.addCommand("left", onLinks);
  sCmd.addCommand("right", onRechts);
  sCmd.addCommand("reverse", onReverse);
  sCmd.addCommand("debug", onDebug);
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM_readAnything(0, params);
  params.start = false;
  SerialPort.println("ready");
}

void loop()
{
  sCmd.readSerial();
 
  unsigned long current = micros();
  if (current - previous >= params.cycleTime)
  {
    previous = current;
  }
 int speed = params.power;
    if(params.start){
      if(params.left){
          analogWrite(3, abs(speed));
          digitalWrite(5, LOW);
          digitalWrite(10, LOW);
          digitalWrite(9, LOW);
      }
      else if(params.right){
          digitalWrite(3, LOW);
          digitalWrite(5, LOW);
          analogWrite(9, abs(speed));
          digitalWrite(10, LOW);
      }
      else if(params.reverse){
          analogWrite(5, abs(speed));
          digitalWrite(3, LOW);
          analogWrite(9, abs(speed));
          digitalWrite(10, LOW);
      }else{
          analogWrite(3, abs(speed));
          digitalWrite(5, LOW);
          analogWrite(10, abs(speed));
          digitalWrite(9, LOW);
      }
    }else{
      digitalWrite(3, LOW);
      digitalWrite(5, LOW);
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
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
    params.cycleTime = atol(value);
  }
  if (strcmp(param, "power") == 0) 
  {
    params.power = atoi(value);
    
  }
  SerialPort.println("done");
  
  EEPROM_writeAnything(0, params);
}

void onDebug()
{
  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);
  SerialPort.print("Aan : ");
  SerialPort.println(params.start);
  SerialPort.print("Power : ");
  SerialPort.println(params.power); 
  SerialPort.print("calculation time: ");
  SerialPort.print(calculationTime);
  calculationTime = 0;
}

void onStart()

{
    params.start = true;
    
}

void onStop()

{
    params.start = false;
    params.left = false;
    params.right = false;
    params.reverse =false;
}
void onLinks()
{
  params.left = true;
  params.reverse = false;
  params.right = false;
  
}
void onRechts()
{
  params.right = true;
  params.left = false;
  params.reverse = false;
}
void onReverse()
{
  params.reverse = true;
  params.left = false;
  params.right = false;
}


   
