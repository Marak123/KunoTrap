#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "Servo.h"
#include <NewPing.h>

const char* ssid = "";
const char* password = "";

#define BOTtoken ""  // your Bot Token (Get from Botfather)
#define CHAT_ID ""

WiFiClientSecure client;
// Servo myservo;
UniversalTelegramBot bot(BOTtoken, client);

// #define servoMotorPin = 26;
#define ledPin 13
#define trigPinSensor 2
#define echoPinSensor 4

#define averageSize 100
#define margineOfDistanse 100
#define saveSizeValue 100

NewPing sonar(trigPinSensor, echoPinSensor, 300);
void setup() {
  Serial.begin(9600);

  pinMode(trigPinSensor, OUTPUT);
  pinMode(echoPinSensor, INPUT);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Polaczono z WiFi :) \nIP Adres plytki: " + WiFi.localIP().toString());

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // myservo.attach(servoMotorPin);
  // myservo.write(0);
}


int saveValueArray[saveSizeValue] = { 0 };
int saveIndex, minSaveValue, maxSaveValue;

void loop() {

  // digitalWrite(trigPinSensor, LOW);
  // delayMicroseconds(2);
  // digitalWrite(trigPinSensor, HIGH);
  // delayMicroseconds(10);
  // digitalWrite(trigPinSensor, LOW);

  //duration = pulseIn(echoPinSensor, HIGH);
  // distance = duration * 0.0340 / 2;

  unsigned int distance = sonar.ping_median(averageSize);

  if(saveIndex < saveSizeValue){
    saveValueArray[saveIndex] = distance;

    Serial.print(saveIndex);
    Serial.print(" Save!! - ");

    saveIndex++;

    if(saveIndex >= saveSizeValue){
      minSaveValue = saveValueArray[0];
      for(int element : saveValueArray){
        if(minSaveValue > element) minSaveValue = element;
        if(maxSaveValue < element) maxSaveValue = element;
      }
      minSaveValue -= margineOfDistanse;
      maxSaveValue += margineOfDistanse;
    }
  }

  Serial.print("Distance: ");
  Serial.println(distance);

  if(saveIndex >= saveSizeValue && !((distance > minSaveValue) && (distance < maxSaveValue))){
    for(int i=0; i < 10; i++){
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(CHAT_ID, "Kunolis w klatce!! (chyba) :)", "");
      Serial.println("Kunolis chyba w klatce!!");
    }
    while(true){}
  }
}