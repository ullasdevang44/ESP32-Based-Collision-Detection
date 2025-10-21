#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <ESP_Mail_Client.h>
#include <WiFiManager.h>
#include "BluetoothSerial.h"
#include "time.h"

// ========== Pin Definitions ==========
#define COLLISION_SENSOR 34

#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33
#define EN 32

#define FRONT_LED 12
#define BACK_LED 13
#define BUZZER 15

// ========== SMTP Configuration ==========
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "ullasdevang44@gmail.com"
#define AUTHOR_PASSWORD "gywp inte vbea hwng"
#define RECIPIENT_EMAIL1 "ullasdevang44@gmail.com"
#define RECIPIENT_EMAIL2 "secondemail@example.com"

SMTPSession smtp;
const char *apiURL = "http://ip-api.com/json/";

// ========== Bluetooth ==========
BluetoothSerial SerialBT;

// ========== Control Flags ==========
bool isAlertActive = false;

// ========== Function Prototypes ==========
void stopMotors();
void forward();
void backward();
void left();
void right();
void flushBluetoothInput();
String fetchLocation();
void sendEmailWithRetry(String locationData);
void triggerAccident();
void resetAlertOutputs();
void smtpCallback(SMTP_Status status);
bool setSystemTime();
bool ensureWiFiConnected();
void testPing();
void beepBuzzer(unsigned long duration, unsigned int interval);
void blinkLEDs(unsigned long duration, unsigned int interval);

void setup() {
  Serial.begin(115200);
  delay(300);

  // Motor setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(EN, OUTPUT);
  stopMotors();

  // Collision sensor setup (pulldown)
  pinMode(COLLISION_SENSOR, INPUT_PULLDOWN);

  // LEDs & Buzzer
  pinMode(FRONT_LED, OUTPUT);
  pinMode(BACK_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  resetAlertOutputs();

  SerialBT.begin("ESP32_Car");
  Serial.println("✅ Bluetooth started: ESP32_Car");

  WiFiManager wm;
  if (!wm.autoConnect("ESP32_Config")) {
    Serial.println("WiFi connection failed! Restarting...");
    ESP.restart();
  }
  Serial.print("🌐 WiFi connected. IP: ");
  Serial.println(WiFi.localIP());

  testPing();

  if (!setSystemTime()) {
    Serial.println("⚠️ Time sync failed! Email may not work.");
  }

  smtp.callback(smtpCallback);

  Serial.println("🚀 System initialized successfully.");
}

void loop() {
  // Bluetooth Car Control
  if (!isAlertActive && SerialBT.available()) {
    char cmd = SerialBT.read();
    Serial.printf("BT cmd: %c\n", cmd);
    switch (cmd) {
      case 'F': forward();  break;
      case 'B': backward(); break;
      case 'L': left();     break;
      case 'R': right();    break;
      case 'S': stopMotors(); break;
      default: stopMotors(); break;
    }
  }

  // Collision Detection
  if (digitalRead(COLLISION_SENSOR) == LOW && !isAlertActive) {
    Serial.println("⚠️ Collision Detected!");
    triggerAccident();

    // Blink LEDs until collision sensor goes HIGH (reset)
    while(digitalRead(COLLISION_SENSOR) == LOW) {
      blinkLEDs(500, 250);  // Blink for 500ms with 250 ms ON/OFF cycle
    }
    Serial.println("✅ Collision sensor reset for next detection.");
  }

  delay(5);
}

// Motor and LED behavior during movement
void forward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(EN, 200);
  digitalWrite(FRONT_LED, HIGH);
  digitalWrite(BACK_LED, LOW);
}
void backward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(EN, 200);
  digitalWrite(FRONT_LED, LOW);
  digitalWrite(BACK_LED, HIGH);
}
void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(EN, 200);
  digitalWrite(FRONT_LED, LOW);
  digitalWrite(BACK_LED, LOW);
}
void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(EN, 200);
  digitalWrite(FRONT_LED, LOW);
  digitalWrite(BACK_LED, LOW);
}
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(EN, 0);
  digitalWrite(FRONT_LED, LOW);
  digitalWrite(BACK_LED, LOW);
}

// Accident Trigger (Collision)
void triggerAccident() {
  isAlertActive = true;
  stopMotors();
  flushBluetoothInput();

  // LEDs ON steady during buzzer beep
  digitalWrite(FRONT_LED, HIGH);
  digitalWrite(BACK_LED, HIGH);

  // Loud buzzer beep for 2 seconds (200ms ON/OFF cycle)
  beepBuzzer(2000, 200);

  resetAlertOutputs();

  String locationData = fetchLocation();
  Serial.println(locationData);

  sendEmailWithRetry(locationData);

  // Blink LEDs rapidly for 5 seconds indicating alert phase
  unsigned long alertStart = millis();
  while (millis() - alertStart < 5000) {
    blinkLEDs(200, 100);  // blink for 200ms with 100 ms ON/OFF cycle
  }

  resetAlertOutputs();
  isAlertActive = false;
  Serial.println("✅ Collision alert handled and system reset.");
}

// Buzzer beep helper with specified duration and interval for loud beep
void beepBuzzer(unsigned long duration, unsigned int interval) {
  unsigned long start = millis();
  bool buzzerState = false;
  while (millis() - start < duration) {
    buzzerState = !buzzerState;
    digitalWrite(BUZZER, buzzerState ? HIGH : LOW);
    delay(interval);
  }
  digitalWrite(BUZZER, LOW);
}

// LED blink helper: blinks both front & back LEDs for duration (one cycle)
void blinkLEDs(unsigned long duration, unsigned int interval) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    digitalWrite(FRONT_LED, HIGH);
    digitalWrite(BACK_LED, HIGH);
    delay(interval);
    digitalWrite(FRONT_LED, LOW);
    digitalWrite(BACK_LED, LOW);
    delay(interval);
  }
}

void resetAlertOutputs() {
  digitalWrite(FRONT_LED, LOW);
  digitalWrite(BACK_LED, LOW);
  digitalWrite(BUZZER, LOW);
}
void flushBluetoothInput() {
  while (SerialBT.available()) SerialBT.read();
}

String fetchLocation() {
  if (!ensureWiFiConnected()) {
    Serial.println("WiFi not connected!");
    return "WiFi not connected!";
  }
  HTTPClient http;
  http.begin(apiURL);
  http.setTimeout(5000);
  int httpCode = http.GET();
  String locationMsg = "";
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    JSONVar data = JSON.parse(payload);
    if (JSON.typeof(data) != "undefined" && String((const char*)data["status"]) == "success") {
      double lat = (double)data["lat"];
      double lon = (double)data["lon"];
      locationMsg = "Collision detected!\n";
      locationMsg += "Latitude: " + String(lat, 6) + "\n";
      locationMsg += "Longitude: " + String(lon, 6) + "\n";
      locationMsg += "Google Maps: https://www.google.com/maps?q=" + String(lat, 6) + "," + String(lon, 6);
    } else {
      locationMsg = "Location unavailable.";
    }
  } else {
    locationMsg = "HTTP error: " + String(httpCode);
  }
  http.end();
  return locationMsg;
}

void sendEmailWithRetry(String locationData) {
  if (!ensureWiFiConnected()) {
    Serial.println("❌ Email not sent: No WiFi connection.");
    return;
  }
  smtp.callback(smtpCallback);

  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  session.time.ntp_server = "pool.ntp.org,time.google.com,time.nist.gov";
  session.time.gmt_offset = 19800;
  session.time.day_light_offset = 0;

  SMTP_Message message;
  message.sender.name = "ESP32 Collision Alert";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "🚨 Collision Alert - ESP32 Vehicle";
  message.addRecipient("Owner1", RECIPIENT_EMAIL1);
  message.addRecipient("Owner2", RECIPIENT_EMAIL2);

  String html = "<h3>🚨 Collision Detected!</h3><p>A collision has been detected by your ESP32 vehicle system.</p><pre>" + locationData + "</pre>";
  message.html.content = html.c_str();
  message.html.transfer_encoding = "base64";
  message.text.charSet = "us-ascii";

  const int maxRetries = 3;
  int retryCount = 0;
  bool emailSent = false;

  while (!emailSent && retryCount < maxRetries) {
    retryCount++;
    Serial.printf("Attempt %d/%d: Connecting to SMTP server...\n", retryCount, maxRetries);
    if (!smtp.connect(&session)) {
      Serial.println("❌ Could not connect to mail server: " + smtp.errorReason());
      smtp.closeSession();
      if (retryCount < maxRetries) {
        Serial.println("Retrying in 2s...");
        delay(2000);
      }
      continue;
    }

    Serial.println("Sending email...");
    if (MailClient.sendMail(&smtp, &message)) {
      Serial.println("✅ Email sent successfully!");
      emailSent = true;
    } else {
      Serial.println("❌ Sending failed: " + smtp.errorReason());
      smtp.closeSession();
      if (retryCount < maxRetries) {
        Serial.println("Retrying in 2s...");
        delay(2000);
      }
    }
    smtp.closeSession();
  }

  if (!emailSent) {
    Serial.printf("❌ Email sending failed after %d retries. Continuing system operation.\n", maxRetries);
  }
}

bool setSystemTime() {
  Serial.println("⏱️ Syncing time via NTP...");
  configTime(19800, 0, "pool.ntp.org", "time.google.com", "time.nist.gov");
  struct tm timeinfo;
  int retries = 0, maxRetries = 20;
  while (!getLocalTime(&timeinfo) && retries < maxRetries) {
    Serial.print(".");
    delay(1000);
    retries++;
  }
  if (retries >= maxRetries) {
    Serial.println("\n❌ Time sync failed after " + String(maxRetries) + " retries.");
    return false;
  }
  Serial.println("\n✅ Time synced: " + String(timeinfo.tm_year + 1900) + "-" +
                 String(timeinfo.tm_mon + 1) + "-" + String(timeinfo.tm_mday) + " " +
                 String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec));
  return true;
}

bool ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) return true;
  Serial.println("🌐 WiFi disconnected! Attempting to reconnect...");
  WiFi.reconnect();
  int retries = 0, maxRetries = 10;
  while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
    Serial.print(".");
    delay(500);
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n🌐 WiFi reconnected. IP: " + WiFi.localIP().toString());
    return true;
  } else {
    Serial.println("\n❌ WiFi reconnection failed after " + String(maxRetries) + " retries.");
    return false;
  }
}

void testPing() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://8.8.8.8");
    http.setTimeout(5000);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("✅ Ping to 8.8.8.8 successful.");
    } else {
      Serial.println("❌ Ping to 8.8.8.8 failed. HTTP code: " + String(httpCode));
    }
    http.end();
  } else {
    Serial.println("❌ WiFi not connected for ping test.");
  }
}

void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
  if (status.success()) {
    Serial.println("📨 Email delivery confirmed by callback!");
  } else {
    Serial.println("Callback: Email not delivered yet.");
  }
}
