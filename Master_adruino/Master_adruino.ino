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
int xGoTo;
int yGoTo;
int offsetX = 1700;  // offset from the start of the rail  to the middle of the first cell
int cellwidth = 700;
int offsetY = 500;  // offset from the bottom to the pickuphight for the first pakkage
int cellHeight = 50;
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

  xGoTo=calculateXpos(0);
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
  //zet alle nodige pinmodes
  //stoplicht leds
  pinMode(LED[0], OUTPUT);
  pinMode(LED[1], OUTPUT);
  pinMode(LED[2], OUTPUT);
  //x as motor
  pinMode(motorPin[0], OUTPUT);
  pinMode(directionPin[0], OUTPUT);
}

void countpluse() {
  //deze functie zorgt er voor dat de xpos altijd door de encoder wordt bijgehouden
  if (digitalRead(directionPin[0]) == HIGH) {
    Xpos--;
  } else {
    Xpos++;
  }
}


void loop() {

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
  //dit is de mode voor de noodstop deze zet alle motoren uit
  digitalWrite(motorPin[0], LOW);
  sendWire("YSZS");
}

void manualLoop() {
  //dit is de mode voor handmatige besturing
  String wireMessage = "";
  //X
  if (joystickX < 400) {
    digitalWrite(directionPin[0], HIGH);
    digitalWrite(motorPin[0], HIGH);
  } else if (joystickX > 800) {
    digitalWrite(directionPin[0], LOW);
    digitalWrite(motorPin[0], HIGH);
  } else {
    digitalWrite(motorPin[0], LOW);
  }
  //Y
  if (joystickY < 400) {
    wireMessage.concat("Y-");
  } else if (joystickY > 800) {
    wireMessage.concat("Y+");
  } else {
    wireMessage.concat("YS");
  }
  // Z
  // if (joystickX < 400) {
  //   wireMessage.concat("Z-");
  // } else if (joystickX > 800) {
  //   wireMessage.concat("Z+");
  // } else {
  //   wireMessage.concat("ZS");
  // }
  wireMessage.concat("ZS");
  sendWire(wireMessage);
}

void automaticLoop() {
  if (xGoTo > Xpos) {
    digitalWrite(directionPin[0], LOW);
    digitalWrite(motorPin[0], HIGH);
  } else if (xGoTo < Xpos) {
    digitalWrite(directionPin[0], HIGH);
    digitalWrite(motorPin[0], HIGH);
  } else {
    digitalWrite(motorPin[0], LOW);
  }
}


void SerialDebugger() {
  Serial.print("Xpos: ");
  Serial.println(Xpos);
}

void readSerial() {
  //leest de serialcomm uit en stopt het binnenkomende command in de command variable
  if (Serial.available() > 0) {
    command = Serial.readString();
    command.trim();

    // String* commandArr = split(command);
    // if (commandArr[0] == "c") {
    //   command = commandArr[0];
    //   xGoTo = commandArr[1].toInt();
    //   yGoTo = commandArr[2].toInt();
    // } else {
    //   command = commandArr[0];
    // }
    // Serial.println(command);
  }
}


//split een string met spaties en zet deze in een array. Als er geen spaties zijn heb je een array van lengte 1 met de string
String* split(String str) {
  String strs[3];
  int StringCount = 0;
  while (str.length() > 0) {
    int index = str.indexOf(' ');
    if (index == -1)  //geen spatie
    {
      strs[StringCount++] = str;
      break;
    } else {
      strs[StringCount++] = str.substring(0, index);
      str = str.substring(index + 1);
    }
  }
  return strs;
}

void handleSerialResponse() {
  //leest de command variable en handelt deze af
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

void sendWire(String message) {
  //verzend een string naar de adruino met adress 2
  Wire.beginTransmission(2);
  Wire.write(message.c_str());
  Wire.endTransmission();
}

int calculateXpos(int coordinaat) {
  //geeft de encoder Xpositie voor een gegeven coordinaat
  int positie = offsetX + (cellwidth * (coordinaat));
  return positie;
}

int calculateYpos(int coordinaat) {
  //geeft de encoder Ypositie voor een gegeven coordinaat
  int positie = offsetY + (cellHeight * (coordinaat));
  return positie;
}
