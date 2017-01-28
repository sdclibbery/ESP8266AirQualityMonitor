#include "RunningMedian.h"
#include <ESP8266WiFi.h>

const int dustPin = A0;
const float ADC_REF = 3.3;
const int ADC_MAX = 1024;
const int ledPower = D1;

// WiFi connection
#include "wifi-credentials.h"

// Data stream
const char* statName = "dust";

// Sharp timing
int waitToRead = 280;
int waitToTurnOff = 40;
int waitForNext= 9680;

// sampling
int sampleSize = 15;
RunningMedian samples = RunningMedian(sampleSize);

int interval = 3000;
float lastReading ;
void setup(){
   Serial.begin(9600);
   connectWifi();
   pinMode(ledPower,OUTPUT);
   lastReading = millis();
}

void loop(){
  int baseDigital = analogRead(dustPin);
  digitalWrite(ledPower, LOW); // power on the LED
  delayMicroseconds(waitToRead);
  int readDigital = analogRead(dustPin);
  delayMicroseconds(waitToTurnOff);
  digitalWrite(ledPower, HIGH); // turn the LED off
  int dustDigital = readDigital - baseDigital;
  samples.add(dustDigital);
  if (millis() > lastReading + interval) {
    Serial.println("");
    float dustVoltage = samples.getAverage() * ADC_REF / ADC_MAX;
    String report = String("dustVoltage=")+dustVoltage;
    Serial.println(report);
    logData(dustVoltage);
    lastReading = millis();
  }
  delayMicroseconds(waitForNext);
}

void connectWifi() {
// Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void logData(float value) {
   String data = "stat=";
   data += statName;
   data += "&email=12.hhc.air.quality.monitor@gmail.com&value=";
   data += value;
   httpPost("api.stathat.com", "/ez", data);
}

void httpPost(const char* host, String url, String data) {
  int httpPort = 80;
  WiFiClient client;
  url.replace(" ", "+");
  Serial.print("POST to: ");
  Serial.print(host);
  Serial.println(url);
  while (true) {
    if (client.connect(host, httpPort)) break;
    Serial.println("connection failed");
    delay(100);
  }
  Serial.println("connected ");

  String httprequest = String("POST ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "User-Agent: ESP8266\r\n" +
              "Accept: */*\r\n" +
              "Content-Length: " + data.length() + "\r\n" +
              "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
              data;
//Serial.println(httprequest);
  client.print(httprequest);

  Serial.println("closing connection");
}
