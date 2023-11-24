#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Informasi broker MQTT
const char* mqtt_server = "public.mqtthq.com"; // public.mqtthq.com => Menggunakan Broker MQTT online Publik
const char* mqtt_topic = "sensor/data";

// Informasi koneksi Ethernet
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Inisialisasi koneksi Ethernet
EthernetClient ethClient;
PubSubClient client(ethClient);

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
}

void loop() {
  // Kirim data ke topik MQTT
  float sensorValue = analogRead(A0) * 0.0048828125;
  char message[10];
  dtostrf(sensorValue, 1, 2, message);
  client.publish(mqtt_topic, message);

  delay(5000); // Kirim setiap 5 detik
}
