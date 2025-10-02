#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ขา LoRa SX1276/78 สำหรับ ESP32-C3
#define LORA_SCK   8
#define LORA_MISO  9
#define LORA_MOSI  10
#define LORA_CS    4
#define LORA_RST   3
#define LORA_IRQ   2   // DIO0

// OLED 128x64
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

byte msgCount = 0;
int interval = 2000;
long lastSendTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Duplex + OLED - ESP32-C3");

  // เริ่มต้น OLED
  Wire.begin(6, 7); // SDA=6, SCL=7
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Ready!");
  display.display();

  // เริ่มต้น SPI
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring.");
    display.println("LoRa Failed!");
    display.display();
    while (true);
  }

  LoRa.setSpreadingFactor(8);
  Serial.println("LoRa init succeeded.");
  display.println("LoRa OK @433MHz");
  display.display();
}

void loop() {
  if (millis() - lastSendTime > interval) {
    String message = "HeLoRa World! " + String(msgCount);
    sendMessage(message);
    Serial.println("Sending: " + message);

    // แสดงบน OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sending:");
    display.println(message);
    display.display();

    lastSendTime = millis();
    interval = random(2000) + 1000;
    msgCount++;
  }

  onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();
  LoRa.print(outgoing);
  LoRa.endPacket();
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  Serial.println("Received: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("SNR: " + String(LoRa.packetSnr()));

  // แสดงบน OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Received:");
  display.println(incoming);
  display.print("RSSI: ");
  display.println(LoRa.packetRssi());
  display.print("SNR: ");
  display.println(LoRa.packetSnr());
  display.display();
}
