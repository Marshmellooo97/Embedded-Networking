#include "DHT.h"        //DHT Bibliothek für Temperatursensor und Feuchtigkeit
#include <IRremote.h>   //IRremote Bibliothek für Infrarotsensor und Fernbedinung
#include <Servo.h>      //Servo Bibliothek für den Servo
#include <SPI.h>        //Serial Peripheral Interface Lib zur Kommunikation mit dem RFID Reader
#include <MFRC522.h>    //Bibliothek für den RFID Reader

// Konfiguration Temperatursensor und Feuchtigkeit
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); //Der Sensor wird ab jetzt mit „dth“ angesprochen


// Konfiguration Infrarotsensor und Fernbedinung
const int fernPIN = 4;   //Konstanten Definition für den Hardware Pin(Konstant, weil dieser sich nur durch Hardware modifikation ändert) 
IRrecv irrecv(fernPIN);  //Objekt der Klasse IRrecv, IRrecv ist eine Bibliothek, Objekt wird mit Konstanten fernPin erstellt
decode_results results;  //decode_results ist ebenfalls eine Bibliothek, Klasse decode_results setzt an dieser Stelle, Ergebnisse des Decodierens in das Objekt results


// Konfiguration Servo
Servo serv;

//Konfiguration RFID Reader         !!!Bitte noch Pins ändern!!!
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);                               // Erstellt MFRC522 Instanz.

// Konfiguration PWM Ausgang/ Ausgang
int pwmLuefter = 6;                                             //Pin für den Lüfer(PIN 6)
int ledAutoOrManuell = 7;                                       //Pin für die LED, die Angibt in welchem Modus man sich gerade Befindet

// Variablen
int InfrarotsensorValue = 0;                                    //Integer für den decodierten und empfangenen command
float luftfeuchtigkeit = 0;                                     //Lüftfeuchtigkeit
float temperatur = 0;                                           //Temperatur
int pwm = 150;                                                  //Lüftergeschwindikeit
bool xAutoOrManuell = false;                                    //Boolean für den Modus in dem Man sich gerade Befindet Auto oder Manuell
bool servstat = false;                                          //Um den Status der Tür zwischenzuspeichern, false Geschlossen, true = Offen 

void setup() {                                                  //setup()-Funktion alle Initialisierungen und Einstellungen, die für das Programm erforderlich sind.
  Serial.begin(9600);                                           //Serielle Verbindung starten
  dht.begin();                                                  //DHT11 Sensor starten
  SPI.begin();                                                  //Initalisiert SPI Bus
  mfrc522.PCD_Init();                                           //Initalisiert MFRC522
  irrecv.enableIRIn();                                          //aktiviert den Infarotsensor sodass er bereit ist Daten zu empfangen
  serv.attach(8);                                               //setzt den Kontoll Pin für den Servo auf Pin 8
  pinMode(pwmLuefter, OUTPUT);
  pinMode(ledAutoOrManuell, OUTPUT);

}

void loop() {
  delay(500);                                                   // Pause von x millisec

  // Temperatursensor und Feuchtigkeit Auslesen und Speichern
  luftfeuchtigkeit = dht.readHumidity();                        //die Luftfeuchtigkeit auslesen und unter „Luftfeutchtigkeit“ speichern
  temperatur = dht.readTemperature();                           //die Temperatur auslesen und unter „Temperatur“ speichern
  if(xAutoOrManuell){
    if(temperatur >= 50 and luftfeuchtigkeit >= 80){pwm = 255;
      }else if(temperatur >= 30 and luftfeuchtigkeit >= 0){pwm = 150;
      }else if(temperatur >= 20){pwm = 100;}
  }
  
  

  // Infrarotsensor und Fernbedinung
  if (IrReceiver.decode()) {                                    //If abfrage überprüft, ob das Infrarot-Empfängermodul ein gültiges Infrarotsignal empfangen konnte
    IrReceiver.resume();                                        //aktiviert den Infrarot-Empfänger 
    InfrarotsensorValue = IrReceiver.decodedIRData.command;     //speichert den Befehlscode des decodierten Infrarotsignals in der Variablen InfrarotsensorValue
    //Serial.println(InfrarotsensorValue);
    if (InfrarotsensorValue == 70) {                            //überprüft ob der Command = 70 ist
      xAutoOrManuell = true;                                    //falls Command = 70 sind wir im Auto Modus(xAutoOrManuell = true)
    }
    if (InfrarotsensorValue == 21) {                            //überprüft ob der Command = 21 ist
      xAutoOrManuell = false;                                   //falls Command = 21 sind wir im Manuellen Modus(xAutoOrManuell = false)
    }
    if (InfrarotsensorValue == 28) {                            //überprüft ob der Command = 28 ist 
      serv.write(10);                                           //falls ja write()-Funktion, verwendet um Daten an ein serielles Gerät zu senden(Hier Wert 10)(rotiere um 10°)
    } else {                                                    //falls der Command nicht 28 ist 
      serv.write(90);                                           //wird 90 an das serielle Gerät gesendet(rotiere um 90°)
    }
    if (xAutoOrManuell == false){                               //überprüft ob wir uns im Manuellen Modus Befinden falls ja
      if (InfrarotsensorValue == 67) {                          //prüft ob der Command 67 ist (von IrReceiver.decodedIRData.command)
        if (pwm <= 200) {                                       //falls ja wird geprüft ob die Lüftergeschwindikeit <= 200 ist, weil der Lüfter nicht unendlich schnell drehen kann
          pwm = pwm + 50;                                       //falls pwm(Lüfergeschwindikeit) <= 200 ist dann erhöhen wir die geschwindikeit um 50
        }
      }else if (InfrarotsensorValue == 68) {                    //prüft ob der Command 67 ist (von IrReceiver.decodedIRData.command)
        if (pwm > 0) {                                          //prüft ob die Drehzahl des Lüfters > 0 ist weil wir den Lüfter nich mit negativer Geschwindikeit drehen lassen können.
          pwm = pwm - 50;}                                      //falls obere Bedinung true, dann wird die Geschwindikeit um 50 reduziert
      }
    }
  }

  analogWrite(pwmLuefter, pwm);
  Serial.print("PWM: ");
  Serial.println(pwm);
  Serial.print("Luft: ");
  Serial.println(luftfeuchtigkeit);
  Serial.print("Temp: ");
  Serial.println(temperatur);
  Serial.print("Infrarot: ");
  Serial.println(InfrarotsensorValue);
  Serial.print("AutoOrManuell: ");
  Serial.println(xAutoOrManuell);

  if(xAutoOrManuell){                                           //Dient zur visuellen Bestätigung in welchem Modus wir uns Befinden
    digitalWrite(ledAutoOrManuell, HIGH);                       //falls wir uns im Auto Modus Befinden wird der PIN 7 auf HIGH gesetzt (Led leuchtet)
  }else{analogWrite(ledAutoOrManuell, LOW);}                    //da ein Bool nur zwei zustände hat müssen wir uns im andern Fall im Manuellen Modus Befinden und die LED wird ausgeschaltet

  
  // Türverrigelung

  // Setz die Schleife zurück, wenn keine neue Karte am Sensor/Lesegerät vorhanden ist. Dies speichert gesamten Vorgang im Leerlauf.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Wählt eine Karte aus
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Wandelt die UUID in einen verarbeitbaren String um
  String content= "";
  byte letter;
  for (byte i = 0; i <   mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10   ? " 0" : " ");          //nötig?
     Serial.print(mfrc522.uid.uidByte[i], HEX);                           //||
     content.concat(String(mfrc522.uid.uidByte[i]   < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i],   HEX));
  }
  Serial.print("Message : ");
  content.toUpperCase();

  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  //Nur um die UUID vom Chip zu erhalten, muss die UUID für den zulässigen Chip in der nächsten if-Anweisung fest codiert werden
                                              
  //prüfung ob die UUID befugt ist
  if (content.substring(1) == "E6 7B B6 F9")  //Hier die UID ändern um einer oder mehrer Karten Zugang zu gewähren
  {                                          
    if (servstat == false) {
        serv.write(180);                      //wenn die Tür geschlossen ist Rotiert der servo um 180° und löst die Verriegelung
        servstat = true;                      //variable servstat wird auf true gesetzt für Tür offen
    }else{
        serv.write(0);                        //wenn die Tür geöffnet ist, wird sie geschlossen(Der Tür mechanismus muss manuell Geschlossen werden)
        servstat = false;                     //variable servstat wird auf false gesetzt für Tür geschlossen
    }
  }
}
