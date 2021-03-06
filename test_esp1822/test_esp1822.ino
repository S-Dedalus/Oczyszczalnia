#include <ELClient.h>
#include <ELClientWebServer.h>
#include <ELClientRest.h>
#include <Ultrasonic.h>
#include <SoftwareSerial.h>

#define TRIGGER_PIN  A1
#define ECHO_PIN     A2
#define MAX_DISTANCE 400
#define WATER_PUMP  A3
#define PUMP_CHECK  A4
#define BUFLEN 266
#define DEBUG_DIS

//Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
SoftwareSerial mySerial(ECHO_PIN, TRIGGER_PIN); //inicjalizuje SoftwareSerial dla 3 trybu pracy czujnika

unsigned long time;
unsigned long previousMillis = 0;
unsigned long sonarPreviousMillis = 0;
unsigned long wyrzutPerviosMillis = 0;
const long pumpInterval = 900000;    //900000 to 15 min. 
const long sonarInterval = 20000;
int poziom_max = 400; //w milimetrach
int poziom_min = 2500; //w milimetrach

ELClient esp(&Serial, &Serial);
ELClientWebServer webServer(&esp);
ELClientRest rest(&esp);

/*void ledPageLoadAndRefreshCb(char * url)
{
  if( digitalRead(WATER_PUMP) )
    webServer.setArgString(F("text"), F("LED is on"));
  else
    webServer.setArgString(F("text"), F("LED is off"));
}

void ledButtonPressCb(char * btnId)
{
  String id = btnId;
  if( id == F("btn_on") )
    digitalWrite(WATER_PUMP, LOW);
  else if( id == F("btn_off") )
    digitalWrite(WATER_PUMP, HIGH);
}*/


void holdTime(word msec) {
  unsigned long czasA = millis();
  unsigned long czasB = millis();
  do {
       czasA = millis();
  } while (czasA - czasB >= msec);
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

void buttonCb(char * button_id)
   {
     String id = button_id;
     if( id == F("id_on") )
     {
      digitalWrite(WATER_PUMP, LOW);
      Serial.println("ON");
     }
     if( id == F("id_off") )
     {
      digitalWrite(WATER_PUMP, HIGH);
      Serial.println("OFF");

     }
   }

void loadCb(char * url)
   {
     bool stan;
     stan = digitalRead(PUMP_CHECK);
     if (stan == LOW){
      webServer.setArgString(F("text"), F("Pompa jest włączona"));
     }
     if (stan == HIGH){
      webServer.setArgString(F("text"), F("Pompa jest wyłączona"));
     }
     }


void refreshCb(char * url)
   {
     bool stan;
     stan = digitalRead(PUMP_CHECK);
     if (stan == LOW){
      webServer.setArgString(F("text"), F("Pompa jest włączona"));
     }
     if (stan == HIGH){
      webServer.setArgString(F("text"), F("Pompa jest wyłączona"));
     }
     }


/*int sprawdzPoziom() {
    int dystans = ultrasonic.distanceRead();
//  Serial.println(dystans);
    int procent;
    procent = ((dystans - poziom_min)*100)/(poziom_max - poziom_min);
//    Serial.print(procent);
//    Serial.println("%");
    return(procent);
}*/

int poziomSredni(){                     //dokończyc!!!
  static int tablica[10];
  for (int i = 0; i<10; i++){
      int poziom_sr = getData();
      tablica[i] = poziom_sr;
      delay(1000);
      }
      return tablica; 
}
    
byte getData(){ //obsługa działania na bitach z czujnika
   byte ramka[3]; 
   ramka[0] = 3;
   while (ramka[0] != 0 and ((ramka[0] + ramka[1] + ramka[2])>>8) != ramka[3]){
    mySerial.print(0x55);
    for (byte i=0; i=3; i++){
      ramka[i]= mySerial.read(); 
     }
   }
   holdTime(300);
   for (byte i=0; i=3; i++){
      Serial.println(ramka[i]); 
   }
   int dystans = ramka[1];
   dystans << 8;
   dystans =+ ramka[2];
   int procent;
   procent = ((dystans - poziom_min)*100)/(poziom_max - poziom_min);
   
return dystans;
}
 

void setup() {
Serial.begin(115200);
mySerial.begin(9600);
pinMode(A0, OUTPUT);// Pompka napowietrzania, cykliczne załączanie
pinMode(TRIGGER_PIN, OUTPUT); //trigger pin 
pinMode(ECHO_PIN, INPUT); //Echo pin 
pinMode(WATER_PUMP, OUTPUT); // wyjście sterowane guzikiem z html (pompka wydzutu wody),
pinMode(PUMP_CHECK, INPUT);
digitalWrite(A0, HIGH);
digitalWrite(WATER_PUMP, HIGH);
Serial.println("koniec sekwencji startowej");

  bool ok;
  do {
    ok = esp.Sync();      // synchronizuje esp-link, blokuje system na 2 sek.
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");


URLHandler *handler = webServer.createURLHandler(F("/Sterowanie.html.json"));
handler->loadCb.attach(&loadCb);
handler->refreshCb.attach(&refreshCb);
handler->buttonCb.attach(&buttonCb);

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

void loop() {

esp.Process();

//obsługa pompki powietrza
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
    int procent = getData();
    String url = ("/json.htm?type=command&param=udevice&idx=33&nvalue=0&svalue=");
    String s_proc = String(procent);
    String dane = url + s_proc;
    Serial.println(dane);
    char dane_char[63];
    strncpy(dane_char, dane.c_str(), sizeof(dane_char));
    dane_char[sizeof(dane_char) - 1] = 0;
    Serial.println(dane_char);
    rest.get(dane_char, "");
    delay(800);
    int srednio = poziomSredni();
    for (int i=0; i<10; i++){
      Serial.println(srednio[i]);
    }

    
      }
}
