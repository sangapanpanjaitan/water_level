#include <Wire.h>
#include <RTClib.h>
#include <Ultrasonic.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Informasi broker MQTT
const char* mqtt_server = "public.mqtthq.com";
const char* mqtt_topic = "water_level";

// Informasi koneksi Ethernet
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Inisialisasi koneksi Ethernet
EthernetClient ethClient;
PubSubClient client(ethClient);

// Inisialisasi sensor ultrasonik
#define trigPin 22
#define echoPin 24
Ultrasonic ultrasonic(trigPin, echoPin);

// Inisialisasi RTC
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);

  // Inisialisasi koneksi Ethernet
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    for(;;);
  }

  // Tunggu koneksi terbentuk
  delay(1000);

  // Inisialisasi koneksi MQTT
  client.setServer(mqtt_server, 1883);

  // Tunggu koneksi ke broker MQTT
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }

  // Inisialisasi RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  // Baca ketinggian air dari sensor ultrasonik
  float waterLevel = ultrasonic.read(CM);

  // Baca waktu dari RTC
  DateTime now = rtc.now();

  // Kirim data ke topik MQTT
  char message[50];
  snprintf(message, sizeof(message), "{\"water_level\": %.2f, \"timestamp\": \"%04d-%02d-%02d %02d:%02d:%02d\"}",
           waterLevel, now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  client.publish(mqtt_topic, message);

  Serial.println(message);

  delay(10000); // Kirim setiap 10 detik
}
