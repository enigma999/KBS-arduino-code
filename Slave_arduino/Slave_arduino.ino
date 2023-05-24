//includes
#include <Wire.h>
//global variables
int directionPin[] = { 12, 13 };
int motorPin[] = { 3, 11 };
int BrakePin[] = { 9, 8 };
int motorCurrentPin[] = { A1, A0 };
int ZmovementPin = 2;
String command;
bool endofZ = true;
int Zpos = 0;

enum modes {
  VOOR,
  ACHTER,
  STOP
};
modes mode;

void setup() {
  //Initialize I2C
  Wire.begin(2);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);

  //pin setup
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ZmovementPin), countpluse, RISING);
  calibrateZ(motorPin[1], directionPin[1]);
}

void loop() {

  delay(100);
  if (command.length() > 0) {
    Serial.println(command);
  };
  handleCommand();

  switch (mode) {
    case VOOR:
      if (!endofZ) {
        digitalWrite(directionPin[1], LOW);
        digitalWrite(motorPin[1], HIGH);
      }
      break;

    case ACHTER:
      digitalWrite(directionPin[1], HIGH);
      digitalWrite(motorPin[1], HIGH);
      break;

    case STOP:
      digitalWrite(motorPin[1], LOW);
      break;

    default:
      Serial.println("no mode of opperation found");
      break;
  }
  //read wire


  //execute cmd

  //return response
}

void calibrateZ(int motorPin, int dirPin) {
  //turn on motor
  digitalWrite(dirPin, HIGH);
  digitalWrite(motorPin, HIGH);
  int oldZPos = 0;
  int start = millis();
  while (true) {
    if (millis() > start + 100) {
      if (oldZPos == Zpos) {
        break;
      } else {
        start = millis();
        oldZPos = Zpos;
      }
    }
  }
  //turn off motor
  digitalWrite(motorPin, LOW);
  start = millis();
  while (millis() < start + 100) {
  }
  Zpos = 0;
  Serial.println("calibration succesfull");
}

void countpluse() {
  if (digitalRead(directionPin[1]) == HIGH) {
    Zpos--;
  } else {
    Zpos++;
  }
}

void handleCommand() {
  if (command == "v") {
    mode = VOOR;
  }
  if (command == "a") {
    mode = ACHTER;
  }
  if (command == "s") {
    mode = STOP;
  }
  command = "";
}

void receiveEvent(int howMany) {
  command = "";
  while (0 < Wire.available()) {
    char c = Wire.read();
    command += c;
  }
}
