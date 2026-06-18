#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo ventServo;

bool coolingState = false;

void setup() {

  Serial.begin(9600);

  dht.begin();

  lcd.init();
  lcd.backlight();

  ventServo.attach(9);

  // Start closed
  ventServo.write(0);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SmartCool AI");
  lcd.setCursor(0,1);
  lcd.print("Starting...");
  delay(2000);
}

void loop() {

  float temp = dht.readTemperature();

  if (isnan(temp)) {

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sensor Error");

    Serial.println("Sensor Error");

    delay(2000);
    return;
  }

  // Hysteresis Logic

  if(temp > 26 && !coolingState){

    ventServo.write(0);
    coolingState = true;

    Serial.println("Cooling ON");
  }

  if(temp < 24 && coolingState){

    ventServo.write(90);
    coolingState = false;

    Serial.println("Cooling OFF");
  }

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C");

  lcd.setCursor(0,1);

  if(coolingState){
    lcd.print("Vent: OPEN");
  }
  else{
    lcd.print("Vent: CLOSED");
  }

  Serial.print("Temperature: ");
  Serial.println(temp);

  delay(2000);
}