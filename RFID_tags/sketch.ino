#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- RFID Pins ---
#define SS_PIN  5
#define RST_PIN 4

// --- OLED Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // We aren't using a dedicated reset pin for the OLED
#define SCREEN_ADDRESS 0x3C // Default I2C address for SSD1306

// Create instances for the hardware
MFRC522 mfrc522(SS_PIN, RST_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // Initialize SPI bus for RFID
  SPI.begin();
  mfrc522.PCD_Init();

  // Initialize I2C for OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Freeze if the screen fails to load
  }

  // Clear the screen buffer and show the startup message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 10);
  display.println("System Ready!");
  display.setCursor(0, 30);
  display.println("Scan your tag...");
  display.display();

  Serial.println("System Ready! Waiting for a card...");
}

void loop() {
  // Check if a new card is present
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Verify the card can be read
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Extract the UID into a readable string
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();

  Serial.print("Tag Scanned! UID:");
  Serial.println(uidString);

  // Update the OLED display with the scanned UID
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Tag Scanned!");
  display.setCursor(0, 30);
  display.println("UID:");
  display.setCursor(0, 45);
  display.println(uidString);
  display.display();

  // Halt the card so it doesn't spam the reader 100 times a second
  mfrc522.PICC_HaltA();

  // Pause for 2 seconds so you can read the screen
  delay(2000);

  // Reset the screen back to the default prompt
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("System Ready!");
  display.setCursor(0, 30);
  display.println("Scan your tag...");
  display.display();
}
