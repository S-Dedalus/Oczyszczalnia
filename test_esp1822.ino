#include <NewPing.h>

#define TRIGGER_PIN  A1
#define ECHO_PIN     A2
#define MAX_DISTANCE 400

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

unsigned long time;
unsigned long previousMillis = 0;
unsigned long sonarPreviousMillis = 0;
const long pumpInterval = 2000;    //900000 to 15 min. 
const long sonarInterval = 4000;

void setup() {
Serial.begin(9600);
pinMode(A0, OUTPUT);
pinMode(A0, OUTPUT);
digitalWrite(A0, LOW);
Serial.println("koniec sekwencji startowej");

}

void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= pumpInterval) {
    previousMillis = currentMillis;
    if (digitalRead(A0) == LOW) {
      digitalWrite(A0, HIGH);
      Serial.println("wlaczam pompke");
          }
          else {
            digitalWrite(A0, LOW);
            Serial.println("wylaczam pompke");
          }
  }
  unsigned long sonarCurrentMillis = millis();
  int pingMs = 0;
  int distance = 0;
  if (sonarCurrentMillis - sonarPreviousMillis >= sonarInterval) {
    sonarPreviousMillis = sonarCurrentMillis;
    Serial.print("Ping: ");
    pingMs = sonar.ping_median(8);
    distance = sonar.convert_cm(pingMs);
    Serial.println("cm");
  }

}
