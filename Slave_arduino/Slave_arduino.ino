//Includes
#include <Wire.h>

//Global variables
int directionPin[] = { 12, 13 };
int motorPin[] = { 3, 11 };
int BrakePin[] = { 9, 8 };
int motorCurrentPin[] = { A1, A0 };
int YmovementPin = 2;
int sensorPin = A2;
String command="";
bool debug = true;
int Ypos = 0;
int Zpos = 0;
enum modes {
  VOOR,
  ACHTER,
  STOP
};
modes ymode = STOP;
modes zmode = STOP;

void setup() {
  //Initialize I2C
  Wire.begin(2);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);

  //Pin setup
  pinSetup();

  //Setup interrupt for Y 
  attachInterrupt(digitalPinToInterrupt(YmovementPin), countpluse, RISING);
  calibrateZ(motorPin[1], directionPin[1]);
  calibrateY(motorPin[0], directionPin[0]);
}

void calibrateY(int motorPin, int dirPin) {
  //Turn on motor
  digitalWrite(dirPin, HIGH);
  digitalWrite(motorPin, HIGH);
  int oldYPos = 0;
  int start = millis();
  while (true) {
    if (millis() > start + 100) {
      if (oldYPos == Ypos) {
        break;
      } else {
        start = millis();
        oldYPos = Ypos;
      }
    }
  }
  //Turn off motor
  digitalWrite(motorPin, LOW);
  start = millis();
  while (millis() < start + 100) {
  }
  Ypos = 0;
  Serial.println("calibration succesfull");
}

void calibrateZ(int motorPin, int dirPin) {
  while (readSensor() < 620) {
    digitalWrite(dirPin, HIGH);
    digitalWrite(motorPin, HIGH);
  }
}

void countpluse() {
  if (digitalRead(directionPin[0]) == HIGH) {
    Ypos--;
  } else {
    Ypos++;
  }
}

void receiveEvent(int howMany) {
  command = "";
  while (0 < Wire.available()) {
    char c = Wire.read();
    command += c;
  }
}

void loop() {
  handleCommand();
  sendWire(String(Ypos));

  switch (ymode) {
    case VOOR:
      digitalWrite(directionPin[0], LOW);
      digitalWrite(motorPin[0], HIGH);
      break;

    case ACHTER:
      digitalWrite(directionPin[0], HIGH);
      digitalWrite(motorPin[0], HIGH);
      break;

    case STOP:
      digitalWrite(motorPin[0], LOW);
      break;

    default:
      Serial.println("no mode of operation found for y-axis");
    
      break;
  }

  switch (zmode) {
    case VOOR:
      if (readSensor() > 370) {
        digitalWrite(directionPin[1], LOW);
        digitalWrite(motorPin[1], HIGH);
      } else {
        digitalWrite(motorPin[1], LOW);
      }
      break;

    case ACHTER:
      if (readSensor() < 620) {
        digitalWrite(directionPin[1], HIGH);
        digitalWrite(motorPin[1], HIGH);
      } else {
        digitalWrite(motorPin[1], LOW);
      }
      break;

    case STOP:
      digitalWrite(motorPin[1], LOW);
      break;

    default:
      Serial.println("no mode of operation found for z-axis");
      break;
  }
  //debug
   if (debug == true) {
    SerialDebugger();
  }
}

void pinSetup(){
  pinMode(motorPin[1], OUTPUT);
  pinMode(directionPin[1], OUTPUT);
}

void handleCommand() {
  String Y = command.substring(0, 2);
  //Serial.println(Y);
  String Z = command.substring(2, 4);
  //Serial.println(Z);
  if (Y == "Y+") {
    ymode = VOOR;
  }
  if (Y == "Y-") {
    ymode = ACHTER;
  }
  if (Y == "YS") {
    ymode = STOP;
  }
  if (Z == "Z+") {
    zmode = VOOR;
  }
  if (Z == "Z-") {
    zmode = ACHTER;
  }
  if (Z == "ZS") {
    zmode = STOP;
  }
  command = "";
}

int readSensor() {
  return analogRead(sensorPin);
}

void SerialDebugger() {
  if (command.length() > 0) {
    Serial.println(command);
    Serial.println(Ypos);
  }
}

void sendWire(String message) {
  //verzend een string naar de adruino met adress 2
  Wire.beginTransmission(1);
  Wire.write(message.c_str());
  Wire.endTransmission();
}