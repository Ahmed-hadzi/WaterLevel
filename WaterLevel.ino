#include <EEPROM.h>

// Udaljenost senzora kad nema vode
int baseLevel = 0;
// Udaljenost nakon koje se promijeni nivo vode
int levelBreak = 0;

// Nivoi vode
enum Level { off, red1 = 8, red2 = 9, yellow1 = 10, yellow2 = 11, green1 = 12, green2 = 13};
Level waterLevel = 0;

// Pinovi prekidaca
#define resetSwitch 4
#define waterLossSwitch 5

// Pinovi senzora
#define echoPin 2
#define trigPin 3

// Podaci sa senzora
int distance;
int sensorMax = 23300; // Maksimalna udaljenost koju senzor moze da izmjeri (4 metra / brzina zvuka) - mikrosekunde

// Water loss
bool waterLoss = false;



void setup() {
  Serial.begin(9600);

  // Pinovi senzora
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Prekidaci
  pinMode(resetSwitch, INPUT);
  pinMode(waterLossSwitch, INPUT);

  // Postavlja LED pinove na OUTPUT i gasi LED diode
  for (int i = red1; i <= green2; i++) {
    pinMode(i, OUTPUT);
  }
  ledcontrol(off);

  // Uzima levelBreak iz EEPROM memorije
  levelBreak = getLevelBreak();
}

//////////////////////////////////
// Ocitava levelBreak iz EEPROM memorije
// Output: (int) levelBreak
//////////////////////////////////
int getLevelBreak() {
  byte byte1 = EEPROM.read(1);
  byte byte2 = EEPROM.read(2);
  return (byte1 << 8) + byte2;
}

//////////////////////////////////
// Ocitava senzor kada je prazan rezervoar
// Output: (int) levelBreak
//////////////////////////////////
int reset() {
  baseLevel = provjeraVode();
  levelBreak = baseLevel / 6;
  resetBaseLossLevel();
  resetLedControl();

  EEPROM.write(1, levelBreak >> 8);
  EEPROM.write(2, levelBreak & 0xFF);

  Serial.print("baseLevel: ");
  Serial.print(baseLevel);
  Serial.println(" microseconds");

  return (levelBreak);
}

void resetLedControl() {
  for (int i = 0; i < 3; i++) {
    ledcontrol(off);
    delay(200);
    ledcontrol(green2);
    delay(200);
  }
}


//////////////////////////////////
// Ocitava senzor i racuna udaljenost vode
// Output: (int) distance
//////////////////////////////////
int provjeraVode() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  distance = pulseIn(echoPin, HIGH);

  if ((distance < sensorMax) || ((baseLevel != 0) && (distance < baseLevel))) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" microseconds");
    return (distance);
  } else {
    // Ponovo ocitava nivo vode ukoliko dodje do greske
    provjeraVode();
  }
}


//////////////////////////////////
// Odredjuje nivo vode (6 nivoa)
// Input: (int) distance - udaljenost vode
// Output: (Level) waterLevel
//////////////////////////////////
Level nivoVode(int distance) {
  switch (distance / levelBreak) {
    case 0: case 1: return green2; break;
    case 2: return green1; break;
    case 3: return yellow2; break;
    case 4: return yellow1; break;
    case 5: return red2; break;
    case 6: return red1; break;
  }
}


//////////////////////////////////
// Kontrolise LED diode
// Input: (Level) nivoVode
//////////////////////////////////
void ledcontrol(Level nivoVode) {
  for (int i = red1; i <= green2; i++) {
    digitalWrite(i, LOW);
  }
  switch (nivoVode) {
    case off:
      break;
    case red1:
      digitalWrite(red1, HIGH);
      break;
    case red2:
      for (int i = red1; i <= red2; i++) {
        digitalWrite(i, HIGH);
      }
      break;
    case yellow1:
      for (int i = red1; i <= yellow1; i++) {
        digitalWrite(i, HIGH);
      }
      break;
    case yellow2:
      for (int i = red1; i <= yellow2; i++) {
        digitalWrite(i, HIGH);
      }
      break;
    case green1:
      for (int i = red1; i <= green1; i++) {
        digitalWrite(i, HIGH);
      }
      break;
    case green2:
      for (int i = red1; i <= green2; i++) {
        digitalWrite(i, HIGH);
      }
      break;
  }
}


//////////////////////////////////
// Pamti trenutni nivo vode za water loss check
//////////////////////////////////
void getBaseLossLevel() {
  int baseLossLevel = provjeraVode();

  EEPROM.write(10, baseLossLevel >> 8);
  EEPROM.write(11, baseLossLevel & 0xFF);

  for (int i = 0; i < 2; i++) {
    ledcontrol(off);
    delay(200);
    ledcontrol(green2);
    delay(200);
  }
}

//////////////////////////////////
// Resetuje baseLossLevel u EEPROM-u na 0
//////////////////////////////////
void resetBaseLossLevel() {
  EEPROM.write(10, 0 >> 8);
  EEPROM.write(11, 0 & 0xFF);
}

//////////////////////////////////
// Cita baseLossLevel iz EEPROM memorije
// Output: (int) baseLossLevel
//////////////////////////////////
int readBaseLossLevel() {
  byte byte1 = EEPROM.read(10);
  byte byte2 = EEPROM.read(11);
  return (byte1 << 8) + byte2;
}

//////////////////////////////////
// Provjerava da li postoji water loss
// Input: (int) baseLossLevel
// Output: (bool) waterLoss
//////////////////////////////////
bool waterLossCheck(int baseLossLevel) {

  if (provjeraVode() > baseLossLevel) {
    // Ako je trenutna udaljenost vode veca od memorisane
    return (true);
  } else {
    return (false);
  }
}

//////////////////////////////////
// Kontrolise LED diode da prikaze postojanje water loss-a
// Input: (bool) waterLoss
//////////////////////////////////
void waterLossLED(bool waterLoss) {
  if (waterLoss) {
    ledcontrol(off);
    delay(500);
    ledcontrol(red2);
    delay(1000);

    waterLoss = false;
    return;
    // Ako postoji waterLoss, pale se crvene LED diode na 2 sekunde
  } else {
    ledcontrol(off);
    delay(500);

    for (int i = green1; i <= green2; i++) {
      digitalWrite(i, HIGH);
    }
    delay(1000);
    return;
    // Ako ne postoji waterLoss, pale se zelene LED diode na 2 sekunde
  }
}


void loop() {

  if (digitalRead(resetSwitch) == HIGH) {
    // Resetuje baseLevel, levelBreak, baseLossLevel
    reset();
  }

  if (levelBreak != 0) {
    // Provjerava udaljenost vode, odredjuje nivo i pali LED diode
    ledcontrol(nivoVode(provjeraVode()));
  }

  if (digitalRead(waterLossSwitch) == HIGH) {
    if (readBaseLossLevel() == 0) {
      // Dobija se nivo vode prije potencijalnog curenja
      getBaseLossLevel();

    } else {
      // Dobija se status postojanja curenja vode (waterLoss)
      waterLoss = waterLossCheck(readBaseLossLevel());
      resetBaseLossLevel();

      // Kontrolise LED diode sukladno waterLoss-u
      waterLossLED(waterLoss);
    }
  }

  delay(1000);
}
