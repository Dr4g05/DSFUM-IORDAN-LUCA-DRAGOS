
#define LATCH_DIO 4
#define CLK_DIO 7
#define DATA_DIO 8
#define BUZZER 3

const byte SEGMENT_MAP[] = {
  0xC0, 0xF9, 0xA4, 0xB0, 0x99,
  0x92, 0x82, 0xF8, 0x80, 0x90
};

const byte SEGMENT_SELECT[] = {
  0xF1, 0xF2, 0xF4, 0xF8
};

enum Mode {
  CLOCK_MODE,
  STOPWATCH_MODE
};

Mode currentMode = CLOCK_MODE;

int clockHour = 0;
int clockMinute = 0;
bool clockInitialized = false;

int stopwatchMinutes = 0;
int stopwatchSeconds = 0;
bool stopwatchInitialized = false;
bool stopwatchRunning = false;
bool stopwatchFinished = false;

bool clockAlarmEnabled = false;
int alarmHour = 0;
int alarmMinute = 0;
int clockAlarmSong = 0;
int stopwatchAlarmSong = 0;
bool clockAlarmTriggered = false;

const int melody0[] = {
  262, 294, 330, 349, 392, 440, 392, 0
};

const int melody1[] = {
  392, 349, 330, 294, 262, 294, 262, 0
};

const int noteDuration = 250;
const int melodyLength = 8;

bool playingMelody = false;
int currentSong = 0;
int currentNote = 0;
unsigned long noteStart = 0;

unsigned long previousSecond = 0;

void writeDigit(byte position, byte value, bool decimalPoint = false) {
  byte segments = SEGMENT_MAP[value];

  if (decimalPoint) {
    segments &= 0x7F;
  }

  digitalWrite(LATCH_DIO, LOW);
  shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, segments);
  shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[position]);
  digitalWrite(LATCH_DIO, HIGH);
}

void clearDigit(byte position) {
  digitalWrite(LATCH_DIO, LOW);
  shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, 0xFF);
  shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[position]);
  digitalWrite(LATCH_DIO, HIGH);
}

void showNumber(int value, bool colon) {
  int digits[4];

  digits[0] = value / 1000;
  digits[1] = (value / 100) % 10;
  digits[2] = (value / 10) % 10;
  digits[3] = value % 10;

  static byte position = 0;

  if (position == 0 && digits[0] == 0) {
    clearDigit(position);
  } else {
    writeDigit(position, digits[position],
               colon && position == 1);
  }

  position = (position + 1) % 4;
}

void startMelody(int song) {
  if (playingMelody) return;

  currentSong = song;
  currentNote = 0;
  playingMelody = true;
  noteStart = 0;
}

void updateMelody() {
  if (!playingMelody) return;

  unsigned long now = millis();

  if (noteStart == 0 || now - noteStart >= noteDuration) {
    noTone(BUZZER);

    if (currentNote >= melodyLength) {
      playingMelody = false;
      return;
    }

    int frequency;

    if (currentSong == 0) {
      frequency = melody0[currentNote];
    } else {
      frequency = melody1[currentNote];
    }

    if (frequency > 0) {
      tone(BUZZER, frequency);
    }

    currentNote++;
    noteStart = now;
  }
}

void updateClock() {
  clockMinute++;

  if (clockMinute >= 60) {
    clockMinute = 0;
    clockHour++;

    if (clockHour >= 24) {
      clockHour = 0;
    }
  }

  if (clockAlarmEnabled &&
      clockHour == alarmHour &&
      clockMinute == alarmMinute) {

    if (!clockAlarmTriggered) {
      startMelody(clockAlarmSong);
      clockAlarmTriggered = true;
    }
  } else {
    clockAlarmTriggered = false;
  }
}

void updateStopwatch() {
  if (!stopwatchInitialized ||
      !stopwatchRunning ||
      stopwatchFinished) {
    return;
  }

  if (stopwatchSeconds > 0) {
    stopwatchSeconds--;
  } else if (stopwatchMinutes > 0) {
    stopwatchMinutes--;
    stopwatchSeconds = 59;
  }

  if (stopwatchMinutes == 0 &&
      stopwatchSeconds == 0) {

    stopwatchRunning = false;
    stopwatchFinished = true;

    startMelody(stopwatchAlarmSong);
  }
}

void processCommand(String command) {
  command.trim();

  int h, m, song;

  if (sscanf(command.c_str(), "clock %d:%d", &h, &m) == 2) {
    if (!clockInitialized &&
        h >= 0 && h < 24 &&
        m >= 0 && m < 60) {

      clockHour = h;
      clockMinute = m;
      clockInitialized = true;
      currentMode = CLOCK_MODE;

      previousSecond = millis();

      Serial.println("Clock initialized");
    } else {
      Serial.println("Clock already initialized or invalid");
    }

    return;
  }

  if (sscanf(command.c_str(), "stopwatch %d:%d", &m, &h) == 2) {
    if (!stopwatchInitialized &&
        m >= 0 && m < 100 &&
        h >= 0 && h < 60) {

      stopwatchMinutes = m;
      stopwatchSeconds = h;
      stopwatchInitialized = true;
      stopwatchRunning = true;
      stopwatchFinished = false;

      currentMode = STOPWATCH_MODE;

      previousSecond = millis();

      Serial.println("Stopwatch initialized");
    } else {
      Serial.println("Stopwatch already initialized or invalid");
    }

    return;
  }

  if (sscanf(command.c_str(),
             "alarm clock %d:%d %d",
             &h, &m, &song) == 3) {

    if (h >= 0 && h < 24 &&
        m >= 0 && m < 60 &&
        song >= 0 && song <= 1) {

      alarmHour = h;
      alarmMinute = m;
      clockAlarmSong = song;
      clockAlarmEnabled = true;
      clockAlarmTriggered = false;

      Serial.println("Clock alarm configured");
    } else {
      Serial.println("Invalid clock alarm");
    }

    return;
  }

  if (sscanf(command.c_str(),
             "alarm stopwatch %d",
             &song) == 1) {

    if (song >= 0 && song <= 1) {
      stopwatchAlarmSong = song;
      Serial.println("Stopwatch alarm configured");
    } else {
      Serial.println("Invalid song index");
    }

    return;
  }

  if (command == "clock") {
    currentMode = CLOCK_MODE;
    Serial.println("Clock mode");
    return;
  }

  if (command == "stopwatch") {
    currentMode = STOPWATCH_MODE;
    Serial.println("Stopwatch mode");
    return;
  }

  Serial.println("Unknown command");
}

void setup() {
  Serial.begin(9600);

  pinMode(LATCH_DIO, OUTPUT);
  pinMode(CLK_DIO, OUTPUT);
  pinMode(DATA_DIO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(BUZZER, HIGH);

  previousSecond = millis();

  Serial.println("Multifunction Shield Ready");
  Serial.println("clock HH:MM");
  Serial.println("stopwatch MM:SS");
  Serial.println("clock");
  Serial.println("stopwatch");
  Serial.println("alarm clock HH:MM INDEX");
  Serial.println("alarm stopwatch INDEX");
}

void loop() {
  unsigned long now = millis();

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }

  if (now - previousSecond >= 1000) {
    previousSecond += 1000;

    if (clockInitialized) {
      updateClock();
    }

    if (stopwatchInitialized) {
      updateStopwatch();
    }
  }

  if (currentMode == CLOCK_MODE && clockInitialized) {
    showNumber(clockHour * 100 + clockMinute, true);
  } else if (currentMode == STOPWATCH_MODE &&
             stopwatchInitialized) {
    showNumber(stopwatchMinutes * 100 + stopwatchSeconds, true);
  } else {
    static byte blankPosition = 0;
    clearDigit(blankPosition);
    blankPosition = (blankPosition + 1) % 4;
  }

  updateMelody();
}
