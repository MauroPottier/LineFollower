

#include "SerialCommand.h"

#include <EEPROM.h>
#include "EEPROMAnything.h"

#define SerialPort Serial

#define Baudrate 11520

SerialCommand sCmd(SerialPort);
bool debug;
unsigned long previous, calculationTime;
const int sensor [] = {A5, A4, A3, A2, A1, A0};         
struct param_t

{
  unsigned long getal;
  unsigned long cycleTime;
  int black[6];
  int white[6];
} params;

void setup()
{
  SerialPort.begin(Baudrate);

  sCmd.addCommand("set", onSet);
  sCmd.addCommand("debug", onDebug);
  
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM_readAnything(0, params);
  SerialPort.println("ready");
}

void loop()
{
  sCmd.readSerial();
 
  unsigned long current = micros();
  if (current - previous >= params.cycleTime)
  {
    previous = current;

    SerialPort.print("Values");
    for (int i = 0; i < 6; i++)
    {
      SerialPort.print(analogRead(sensor[i]));
      SerialPort.print(" ");
    }
    SerialPort.println();
  }  
  unsigned long difference = micros() - current;
  if (difference > calculationTime) 
  {
    calculationTime = difference;
  }
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

  EEPROM_writeAnything(0, params);
}

void onDebug()
{
  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);

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
  SerialPort.print(calculationTime);
  calculationTime = 0;
}
