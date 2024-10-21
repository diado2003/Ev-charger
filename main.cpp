#include <Arduino.h>
// Definiții pentru pini
const int ledRGB_R = 6;
const int ledRGB_G = 5;
const int led1 = 10;
const int led2 = 9; 
const int led3 = 8;   
const int led4 = 7; 
const int butonStart = 2; 
const int butonStop = 3;  

// Parametri pentru debouncing
unsigned long ultimulDebounceTimpStart = 0;
unsigned long ultimulDebounceTimpStop = 0;
const unsigned long intarziereDebounce = 50;
bool stareButonStart = LOW;
bool stareButonStop = LOW;
bool ultimaStareButonStart = LOW;
bool ultimaStareButonStop = LOW;

// Starea stației
enum StareStatie { LIBER, OCUPAT };
StareStatie stareStatia = LIBER;

// Parametri încărcare
int nivelIncarcare = 0;
bool incarca = false;
bool oprireIncarcare = false;
unsigned long timpAnterior = 0;
const long intervalIncarcare = 3000; 

unsigned long timpUltimClipit = 0;
const long intervalClipire = 500;
bool stareLED = LOW;

void setup() {
  // Configurare pini
  pinMode(ledRGB_R, OUTPUT);
  pinMode(ledRGB_G, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(butonStart, INPUT);
  pinMode(butonStop, INPUT);

  digitalWrite(ledRGB_G, HIGH);
  digitalWrite(ledRGB_R, LOW);
}

void seteazaToateLEDurile(bool stare) {
  digitalWrite(led1, stare);
  digitalWrite(led2, stare);
  digitalWrite(led3, stare);
  digitalWrite(led4, stare);
}

void reseteazaStatia() {
  stareStatia = LIBER;
  digitalWrite(ledRGB_G, HIGH);
  digitalWrite(ledRGB_R, LOW);
  seteazaToateLEDurile(LOW);
}

void incepeIncarcare() {
  reseteazaStatia();
  incarca = true;
  nivelIncarcare = 0;
  stareStatia = OCUPAT;
  digitalWrite(ledRGB_G, LOW);
  digitalWrite(ledRGB_R, HIGH); 
}

void clipesteLEDulCurent(unsigned long timpCurent) {
  if (timpCurent - timpUltimClipit >= intervalClipire) {
    timpUltimClipit = timpCurent;
    stareLED = !stareLED;

    switch (nivelIncarcare) {
      case 1: digitalWrite(led1, stareLED); break;
      case 2: digitalWrite(led2, stareLED); break;
      case 3: digitalWrite(led3, stareLED); break;
      case 4: digitalWrite(led4, stareLED); break;
    }
  }
}

void actualizeazaIndicator(unsigned long timpCurent) {
  if (nivelIncarcare > 1) digitalWrite(led1, HIGH);
  if (nivelIncarcare > 2) digitalWrite(led2, HIGH);
  if (nivelIncarcare > 3) digitalWrite(led3, HIGH);

  if (nivelIncarcare > 0 && nivelIncarcare <= 4) {
    clipesteLEDulCurent(timpCurent);
  }
}

void finalizeazaIncarcare() {
  incarca = false;
  for (int i = 0; i < 4; i++) {
    seteazaToateLEDurile(HIGH);
    delay(500);
    seteazaToateLEDurile(LOW);
    delay(500);
  }
  reseteazaStatia();
}

void opresteIncarcare() {
  incarca = false;
  oprireIncarcare = false;
  for (int i = 0; i < 4; i++) {
    seteazaToateLEDurile(HIGH);
    delay(500);
    seteazaToateLEDurile(LOW);
    delay(500);
  }
  reseteazaStatia();
}

void loop() {
  unsigned long timpCurent = millis();

  bool citireStart = digitalRead(butonStart);
  bool citireStop = digitalRead(butonStop);

  if (citireStart != ultimaStareButonStart) {
    ultimulDebounceTimpStart = timpCurent;
  }

  if ((timpCurent - ultimulDebounceTimpStart) > intarziereDebounce) {
    if (citireStart != stareButonStart) {
      stareButonStart = citireStart;
      if (stareButonStart == HIGH && stareStatia == LIBER) {
        incepeIncarcare();
      }
    }
  }

  if (citireStop != ultimaStareButonStop) {
    ultimulDebounceTimpStop = timpCurent;
  }

  if ((timpCurent - ultimulDebounceTimpStop) > intarziereDebounce) {
    if (citireStop != stareButonStop) {
      stareButonStop = citireStop;
      if (stareButonStop == HIGH && stareStatia == OCUPAT) {
        unsigned long durataApasare = millis();
        while (digitalRead(butonStop) == HIGH) {
          if (millis() - durataApasare >= 1000) {
            oprireIncarcare = true;
            break;
          }
        }
      }
    }
  }

  if (incarca && !oprireIncarcare) {
    if (timpCurent - timpAnterior >= intervalIncarcare) {
      timpAnterior = timpCurent;

      if (nivelIncarcare < 4) {
        nivelIncarcare++;
      } else {
        finalizeazaIncarcare();
      }
    }
    actualizeazaIndicator(timpCurent);
  } else if (oprireIncarcare) {
    opresteIncarcare();
  }

  ultimaStareButonStart = citireStart;
  ultimaStareButonStop = citireStop;
}
