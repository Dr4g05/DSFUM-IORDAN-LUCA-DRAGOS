const int latchPin = 4;
const int clockPin = 7;
const int dataPin = 8;

const int btnA1 = A1;
const int btnA2 = A2;
const int btnA3 = A3;

byte digits[10] = {
  0b00000011, // 0
  0b11110011, // 1
  0b00100101, // 2
  0b01100001, // 3
  0b11010001, // 4
  0b01001001, // 5
  0b00001001, // 6
  0b11100011, // 7
  0b00000001, // 8
  0b01000001  // 9
};

byte positions[4] = {
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
};

const byte l_A = 0b10000001;
const byte l_b = 0b00011001;
const byte l_C = 0b00001111;
const byte l_d = 0b00110001;
const byte l_E = 0b00001101;
const byte l_F = 0b10001101;
const byte l_L = 0b00011111;
const byte l_i = 0b11111011;
const byte l_J = 0b00110011;
const byte l_O = 0b00000011;
const byte l_P = 0b10000101;
const byte l_S = 0b01001001;
const byte l_U = 0b00010011;
const byte l__ = 0b01111111;
const byte space = 0b11111111;

const int N = 3;
unsigned long records[3] = {0};
int numbers[N] = {0};
int iteration = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  
  pinMode(btnA1, INPUT);
  pinMode(btnA2, INPUT);
  pinMode(btnA3, INPUT);
}

void loop() {
  ex4();
}

void afiseazaValoare(byte valoare, byte segment) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, valoare);
  shiftOut(dataPin, clockPin, LSBFIRST, segment);
  digitalWrite(latchPin, HIGH);
}

void ex1() {
  for (int i = 0; i < 10; i++) {
    afiseazaValoare(digits[i], 0b10000000);
    delay(1000);
  }
}

void ex2() {
  if (iteration >= N) return;

  String command = "";
  int numarCitit = -1;

  int nr = random(1000, 10000);

  int cifre[4] = {
    nr % 10,
    nr / 10 % 10,
    nr / 100 % 10,
    nr / 1000
  };

  unsigned long start = millis();

  while (numarCitit != nr) {
    for (int i = 0; i < 4; i++) {
      afiseazaValoare(digits[cifre[i]], positions[i]);
      delay(2);
    }

    while (Serial.available()) {
      char c = Serial.read();

      if (c == '\n' || c == '\r') {
        if (command.length() > 0) {
          numarCitit = command.toInt();
          command = "";

          if (numarCitit != nr) {
            Serial.println("Wrong");
          }
        }
      } else {
        command += c;
      }
    }
  }

  records[iteration] = millis() - start;
  numbers[iteration] = nr;

  Serial.print("Timp: ");
  Serial.print(records[iteration]);
  Serial.println(" ms");

  iteration++;

  if (iteration == N) {
    statistici();
  }
}

void statistici() {
  unsigned long minT = records[0];
  unsigned long maxT = records[0];
  unsigned long suma = 0;

  int minNr = numbers[0];
  int maxNr = numbers[0];

  for (int i = 0; i < N; i++) {
    suma += records[i];

    if (records[i] < minT) {
      minT = records[i];
      minNr = numbers[i];
    }

    if (records[i] > maxT) {
      maxT = records[i];
      maxNr = numbers[i];
    }
  }

  Serial.print("Minim: ");
  Serial.print(minT);
  Serial.print(" ms, numar: ");
  Serial.println(minNr);

  Serial.print("Maxim: ");
  Serial.print(maxT);
  Serial.print(" ms, numar: ");
  Serial.println(maxNr);

  Serial.print("Media: ");
  Serial.print(suma / (float)N);
  Serial.println(" ms");
}

int c = 0;
bool lastA1 = LOW, lastA2 = LOW, lastA3 = LOW;

void ex3() {
  bool currentA1 = digitalRead(btnA1);
  bool currentA2 = digitalRead(btnA2);
  bool currentA3 = digitalRead(btnA3);

  if (currentA1 && !lastA1 && c < 9999) c++;
  if (currentA2 && !lastA2 && c > 0) c--;
  if (currentA3 && !lastA3) c = 0;

  lastA1 = currentA1; lastA2 = currentA2; lastA3 = currentA3;

  if (c == 0) {
    afiseazaValoare(digits[0], positions[3]);
    delay(2);
  } else {
    int temp = c;
    int pos = 3;
    while(temp > 0 && pos >= 0) {
      afiseazaValoare(digits[temp % 10], positions[pos]);
      temp /= 10;
      pos--;
      delay(2);
    }
  }
}

byte litereEx4[] = {l_A, l_b, l_C, l_d, l_E, l_F};

void ex4() {
  for (int i = 0; i < 6; i++) {
    afiseazaValoare(litereEx4[i], positions[0]);
    delay(1000); 
  }
}

byte scrollText[] = {
  space, space, space, space, 
  l_d, l_E, l_C, l_E, l_b, l_A, l_L, l__, l_S, l_i, l__, l_F, l_i, l_U, l_L, l__, l_S, l_A, l_U, l__,
  l_L, l_O, l_C, l_U, l_i, l_E, l_S, l_C, l__, l_P, l_E, l__, l_P, l_L, l_A, l_i, l_U, l_L, l__,
  l_d, l_A, l_C, l_i, l_E, l_i,
  space, space, space, space 
};

unsigned long lastScrollTime = 0;
int scrollIndex = 0;
bool isPaused = false;
bool lastAnyBtn = LOW;

void ex5() {
  bool currentAnyBtn = digitalRead(btnA1) || digitalRead(btnA2) || digitalRead(btnA3);

  if (currentAnyBtn && !lastAnyBtn) {
    isPaused = !isPaused;
  }
  lastAnyBtn = currentAnyBtn;

  if (!isPaused && (millis() - lastScrollTime > 300)) {
    scrollIndex++;
    if (scrollIndex > sizeof(scrollText) - 4) {
      scrollIndex = 0; 
    }
    lastScrollTime = millis();
  }

  for (int i = 0; i < 4; i++) {
    afiseazaValoare(scrollText[scrollIndex + i], positions[i]);
    delay(2); 
  }
}