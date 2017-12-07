#include <NewPing.h> //biblioteka czujnika ultradzwiekowego
#include <ELClient.h>
#include <ELClientWebServer.h>

#define TRIGGER_PIN  A1
#define ECHO_PIN     A2
#define MAX_DISTANCE 400
//#define LED_PIN A3


NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

unsigned long time;
unsigned long previousMillis = 0;
unsigned long sonarPreviousMillis = 0;
const long pumpInterval = 2000;    //900000 to 15 min. 
const long sonarInterval = 4000;

ELClient esp(&Serial);
ELClientWebServer webServer(&esp);
void ledPageLoadAndRefreshCb(char * url)
{
  if( digitalRead(A3) )
    webServer.setArgString(F("text"), F("LED is on"));
  else
    webServer.setArgString(F("text"), F("LED is off"));
}

void ledButtonPressCb(char * btnId)
{
  String id = btnId;
  if( id == F("btn_on") )
    digitalWrite(A3, HIGH);
  else if( id == F("btn_off") )
    digitalWrite(A3, LOW);
}
void resetCb(void) {
  Serial.println("EL-Client (re-)starting!");
  bool ok = false;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");
  
  webServer.setup();
}





void setup() {
Serial.begin(9600);
pinMode(A0, OUTPUT);
pinMode(A0, OUTPUT);
pinMode(A3, OUTPUT);
digitalWrite(A0, LOW);
Serial.println("koniec sekwencji startowej");

URLHandler *ledHandler = webServer.createURLHandler(F("/SimpleLED.html.json"));
ledHandler->loadCb.attach(&ledPageLoadAndRefreshCb);
ledHandler->refreshCb.attach(&ledPageLoadAndRefreshCb);
ledHandler->buttonCb.attach(&ledButtonPressCb);

esp.resetCb = resetCb;
resetCb();

}

void loop() {

esp.Process();

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
