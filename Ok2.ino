#include "DHT.h"        //DHT Bibliothek für Temperatursensor und Feuchtigkeit
#include <IRremote.h>   //IRremote Bibliothek für Infrarotsensor und Fernbedinung
#include <Servo.h>      //Servo Bibliothek für den Servo

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

// Konfiguration PWM Ausgang/ Ausgang
int pwmLuefter = 6;
int ledAutoOrManuell = 13;

// Varablen
int InfrarotsensorValue = 0;
float luftfeuchtigkeit = 0;
float temperatur = 0;
int pwm = 150;
bool xAutoOrManuell = false;


void setup() {
  Serial.begin(9600); //Serielle Verbindung starten
  dht.begin(); //DHT11 Sensor starten
  irrecv.enableIRIn();
  serv.attach(8);
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
  /*
    if(x == true){servoblau.write(10);}
    else{servoblau.write(100);}
  */



}
