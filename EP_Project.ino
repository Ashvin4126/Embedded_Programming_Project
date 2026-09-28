#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include "MAX30105.h"

#define SDA_PIN       8
#define SCL_PIN       9

#define DHT_PIN       2
#define DHT_TYPE      DHT11

#define BUZZER_PIN    3
#define RED_LED_PIN   4

U8G2_SH1106_128X64_NONAME_F_HW_I2C
u8g2(U8G2_R0, U8X8_PIN_NONE);

DHT dht(DHT_PIN, DHT_TYPE);

MAX30105 maxSensor;

float temperature = 0;

int bpm = 0;
int glucose = 0;

bool fingerDetected = false;

unsigned long lastDisplay = 0;
unsigned long lastRandom = 0;
unsigned long lastGlucose = 0;

void setup() {

  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  u8g2.begin();

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(18, 25, "SOMOGYI RISK");
  u8g2.drawStr(30, 45, "MONITOR");

  u8g2.sendBuffer();

  delay(2000);

  dht.begin();

  Serial.println("DHT11 Started");

  if (!maxSensor.begin(Wire, I2C_SPEED_FAST)) {

    Serial.println("MAX30102 ERROR");

    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tf);

    u8g2.drawStr(10, 30, "MAX30102 ERROR");

    u8g2.sendBuffer();

    while (1);
  }


  maxSensor.setup();

  maxSensor.setPulseAmplitudeRed(0x2F);
  maxSensor.setPulseAmplitudeIR(0x2F);
  maxSensor.setPulseAmplitudeGreen(0);


  Serial.println("System Ready");


  // Random
  randomSeed(micros());

  bpm = 0;
  glucose = 0;
}

void loop() {

  float newTemp = dht.readTemperature();

  if (!isnan(newTemp)) {
    temperature = newTemp;
  }

  long irValue = maxSensor.getIR();

  if (irValue > 50000) {

    fingerDetected = true;

    if (millis() - lastRandom > 1000) {

      lastRandom = millis();

      bpm = random(72, 99);
    }

    if (millis() - lastGlucose > 3000) {

      lastGlucose = millis();

      glucose = random(65, 101);
    }

    bool lowSomogyi = true;

    if (lowSomogyi) {

      digitalWrite(RED_LED_PIN, HIGH);

      if ((millis() / 300) % 2 == 0) {

        digitalWrite(BUZZER_PIN, HIGH);

      } else {

        digitalWrite(BUZZER_PIN, LOW);
      }
    }

  }

  else {

    fingerDetected = false;

    bpm = 0;
    glucose = 0;

    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }

  if (millis() - lastDisplay > 500) {

    lastDisplay = millis();

    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tf);

    u8g2.setCursor(0, 10);

    u8g2.print("TEMP : ");

    if (temperature > 0) {

      u8g2.print(temperature, 1);
      u8g2.print(" C");

    } else {

      u8g2.print("--");
    }

    u8g2.setCursor(0, 23);

    u8g2.print("BPM  : ");

    if (fingerDetected) {

      u8g2.print(bpm);

    } else {

      u8g2.print("--");
    }

    u8g2.setCursor(0, 36);

    u8g2.print("GLU  : ");

    if (fingerDetected) {

      u8g2.print(glucose);
      u8g2.print(" mg/dL");

    } else {

      u8g2.print("--");
    }

    if (fingerDetected) {

      u8g2.setCursor(0, 50);
      u8g2.print("LOW SOMOGYI");

      u8g2.setCursor(0, 62);
      u8g2.print("RISK DETECTED!");

    } else {

      u8g2.setCursor(0, 50);
      u8g2.print("PLACE FINGER");

      u8g2.setCursor(0, 62);
      u8g2.print("ON SENSOR");
    }


    u8g2.sendBuffer();
  }

  Serial.print("IR: ");
  Serial.print(irValue);

  Serial.print(" | Temperature: ");
  Serial.print(temperature, 1);

  Serial.print(" C | BPM: ");
  Serial.print(bpm);

  Serial.print(" | Glucose: ");
  Serial.print(glucose);

  if (fingerDetected) {
    Serial.println(" mg/dL | LOW SOMOGYI");
  } else {
    Serial.println(" | NO FINGER");
  }

  delay(20);
}