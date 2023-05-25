//includes
#include <Wire.h>

//global variables
int directionPin[] = { 12, 13 };
int motorPin[] = { 3, 11 };
int BrakePin[] = { 9, 8 };
int motorCurrentPin[] = { A1, A0 };
int LED[] = { 4, 5, 6 };
int XmovementPin = 2;
int Xpos = 0;
int joystickY;
int joystickX;
bool debug = false;
String wireResponse;
String command;
enum modes {
  STOP,
  MAN,
  AUTO
};
modes mode = MAN;

void setup() {
  //Initialize I2C
  Wire.begin(1);

  //Initialize serial
  Serial.begin(9600);
  Serial.setTimeout(10);
  TCCR2B = TCCR2B & B11111000 | B00000111;  // for PWM frequency of 30.64 Hz

  //setup interupt for X
  attachInterrupt(digitalPinToInterrupt(XmovementPin), countpluse, RISING);

  //pin setup
  pinSetup();

  //calibrate X
  calibrateX(motorPin[0], directionPin[0]);
}

void calibrateX(int motorPin, int dirPin) {
  //turn on motor
  digitalWrite(dirPin, HIGH);
  digitalWrite(motorPin, HIGH);
  int oldXPos = 0;
  int start = millis();
  while (true) {
    if (millis() > start + 100) {
      if (oldXPos == Xpos) {
        break;
      } else {
        start = millis();
        oldXPos = Xpos;
      }
    }
  }
  //turn off motor
  digitalWrite(motorPin, LOW);
  start = millis();
  while (millis() < start + 100) {
  }
  Xpos = 0;
  Serial.println("calibration succesfull");
}

void pinSetup() {
  pinMode(motorPin[1], OUTPUT);
  pinMode(LED[0], OUTPUT);
  pinMode(LED[1], OUTPUT);
  pinMode(LED[2], OUTPUT);
  pinMode(directionPin[1], OUTPUT);
  pinMode(motorPin[0], OUTPUT);
  pinMode(directionPin[0], OUTPUT);
  pinMode(7, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
}

void countpluse() {
  if (digitalRead(directionPin[1]) == HIGH) {
    Xpos--;
  } else {
    Xpos++;
  }
}


void loop() {
  //clear comunication
  //read wire

  readSerial();
  handleSerialResponse();
  //read joystick input
  joystickY = analogRead(A3);
  joystickX = analogRead(A2);



  switch (mode) {
    case STOP:
      ModeLED(3);
      stopLoop();
      break;

    case MAN:
      ModeLED(2);
      pinMode(LED[1], HIGH);
      manualLoop();
      break;

    case AUTO:
      ModeLED(1);
      automaticLoop();
      break;

    default:
      Serial.println("no mode of opperation found");
      break;
  }

  if (debug == true) {
    SerialDebugger();
  }
}

void ModeLED(int modeNumber) {
  switch (modeNumber) {
    case 1:
      digitalWrite(LED[0], HIGH);
      digitalWrite(LED[1], LOW);
      digitalWrite(LED[2], LOW);
      break;
    case 2:
      digitalWrite(LED[0], LOW);
      digitalWrite(LED[1], HIGH);
      digitalWrite(LED[2], LOW);
      break;
    case 3:
      digitalWrite(LED[0], LOW);
      digitalWrite(LED[1], LOW);
      digitalWrite(LED[2], HIGH);
      break;
  }
}

void stopLoop() {
}

void manualLoop() {
String  wireMessage="";
  //X
  // if (joystickX < 400) {
  //   digitalWrite(directionPin[0], HIGH);
  //   digitalWrite(motorPin[0], HIGH);
  // } else if (joystickX > 800) {
  //   digitalWrite(directionPin[0], LOW);
  //   digitalWrite(motorPin[0], HIGH);
  // } else {
  //   digitalWrite(motorPin[0], LOW);
  // }
  //Y
  if (joystickY < 400) {
    wireMessage.concat("Y-");
  } else if (joystickY > 800) {
    wireMessage.concat("Y+");
  } else {
    wireMessage.concat("YS");
  }
  // Z
   if (joystickX < 400) {
    wireMessage.concat("Z-");
  } else if (joystickX > 800) {
    wireMessage.concat("Z+");
  } else {
    wireMessage.concat("ZS");
  }
  sendWire(wireMessage);
}

void automaticLoop() {
}


void SerialDebugger() {
  Serial.print("Xpos: ");
  Serial.println(Xpos);
}

void readSerial() {

  if (Serial.available() > 0) {
    command = Serial.readString();
    command.trim();
    Serial.println(command);
  }
}

void handleSerialResponse() {
  if (command == "n") {
    mode = STOP;
    pinMode(LED[0], LOW);
    pinMode(LED[1], LOW);
    pinMode(LED[2], HIGH);
  }
  if (command == "h") {
    mode = MAN;
    pinMode(LED[0], LOW);
    pinMode(LED[1], HIGH);
    pinMode(LED[2], LOW);
  }
  if (command == "a") {
    mode = AUTO;
    pinMode(LED[0], HIGH);
    pinMode(LED[1], LOW);
    pinMode(LED[2], LOW);
  }
  command = "";
}


void sendData(String message) {
  while (message.length() < 30) {
    message += ' ';
  }
  Wire.write(message.c_str(), message.length());  // Send the message to the master
}

void sendWire(String message) {
  Wire.beginTransmission(2);
  Wire.write(message.c_str());
  Wire.endTransmission();
}
