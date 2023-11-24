#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int trigPin = 9;
const int echoPin = 10;
const int chipSelect = 53;

unsigned long previousMillis = 0;
const long interval = 10000;  // Kirim data setiap 10 detik

const char* ssid = "Nama_Wifi";
const char* password = "Password_Wifi";

const char* scriptId = "YourGoogleScriptID";
const char* sheetId = "YourGoogleSheetID";

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (!display.begin(SSD1306_I2C_ADDRESS, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(115200);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    for (;;);
  }

  Serial.println("Card initialized.");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float distance = getDistance();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    writeToSDCard(distance);
    writeToGoogleSheets(distance);
    displayDistance(distance);
  }
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  return pulseIn(echoPin, HIGH) * 0.034 / 2;
}

void writeToSDCard(float distance) {
  File dataFile = SD.open("data.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print("Distance: ");
    dataFile.print(distance);
    dataFile.println(" cm");
    dataFile.close();
  } else {
    Serial.println("Error opening data.txt");
  }
}

void writeToGoogleSheets(float distance) {
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + String(scriptId) + "/exec?";
  url += "sheet=" + String(sheetId);
  url += "&data=" + String(distance);

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.println("Error sending data to Google Sheets");
  }

  http.end();
}

void displayDistance(float distance) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Distance: ");
  display.print(distance);
  display.print(" cm");
  display.display();
}
