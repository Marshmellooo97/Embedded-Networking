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
const int fernPIN = 10;
IRrecv irrecv(fernPIN);
decode_results results;


// Konfiguration Servo
Servo serv;

//Konfiguration RFID Reader         !!!Bitte noch Pins ändern!!!
#define SS_PIN 10   
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);    // Create MFRC522 instance.

// Konfiguration PWM Ausgang/ Ausgang
int pwmLuefter = 6;
int ledAutoOrManuell = 13;

// Variablen
int InfrarotsensorValue = 0;
float luftfeuchtigkeit = 0;
float temperatur = 0;
int pwm = 150;
bool xAutoOrManuell = false;
bool servstat = false;              //for saving look status, false = closed true = open

void setup() {
  Serial.begin(9600);     //Serielle Verbindung starten
  dht.begin();            //DHT11 Sensor starten
  SPI.begin();            //Initiate SPI Bus
  mfrc522.PCD_Init();     //Initiate MFRC522
  irrecv.enableIRIn();
  serv.attach(8);                      // set the controll pin for serv to 8
  pinMode(pwmLuefter, OUTPUT);
  pinMode(ledAutoOrManuell, OUTPUT);

}

void loop() {
  delay(500); // Pause von x millisec

  // Temperatursensor und Feuchtigkeit Auslesen und Speichern
  luftfeuchtigkeit = dht.readHumidity(); //die Luftfeuchtigkeit auslesen und unter „Luftfeutchtigkeit“ speichern
  temperatur = dht.readTemperature();//die Temperatur auslesen und unter „Temperatur“ speichern
  if(xAutoOrManuell){
    if(temperatur >= 50 and luftfeuchtigkeit >= 80){pwm = 255;
      }else if(temperatur >= 30 and luftfeuchtigkeit >= 0){pwm = 150;
      }else if(temperatur >= 20){pwm = 50;}
  }
  
  

  // Infrarotsensor und Fernbedinung
  if (IrReceiver.decode()) {
    IrReceiver.resume();
    InfrarotsensorValue = IrReceiver.decodedIRData.command;
    //Serial.println(InfrarotsensorValue);
    if (InfrarotsensorValue == 70) {
      xAutoOrManuell = true;
    }
    if (InfrarotsensorValue == 21) {
      xAutoOrManuell = false;
    }
    if (InfrarotsensorValue == 28) {
      serv.write(10);
    } else {
      serv.write(90);
    }
    if (xAutoOrManuell == false){
      if (InfrarotsensorValue == 67) {
        if (pwm <= 200) {
          pwm = pwm + 50;
        }
      }else if (InfrarotsensorValue == 68) {
        if (pwm > 0) {
          pwm = pwm - 50;}
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

  if(xAutoOrManuell){
    digitalWrite(ledAutoOrManuell, HIGH);
  }else{analogWrite(ledAutoOrManuell, LOW);}

  
  // Türverrigelung

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //build the UUID into workable string
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

  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  //only for getting UUID from chip, UUID for allowed chip needs to get hardcoded in next if statement

  //check if UUID is allowed
  if (content.substring(1) == "1A 7C 78 30") //change here the UID of the card/cards that you want to give access
  {
    if (servStatus == false) {
        serv.write(180);                //if door is closed, servo rotates 180° and frees door
        servstat = true;
    }else{
        serv.write(0);                  //if door is opened, door will close (Door mechanism needs to be closed manually)
        servstat = false;
    }
  }
}
