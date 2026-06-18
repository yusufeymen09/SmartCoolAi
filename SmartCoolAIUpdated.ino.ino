#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo ventServo;

// ---- adaptive system ----
float targetTemp = 25.0;
float learningRate = 0.015;

// ---- tracking ----
float lastTemp = 0;
int lastSpeed = 90; // 90 = STOP for continuous rotation servo

// ---- compute servo speed (CR servo model) ----
int computeVentSpeed(float temp, float trend) {

  int speed;

  // 90 = STOP
  if (temp < 24) {
    speed = 90;     // no movement (stable)
  }
  else if (temp < 26) {
    speed = 95;     // slow cooling
  }
  else if (temp < 28) {
    speed = 110;    // medium cooling
  }
  else {
    speed = 130;    // strong cooling
  }

  // predictive behavior (react earlier if heating fast)
  if (trend > 0.4) {
    speed += 10;
  }

  return constrain(speed, 0, 180);
}

void setup() {
  Serial.begin(9600);

  dht.begin();

  lcd.init();
  lcd.backlight();

  ventServo.attach(9);
  ventServo.write(90); // STOP initially

  lcd.setCursor(0, 0);
  lcd.print("Smart Vent AI+");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);
}

void loop() {

  float temp = dht.readTemperature();

  // sensor check
  if (isnan(temp)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
    delay(2000);
    return;
  }

  // ---- trend (prediction) ----
  float tempTrend = temp - lastTemp;

  // ---- adaptive learning ----
  targetTemp = targetTemp * (1 - learningRate) + temp * learningRate;

  // ---- compute motor speed ----
  int speed = computeVentSpeed(temp, tempTrend);

  // ---- avoid jitter ----
  if (abs(speed - lastSpeed) > 2) {
    ventServo.write(speed);
    lastSpeed = speed;
  }

  // ---- system state ----
  String state;

  if (temp < 24) state = "STABLE";
  else if (temp > targetTemp + 2) state = "COOLING";
  else state = "ADJUSTING";

  // ---- LCD DISPLAY (clean UI) ----
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C S:");
  lcd.print(speed);

  lcd.setCursor(0, 1);
  lcd.print(state);

  // ---- Serial debug ----
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" | Trend: ");
  Serial.print(tempTrend);
  Serial.print(" | Speed: ");
  Serial.print(speed);
  Serial.print(" | Target: ");
  Serial.print(targetTemp);
  Serial.print(" | State: ");
  Serial.println(state);

  // update memory
  lastTemp = temp;

  delay(1500);
}