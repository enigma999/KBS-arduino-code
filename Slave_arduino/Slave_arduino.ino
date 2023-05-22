//includes
#include <Wire.h>
//global variables
int directionPin[] = { 12, 13 };
int motorPin[] = { 3, 11 };
int BrakePin[] = { 9, 8 };
int motorCurrentPin[] = { A1, A0 };


void setup() {
  //Initialize I2C
  Wire.begin(2);

  //pin setup

  //calibrate y

  //calibrate z
}

void loop() {
  //read wire
  String wireResponse = readWire();

  //exicute cmd

  //return response
}

String readWire() {
  String command;
  while (Wire.available()) {
    char c = Wire.read();  // Read received data
    command += c;          // Add received char to the received message
  }
  return command;
}
