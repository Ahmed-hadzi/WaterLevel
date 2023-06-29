// Udaljenost senzora kad nema vode
int baseLevel = 0;
// Udaljenost nakon koje se promijeni nivo vode
int levelBreak = 0;

// Nivoi vode
enum Level { red1 = 8, red2 = 9, yellow1 = 10, yellow2 = 11, green1 = 12, green2 = 13};
Level waterLevel = 0;

// Pinovi prekidaca
#define baseLevelSwitch 4
#define waterLossSwitch 5

// Pinovi senzora
#define echoPin 2
#define trigPin 3

// Podaci sa senzora
long duration;
int distance;
int sensorMax = 399; // Maksimalna udaljenost (u cm) koju senzor moze da izmjeri (maks. ~400cm)

// Water loss
int baseLossLevel = 0;
bool waterLoss = false;



void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(baseLevelSwitch, INPUT);
  pinMode(waterLossSwitch, INPUT);

  for (int i = red1; i <= green2; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  // PRIVREMENO bez prekidaca
  getBaseLevel();
}


//////////////////////////////////
// Ocitava senzor kada nema vode
// Output: (int) baseLevel, (int) levelBreak
//////////////////////////////////
int getBaseLevel() {
  baseLevel = provjeraVode();
  levelBreak = baseLevel / 6;
  Serial.print("baseLevel: ");
  Serial.print(baseLevel);
  Serial.println(" cm");
}


//////////////////////////////////
// Ocitava senzor
// Output: (int) distance
//////////////////////////////////
int provjeraVode() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  if ((distance < sensorMax) || ((baseLevel != 0) && (distance < baseLevel))) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    return (distance);
  } else {
    // Ponovo ocitava nivo vode ukoliko dodje do greske
    provjeraVode();
  }
}


//////////////////////////////////
// Pamti nivo vode
// Output: (Level) waterLevel
//////////////////////////////////
Level nivoVode() {
  switch (provjeraVode() / levelBreak) {
    case 0: case 1: return red1; break;
    case 2: return red2; break;
    case 3: return yellow1; break;
    case 4: return yellow2; break;
    case 5: return green1; break;
    case 6: return green2; break;
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
// Output: (int) baseLossLevel
//////////////////////////////////
int getBaseLossLevel() {
  baseLossLevel = provjeraVode();
  return (baseLossLevel);
}


//////////////////////////////////
// Provjerava da li postoji water loss
// Input: (int) baseLossLevel
// Output: (bool) waterLoss
//////////////////////////////////
bool waterLossCheck(int baseLossLevel) {
  if (provjeraVode() < baseLossLevel) {
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
    ledcontrol(red2);
    delay(2000);
    waterLoss = false;
    return;
  } else {
    for (int i = red1; i <= green2; i++) {
      digitalWrite(i, LOW);
    }
    for (int i = green1; i <= green2; i++) {
      digitalWrite(i, HIGH);
    }
    delay(2000);
    return;
  }
}


void loop() {

  /* Sa prekidacem
    if(digitalRead(baseLevelSwitch) == HIGH){
    getBaseLevel();
    }*/

  if (baseLevel != 0) {
    ledcontrol(nivoVode());
  }

  if (digitalRead(waterLossSwitch) == HIGH) {
    if (baseLossLevel == 0) {
      // Dobija se nivo vode prije curenja
      getBaseLossLevel();
    } else {
      // Dobija se status postojanja curenja vode (waterLoss)
      waterLoss = waterLossCheck(baseLossLevel);
      baseLossLevel = 0;
      waterLossLED(waterLoss);
    }
  }

  if (waterLoss == true) {
    // Ponavlja se ako je waterLoss == true
    Serial.println("WATER LOSS");
  }

  delay(1000);
}
