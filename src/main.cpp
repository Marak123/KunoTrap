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
bool isNotifIsNotActive = true;

// #define servoMotorPin = 26;
#define ledPin 13 //Pin wyjsciowy / output pin number on the board
#define ledState false //Poczatkowy status pinu / Initial pin state

#define trigPinSensor 2 //Pin czujnika odleglosci (triger) / Distance sensor pin (trigger)
#define echoPinSensor 4 //Pin czujnika odleglosci (echo) / Distance sensor pin (echo)

#define averageSize 100 //Dokladnosc wykonywanego pomiaru. Im wieksza tym dokladniejszy pomiara ale dluzszy czas pomiaru
                        //Accuracy of the measurement. The larger the number, the more accurate the measurement, but the longer the measurement time

#define margineOfDistanse 100 //Margines bledu czujnika odleglosci / Distance sensor margin of error

#define saveSizeValue 100 //Ilosc petli zapisu pomiarow odleglosci configuracyjnej. Im wieksza tym wykrywanie ruchu jest bardziej precyzyjne. Zalezne od otoczenia w ktorym czujnik sie znajduje. Nie warto przesadzac z wielkoscia
                          //Number of recording loops of configuration distance measurements. The higher the number, the more precise the motion detection is. Depends on the environment in which the sensor is located. It's not worth exaggerating the size

#define maxTimeToConect 20 //Ilosc petli która sprawdza połaczenie WIFI. Czas wykonywania petli laczenia sie z WIFI
bool isConnectToWifi = false;

NewPing sonar(trigPinSensor, echoPinSensor, 300);
void setup() {
  Serial.begin(9600);

  pinMode(trigPinSensor, OUTPUT);
  pinMode(echoPinSensor, INPUT);

  if(BOTtoken.length() == 0 || CHAT_ID.length() == 0) isNotifIsNotActive = false;

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(!isNotifIsNotActive) client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  int i=0;
  while (i < maxTimeToConect) {
    i++;
    if(WiFi.status() == WL_CONNECTED){
      isConnectToWifi = true;
      break;
    }
    Serial.print(".");
    delay(500);
  }

  if(isConnectToWifi){
    Serial.println("\nWiFi connected\nIP address: ");
    Serial.println(WiFi.localIP());
    if(!isNotifIsNotActive) bot.sendMessage(CHAT_ID, "Polaczono z WiFi :) \nIP Adres plytki: " + WiFi.localIP().toString());
  }

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

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
      digitalWrite(ledPin, !ledState);
      if(isConnectToWifi || !isNotifIsNotActive) bot.sendMessage(CHAT_ID, "Kunolis w klatce!! (chyba) :)", "");
      Serial.println("Kunolis chyba w klatce!!");
    }
    while(true){}
  }
}