char stareCurenta = '0';
long previousMillis = 0;
int ledState = LOW; 
long interval = 1000;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN , OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (Serial.available() > 0) {
    char s = Serial.read();
    
    
  }

  if(stareCurenta == '3') {
    if(currentMillis - previousMillis >= interval) {
       previousMillis = currentMillis; 
       
       if (ledState == LOW) {
          ledState = HIGH;
       } else {
          ledState = LOW;
       }
       
       digitalWrite(LED_BUILTIN, ledState);
    }
  }
}