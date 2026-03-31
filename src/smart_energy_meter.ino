#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Energy Meter"
#define BLYNK_AUTH_TOKEN     "YOUR_AUTH_TOKEN"

#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EmonLib.h>
#include <BlynkSimpleEsp32.h>

/* ---------------- WIFI ---------------- */
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

/* ---------------- PINS ---------------- */
#define ZMPT_PIN  34
#define SCT_PIN   32

/* ---------------- LCD ----------------- */
#define I2C_ADDR  0x27
LiquidCrystal_I2C lcd(I2C_ADDR, 16, 2);

/* --------------- ENERGY --------------- */
EnergyMonitor emon1;

float voltageCalibration = 650.0;
float energy_kWh = 0.0;

unsigned long lastEnergyMillis = 0;

/* --------------- BLYNK ---------------- */
BlynkTimer timer;

/* -------- VOLTAGE MEASUREMENT --------- */
float readVoltage() {
  const int samples = 1000;
  double sumSquares = 0;

  for (int i = 0; i < samples; i++) {
    int raw = analogRead(ZMPT_PIN);
    float v = (raw / 4095.0) * 3.3;
    float centered = v - 1.65;
    sumSquares += centered * centered;
    delayMicroseconds(200);
  }

  float vrms = sqrt(sumSquares / samples) * voltageCalibration;
  if (vrms < 110) vrms = 0.0;
  return vrms;
}

/* -------- AVERAGED CURRENT ------------ */
double readCurrentAvg() {
  double total = 0;
  int readings = 5;

  for (int i = 0; i < readings; i++) {
    total += emon1.calcIrms(1480);
  }

  double avg = total / readings;

  if (avg < 0.02) avg = 0.0;
  return avg;
}

/* ------- READ + SEND DATA ------------- */
void sendData() {
  float voltage = readVoltage();
  double current = readCurrentAvg();

  if (voltage == 0) current = 0;

  float power = voltage * current;

  unsigned long now = millis();
  if (lastEnergyMillis == 0) lastEnergyMillis = now;
  float hours = (now - lastEnergyMillis) / 3600000.0;
  energy_kWh += (power / 1000.0) * hours;
  lastEnergyMillis = now;

  Serial.print("V: ");  Serial.print(voltage, 1);
  Serial.print("  I: "); Serial.print(current, 2);
  Serial.print("  P: "); Serial.print(power, 2);
  Serial.print("  E: "); Serial.println(energy_kWh, 4);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(voltage, 1);
  lcd.print(" I:");
  lcd.print(current, 2);

  lcd.setCursor(0, 1);
  lcd.print("P:");
  lcd.print(power, 1);
  lcd.print(" E:");
  lcd.print(energy_kWh, 3);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, voltage);
    Blynk.virtualWrite(V1, current);
    Blynk.virtualWrite(V2, power);
    Blynk.virtualWrite(V3, energy_kWh);
  }
}

/* -------------- SETUP ----------------- */
void setup() {
  Serial.begin(9600);

  Wire.begin(21, 22);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Booting...");

  emon1.current(SCT_PIN, 0.55);

  WiFi.begin(ssid, pass);

  Blynk.config(BLYNK_AUTH_TOKEN);

  timer.setInterval(2000L, sendData);

  delay(1500);
  lcd.clear();
}

/* -------------- LOOP ------------------ */
void loop() {
  Blynk.run();
  timer.run();
}
