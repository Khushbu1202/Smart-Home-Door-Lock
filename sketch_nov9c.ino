#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>
#include <Servo.h>

// -----------------------------
// OLED Display Settings
// -----------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -----------------------------
// RFID Settings
// -----------------------------
#define RST_PIN 9
#define SS_PIN 10
MFRC522 rfid(SS_PIN, RST_PIN);

// -----------------------------
// Servo Settings
// -----------------------------
Servo myServo;
int servoPin = 3;
int lockedPos = 0;
int unlockedPos = 190;

// -----------------------------
// Authorized RFID UID (change this to your card UID)
// -----------------------------
byte authorizedUID[4] = {0xDD, 0x37, 0x10, 0x05};

// -----------------------------
// Function Prototype
// -----------------------------
bool checkUID();

// -----------------------------
// Setup
// -----------------------------
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // -----------------------------
  // OLED Initialization
  // -----------------------------
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED initialization failed! Check wiring."));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("LOCKED");
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println("Scan RFID Tag");
  display.display();

  // -----------------------------
  // Check RFID Communication
  // -----------------------------
  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.print(F("MFRC522 Firmware Version: 0x"));
  Serial.println(version, HEX);

  if (version == 0x00 || version == 0xFF) {
    Serial.println(F("⚠️ RFID module not detected! Check wiring and power (use 3.3V)."));
    while (true);
  }

  // -----------------------------
  // Servo Initialization
  // -----------------------------
  myServo.attach(servoPin);
  myServo.write(lockedPos);
  Serial.println(F("System Ready. Waiting for RFID card..."));
}

// -----------------------------
// Main Loop
// -----------------------------
void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Read and display UID
  Serial.print("Scanned UID:");
  String content = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
    content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfid.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("RFID Tag:");
  display.println(content);
  display.display();
  delay(1000);

  // Check if authorized
  if (checkUID()) {
    Serial.println(F("✅ Access Granted"));
    myServo.write(unlockedPos);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("UNLOCKED  WELCOME TO   HOME ");
    display.display();
    delay(5000);  // Keep unlocked for 5 seconds
    myServo.write(lockedPos);
  } else {
    Serial.println(F("❌ Access Denied"));
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("DENIED");
    display.display();
    delay(100);
  }

  // Reset OLED to locked state
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("LOCKED");
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println("Scan RFID Tag");
  display.display();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(200);
}

// -----------------------------
// Function: checkUID()
// -----------------------------
bool checkUID() {
  if (rfid.uid.size != 4) return false;
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}
