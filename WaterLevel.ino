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

  // Privremeno bez prekidaca
  getBaseLevel();
}


//////////////////////////////////
// Ocitava senzor kada nema vode
// Output: (int) baseLevel, (int) levelBreak
//////////////////////////////////
int getBaseLevel() {
  baseLevel = provjeraVode();
  Serial.print("baseLevel: ");
  Serial.print(baseLevel);
  Serial.println(" cm");
  levelBreak = baseLevel / 6;
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
  if ((distance < 350) || ((baseLevel != 0) && (distance < baseLevel))) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    return (distance);
  } else if (distance > 350) {
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
// Input: (Level) nivoVode()
//////////////////////////////////
int ledcontrol(Level nivoVode) {
  for (int i = red1; i <= green2; i++) {
    digitalWrite(i, LOW);
  }
  switch (nivoVode) {
    case red1: digitalWrite(red1, HIGH); break;
    case red2: digitalWrite(red2, HIGH); break;
    case yellow1: digitalWrite(yellow1, HIGH); break;
    case yellow2: digitalWrite(yellow2, HIGH); break;
    case green1: digitalWrite(green1, HIGH); break;
    case green2: digitalWrite(green2, HIGH); break;
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
  // Dodaje 1 cm trenutnom nivou vode da bi se smanjila mogucnost greske
  if (provjeraVode() + 1 < baseLossLevel) {
    return (true);
  } else {
    return (false);
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

  if ((digitalRead(waterLossSwitch) == HIGH) {
    if (baseLossLevel == 0) {
        getBaseLossLevel();
    } else {
        waterLoss = waterLossCheck(baseLossLevel);
        baseLossLevel = 0;
        Serial.println("WATER LOSS");
     }
  }

  delay(1000);
}