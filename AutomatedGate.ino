#include "CMFRC522.h"
#include <SPI.h>
#include <Servo.h>

CMFRC522 RFID;
int RFIDout;
int AC[3] = {770, 642, 890}; // AC = accepted code
int red = 13;
int blue = 12;
int buzzer = 9;


Servo Servo0;

void setup() {
 Serial.begin(9600);
 SPI.begin();
 RFID.InitialRun1();
 RFID.InitialRun2();
 RFID.MFRC522_Init();
 Serial.println("Please put the IC card in the antenna inductive zone! ");
 pinMode(red, OUTPUT);
 pinMode(blue, OUTPUT);
 pinMode(buzzer, OUTPUT);
 
}

void loop() {
  RFIDout =RFID.MFRC522runtime();
  if(RFIDout == AC[0] || RFIDout == AC[1] || RFIDout == AC[2]){
    digitalWrite(red, LOW);
    digitalWrite(blue, HIGH);
    tone(buzzer, 1000);
    delay(1000);
    noTone(buzzer);
    digitalWrite(blue, LOW);
  
    
  }
  else{
    digitalWrite(red, HIGH);
 
  }
}
