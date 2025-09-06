#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// CC1101 pins for Wemos D1 mini Pro
#define CC1101_SCK 14
#define CC1101_MISO 12
#define CC1101_MOSI 13
#define CC1101_CS 15

// LED pin (D1 = GPIO5) 
#define LED_PIN 5

// EEPROM addresses and magic number for validation
#define EEPROM_MAGIC_ADDR 0       // Magic number (4 bytes): 0-3
#define EEPROM_ADDR_FREQ 4        // Float (4 bytes): 4-7
#define EEPROM_ADDR_SPEED 8       // Unsigned int (4 bytes): 8-11  
#define EEPROM_ADDR_INTERVAL 12   // Float (4 bytes): 12-15
#define EEPROM_ADDR_MSG 16        // String (100 bytes): 16-115
#define EEPROM_MSG_MAXLEN 100
#define EEPROM_MAGIC_NUMBER 0x12345678  // Magic number to detect valid EEPROM data

// WiFi and Web server
ESP8266WebServer server(80);
const char* ap_ssid = "FOXTX";
const char* ap_password = "FOXTX1234";

// Global variables
float freqMHz;
unsigned int ditLength;
float sendIntervalMinutes;
unsigned long sendIntervalMillis;
char message[EEPROM_MSG_MAXLEN + 1];

bool transmitting = false;
bool toneMode = false;

// Morse table
struct MorseMap {
  char letter;
  const char *code;
};

MorseMap morseTable[] = {
  {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},  {'E', "."},
  {'F', "..-."}, {'G', "--."},  {'H', "...."}, {'I', ".."},   {'J', ".---"},
  {'K', "-.-"},  {'L', ".-.."}, {'M', "--"},   {'N', "-."},   {'O', "---"},
  {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
  {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"}, {'Y', "-.--"},
  {'Z', "--.."},
  {'0', "-----"},{'1', ".----"},{'2', "..---"},{'3', "...--"},{'4', "....-"},
  {'5', "....."},{'6', "-...."},{'7', "--..."},{'8', "---.."},{'9', "----."},
  {' ', " "}
};

const int intraCharGap = 50;

// ===== EEPROM functions =====
void setDefaultValues() {
  freqMHz = 433.896210;
  ditLength = 120;
  sendIntervalMinutes = 1.0;
  strcpy(message, "CQ CQ CQ TEST TEST SA7BNB SA7BNB K");
  sendIntervalMillis = (unsigned long)(sendIntervalMinutes * 60000UL);
}

bool loadSettings() {
  Serial.println("Checking EEPROM for saved settings...");
  
  // Check magic number to see if EEPROM contains valid data
  uint32_t magic;
  EEPROM.get(EEPROM_MAGIC_ADDR, magic);
  
  if (magic != EEPROM_MAGIC_NUMBER) {
    Serial.println("No valid EEPROM data found, using defaults");
    setDefaultValues();
    return false; // No valid data found
  }
  
  Serial.println("Valid EEPROM data found, loading settings...");
  
  // Load settings from EEPROM
  EEPROM.get(EEPROM_ADDR_FREQ, freqMHz);
  EEPROM.get(EEPROM_ADDR_SPEED, ditLength);
  EEPROM.get(EEPROM_ADDR_INTERVAL, sendIntervalMinutes);
  
  // Load message
  for (int i = 0; i < EEPROM_MSG_MAXLEN; i++) {
    message[i] = EEPROM.read(EEPROM_ADDR_MSG + i);
    if (message[i] == 0) break;
  }
  message[EEPROM_MSG_MAXLEN] = 0;
  
  Serial.printf("Loaded - Freq: %.6f MHz, Dit: %d ms, Interval: %.2f min\n", freqMHz, ditLength, sendIntervalMinutes);
  Serial.printf("Loaded message: '%s'\n", message);
  
  // Validate loaded values and use defaults if invalid
  bool valid = true;
  if (freqMHz < 400.0 || freqMHz > 500.0) {
    Serial.println("Invalid frequency, using default");
    freqMHz = 433.896210;
    valid = false;
  }
  if (ditLength < 50 || ditLength > 1000) {
    Serial.println("Invalid dit length, using default");
    ditLength = 120;
    valid = false;
  }
  if (sendIntervalMinutes < 0.01) {
    Serial.println("Invalid interval, using default");
    sendIntervalMinutes = 1.0;
    valid = false;
  }
  if (strlen(message) == 0) {
    Serial.println("Empty message, using default");
    strcpy(message, "CQ CQ CQ TEST TEST SA7BNB SA7BNB K");
    valid = false;
  }
  
  sendIntervalMillis = (unsigned long)(sendIntervalMinutes * 60000UL);
  
  // If any value was invalid, save corrected values
  if (!valid) {
    Serial.println("Some values were invalid, saving corrected defaults");
    saveSettings();
  }
  
  return true;
}

void saveSettings() {
  Serial.println("Saving settings to EEPROM...");
  Serial.printf("Saving - Freq: %.6f MHz, Dit: %d ms, Interval: %.2f min\n", freqMHz, ditLength, sendIntervalMinutes);
  Serial.printf("Saving message: '%s'\n", message);
  
  // Save magic number first
  uint32_t magic = EEPROM_MAGIC_NUMBER;
  EEPROM.put(EEPROM_MAGIC_ADDR, magic);
  
  // Save all settings
  EEPROM.put(EEPROM_ADDR_FREQ, freqMHz);
  EEPROM.put(EEPROM_ADDR_SPEED, ditLength);
  EEPROM.put(EEPROM_ADDR_INTERVAL, sendIntervalMinutes);
  
  // Save message
  for (int i = 0; i < EEPROM_MSG_MAXLEN; i++) {
    EEPROM.write(EEPROM_ADDR_MSG + i, i < strlen(message) ? message[i] : 0);
  }
  
  EEPROM.commit();
  sendIntervalMillis = (unsigned long)(sendIntervalMinutes * 60000UL);
  Serial.println("Settings saved to EEPROM successfully");
}

void clearEEPROM() {
  Serial.println("Clearing entire EEPROM...");
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  Serial.println("EEPROM cleared");
}

// ===== Morse functions =====
void sendSignal(int duration) {
  ELECHOUSE_cc1101.SetTx();
  digitalWrite(LED_PIN, HIGH); // Turn on LED during transmission
  delay(duration);
  ELECHOUSE_cc1101.setSidle();
  digitalWrite(LED_PIN, LOW);  // Turn off LED
  delay(intraCharGap);
}

void sendDit() { sendSignal(ditLength); }
void sendDah() { sendSignal(ditLength * 3); }

const char* getMorseCode(char c) {
  c = toupper(c);
  for (unsigned int i = 0; i < sizeof(morseTable) / sizeof(MorseMap); i++)
    if (morseTable[i].letter == c) return morseTable[i].code;
  return "";
}

void sendChar(char c) {
  if (c == ' ') {
    delay(ditLength * 7); // Word pause - LED off
    return;
  }
  const char* code = getMorseCode(c);
  for (int i = 0; code[i] != '\0'; i++) {
    if (code[i] == '.') sendDit();
    else if (code[i] == '-') sendDah();
  }
  delay(ditLength * 3); // Letter pause - LED off
}

void sendMessage(const char* msg) {
  for (int i = 0; msg[i] != '\0'; i++) sendChar(msg[i]);
}

// ===== Web Server =====
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Fox Transmitter</title>";
  html += "<style>";
  html += "body{font-family:Arial;margin:20px;background:#f0f0f0;}";
  html += ".container{max-width:600px;margin:0 auto;background:white;padding:30px;border-radius:10px;}";
  html += "h1{text-align:center;color:#333;}";
  html += ".status{padding:15px;margin:15px 0;border-radius:6px;text-align:center;font-weight:bold;}";
  html += ".active{background:#d4edda;color:#155724;}";
  html += ".inactive{background:#f8d7da;color:#721c24;}";
  html += ".tone{background:#fff3cd;color:#856404;}";
  html += "button{background:#007bff;color:white;padding:12px 20px;border:none;border-radius:6px;margin:5px;cursor:pointer;font-size:16px;}";
  html += "button:hover{background:#0056b3;}";
  html += ".success{background:#28a745;}.success:hover{background:#1e7e34;}";
  html += ".danger{background:#dc3545;}.danger:hover{background:#c82333;}";
  html += ".warning{background:#ffc107;color:#212529;}.warning:hover{background:#e0a800;}";
  html += "input,textarea{width:100%;padding:10px;margin:5px 0;border:2px solid #ddd;border-radius:4px;box-sizing:border-box;}";
  html += "label{display:block;margin:10px 0 5px 0;font-weight:bold;}";
  html += ".controls{text-align:center;margin:20px 0;}";
  html += ".section{margin:20px 0;padding:20px;background:#f8f9fa;border-radius:8px;}";
  html += ".clear-eeprom{background:#6c757d;color:white;font-size:14px;padding:8px 16px;}";
  html += ".clear-eeprom:hover{background:#545b62;}";
  html += "</style></head><body>";
  
  html += "<div class='container'>";
  html += "<h1>Fox Transmitter</h1>";
  
  // Status
  html += "<div class='status ";
  if (toneMode) html += "tone'>CARRIER TEST ACTIVE";
  else if (transmitting) html += "active'>TRANSMISSION ACTIVE";
  else html += "inactive'>TRANSMISSION STOPPED";
  html += "</div>";
  
  // Controls
  html += "<div class='controls'>";
  html += "<button onclick=\"location.href='/start'\" class='success'>Start Transmission</button>";
  html += "<button onclick=\"location.href='/stop'\" class='danger'>Stop Transmission</button>";
  html += "<button onclick=\"location.href='/test'\" class='warning'>Test Transmission</button>";
  html += "</div>";
  
  // Carrier controls
  html += "<div class='controls'>";
  if (toneMode) {
    html += "<button onclick=\"location.href='/tone_stop'\" class='danger'>Stop Carrier</button>";
    html += "<div style='margin:20px 0;'>";
    html += "<button onclick=\"location.href='/freq?d=0.001'\" class='success'>+1kHz</button>";
    html += "<button onclick=\"location.href='/freq?d=-0.001'\" class='danger'>-1kHz</button>";
    html += "<button onclick=\"location.href='/freq?d=0.0001'\" class='success'>+100Hz</button>";
    html += "<button onclick=\"location.href='/freq?d=-0.0001'\" class='danger'>-100Hz</button>";
    html += "</div>";
  } else {
    html += "<button onclick=\"location.href='/tone_start'\" class='warning'>Start Carrier Test</button>";
  }
  html += "</div>";
  
  // Settings
  if (!toneMode) {
    html += "<div class='section'>";
    html += "<form method='POST' action='/save'>";
    html += "<label>Frequency (MHz):</label>";
    html += "<input type='number' name='freq' value='" + String(freqMHz, 6) + "' step='0.000001' min='400' max='500'>";
    html += "<label>Dit Length (ms):</label>";
    html += "<input type='number' name='speed' value='" + String(ditLength) + "' min='50' max='1000'>";
    html += "<label>Interval (minutes):</label>";
    html += "<input type='number' name='interval' value='" + String(sendIntervalMinutes, 2) + "' step='0.01' min='0.01'>";
    html += "<label>Message:</label>";
    html += "<textarea name='message' maxlength='100'>" + String(message) + "</textarea>";
    html += "<div class='controls'>";
    html += "<button type='submit' class='success'>Save Settings</button>";
    html += "<button type='button' onclick='location.reload()' class='warning'>Refresh Page</button>";
    html += "<button type='button' onclick=\"if(confirm('Clear all saved settings?')) location.href='/clear_eeprom'\" class='clear-eeprom'>Clear EEPROM</button>";
    html += "</div>";
    html += "</form>";
    html += "</div>";
  }
  
  // Status info
  html += "<div class='section'>";
  html += "<p><strong>Current Frequency:</strong> " + String(freqMHz, 6) + " MHz</p>";
  html += "<p><strong>Current Message:</strong> " + String(message) + "</p>";
  html += "<p><strong>Current Interval:</strong> " + String(sendIntervalMinutes, 1) + " min</p>";
  html += "<p><strong>Current Dit Length:</strong> " + String(ditLength) + " ms</p>";
  html += "</div>";
  
  // Footer with developer credit
  html += "<div class='section' style='text-align:center;font-size:14px;color:#666;'>";
  html += "<p>Fox Transmitter - Designed and developed by SA7BNB</p>";
  html += "<p>Amateur Radio Morse Code Transmitter</p>";
  html += "</div>";
  
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleStart() {
  if (!toneMode) transmitting = true;
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleStop() {
  transmitting = false;
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleTest() {
  if (!transmitting && !toneMode) {
    sendMessage("TEST");
  }
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleToneStart() {
  if (!transmitting) {
    toneMode = true;
    ELECHOUSE_cc1101.SetTx();
  }
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleToneStop() {
  toneMode = false;
  ELECHOUSE_cc1101.setSidle();
  saveSettings(); // Save frequency after carrier test
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleFreq() {
  if (server.hasArg("d") && toneMode) {
    float delta = server.arg("d").toFloat();
    freqMHz += delta;
    if (freqMHz < 400.0) freqMHz = 400.0;
    if (freqMHz > 500.0) freqMHz = 500.0;
    ELECHOUSE_cc1101.setMHZ(freqMHz);
    // Frequency is saved when carrier test is stopped
  }
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleSave() {
  if (server.hasArg("freq")) {
    float newFreq = server.arg("freq").toFloat();
    if (newFreq >= 400.0 && newFreq <= 500.0) {
      freqMHz = newFreq;
      ELECHOUSE_cc1101.setMHZ(freqMHz);
    }
  }
  
  if (server.hasArg("speed")) {
    int newSpeed = server.arg("speed").toInt();
    if (newSpeed >= 50 && newSpeed <= 1000) {
      ditLength = newSpeed;
    }
  }
  
  if (server.hasArg("interval")) {
    float newInterval = server.arg("interval").toFloat();
    if (newInterval >= 0.01) {
      sendIntervalMinutes = newInterval;
    }
  }
  
  if (server.hasArg("message")) {
    String newMessage = server.arg("message");
    newMessage.trim();
    newMessage.toUpperCase();
    if (newMessage.length() <= EEPROM_MSG_MAXLEN) {
      newMessage.toCharArray(message, EEPROM_MSG_MAXLEN + 1);
    }
  }
  
  saveSettings();
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleClearEEPROM() {
  clearEEPROM();
  setDefaultValues();
  saveSettings();
  server.sendHeader("Location", "/");
  server.send(302);
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== FOX TRANSMITTER STARTING ===");
  
  EEPROM.begin(512);
  
  // Try to load settings from EEPROM
  bool settingsLoaded = loadSettings();
  
  if (!settingsLoaded) {
    Serial.println("First time setup - saving default values");
    saveSettings();
  }

  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // LED off at startup

  ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
  
  if (ELECHOUSE_cc1101.getCC1101()) {
    ELECHOUSE_cc1101.Init();
    Serial.println("CC1101 initialized");
  } else {
    Serial.println("WARNING: CC1101 not found!");
  }

  ELECHOUSE_cc1101.setMHZ(freqMHz);
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setPA(12);
  
  // Start WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  // Configure server routes
  server.on("/", handleRoot);
  server.on("/start", handleStart);
  server.on("/stop", handleStop);
  server.on("/test", handleTest);
  server.on("/tone_start", handleToneStart);
  server.on("/tone_stop", handleToneStop);
  server.on("/freq", handleFreq);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/clear_eeprom", handleClearEEPROM);
  server.onNotFound(handleRoot);
  
  server.begin();
  
  Serial.println("WiFi Access Point started");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("Password: ");
  Serial.println(ap_password);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Connect and go to: http://192.168.4.1");
  Serial.println("=== FOX TRANSMITTER READY ===");
  Serial.printf("Frequency: %.6f MHz\n", freqMHz);
  Serial.printf("Message: %s\n", message);
  Serial.printf("Interval: %.1f minutes\n", sendIntervalMinutes);
  Serial.printf("Dit length: %d ms\n", ditLength);
}

// ===== Loop =====
void loop() {
  server.handleClient();
  
  // LED status indication
  if (toneMode) {
    // Slow blinking during carrier test
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    if (millis() - lastBlink > 500) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlink = millis();
    }
  } else if (transmitting) {
    // LED is handled automatically by morse transmission
  } else {
    // Turn off LED when inactive
    digitalWrite(LED_PIN, LOW);
  }

  // Transmission loop
  if (transmitting && !toneMode) {
    static unsigned long lastSend = 0;
    if (millis() - lastSend >= sendIntervalMillis) {
      Serial.print("TX: ");
      Serial.println(message);
      sendMessage(message);
      lastSend = millis();
    }
  }

  delay(10);
}
