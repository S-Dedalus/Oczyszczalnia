#include <ELClient.h>
#include <ELClientWebServer.h>
#include <ELClientRest.h>
#include <Ultrasonic.h>

#define TRIGGER_PIN  A1
#define ECHO_PIN     A2
#define MAX_DISTANCE 400

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);

unsigned long time;
unsigned long previousMillis = 0;
unsigned long sonarPreviousMillis = 0;
unsigned long wyrzutPerviosMillis = 0;
const long pumpInterval = 900000;    //900000 to 15 min. 
const long sonarInterval = 1000;
int poziom_max = 40;
int poziom_min = 250;

ELClient esp(&Serial);
ELClientWebServer webServer(&esp);
ELClientRest rest(&esp);

/*void ledPageLoadAndRefreshCb(char * url)
{
  if( digitalRead(A3) )
    webServer.setArgString(F("text"), F("LED is on"));
  else
    webServer.setArgString(F("text"), F("LED is off"));
}*/

void ledButtonPressCb(char * btnId)
{
  String id = btnId;
  if( id == F("btn_on") )
    digitalWrite(A3, LOW);
  else if( id == F("btn_off") )
    digitalWrite(A3, HIGH);
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


int sprawdzPoziom() {
    int dystans = ultrasonic.distanceRead();
    Serial.println(dystans);
    int procent;
    procent = ((dystans - poziom_min)*100)/(poziom_max - poziom_min);
    Serial.print(procent);
    Serial.println("%");
    return(procent);
}

void setup() {
Serial.begin(9600);
pinMode(A0, OUTPUT);// Pompka napowietrzania, cykliczne załączanie
pinMode(A1, OUTPUT); //trigger pin 
pinMode(A2, INPUT); //Echo pin 
pinMode(A3, OUTPUT); // wyjście sterowane guzikiem z html (pompka wydzutu wody)
digitalWrite(A0, HIGH);
digitalWrite(A3, HIGH);
Serial.println("koniec sekwencji startowej");

  bool ok;
  do {
    ok = esp.Sync();      // synchronizuje esp-link, blokuje system na 2 sek.
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");

URLHandler *ledHandler = webServer.createURLHandler(F("/SimpleLED.html.json"));
//ledHandler->loadCb.attach(&ledPageLoadAndRefreshCb);
//ledHandler->refreshCb.attach(&ledPageLoadAndRefreshCb);
ledHandler->buttonCb.attach(&ledButtonPressCb);

esp.resetCb = resetCb;
resetCb();


  int err = rest.begin("192.168.1.54", 8080);
  if (err != 0) {
    Serial.print("REST begin failed: ");
 //   Serial.println(err);
    while(1) ;
  }
  Serial.println("EL-REST ready");
}

#define BUFLEN 266

void loop() {

esp.Process();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= pumpInterval) {
    previousMillis = currentMillis;
    if (digitalRead(A0) == HIGH) {
      digitalWrite(A0, LOW);
      Serial.println("wlaczam pompke");
          }
     else {
         digitalWrite(A0, HIGH);
         Serial.println("wylaczam pompke");
          }
  }
  unsigned long sonarCurrentMillis = millis();
  if (sonarCurrentMillis - sonarPreviousMillis >= sonarInterval) {
    sonarPreviousMillis = sonarCurrentMillis;
    if (sprawdzPoziom() >= 85) {
      digitalWrite(A3, LOW);
      }
  unsigned long wyrzutCurrentMillis = millis();
      if (wyrzutPerviosMillis - wyrzutCurrentMillis >= 500){
        wyrzutPerviosMillis = wyrzutCurrentMillis;
        if (sprawdzPoziom() <= 15){
          digitalWrite(A3, HIGH);
        }
      }
  
  }

}
