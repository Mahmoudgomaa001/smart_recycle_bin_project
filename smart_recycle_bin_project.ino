#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

const int TRIGGER_PIN = 2;
const int ECHO_PIN = 3;
const int PRESSURE_SENSOR_PIN = A0;
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;

const int MOTOR_A_EN = 6;
const int MOTOR_A_IN1 = 7;
const int MOTOR_A_IN2 = 8;
const int MOTOR_A_SPEED = 255;

const int GREEN_LED_PIN = 9;
const int YELLOW_LED_PIN = 10;
const int RED_LED_PIN = 11;
const int LIMIT_SWITCH_PIN = 12;
const int DOOR_SWITCH_PIN = 13;
const int SIM800L_TX_PIN = A1;
const int SIM800L_RX_PIN = A2;

const int LCD_RS = A3;
const int LCD_E = A4;
const int LCD_D4 = A5;
const int LCD_D5 = A6;
const int LCD_D6 = A7;
const int LCD_D7 = A8;

const int MAX_DISTANCE = 70;
const int MIN_DISTANCE = 30;
const int MAX_PRESSURE = 2;
const int PRESSURE_THRESHOLD = 1;
const int MAX_COMPRESSION_COUNT = 3;
const int OPEN_DELAY = 5000;
const int CLOSE_DELAY = 5000;

int compression_count = 0;
bool door_closed = false;
bool basket_full = false;
int pressure_value = 0;
int pressure = 0;
int count = 10;

SoftwareSerial sim800l(SIM800L_RX_PIN, SIM800L_TX_PIN);
String phoneNumber = "+1234567890";
String message = "تم ملء السلة";

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void updateBasketStatus(int distance);
void compressTrash();
void updateDoorStatus();
void updatePressure();
void sendAlertMessage();
void updateLCD(int distance);

void setup() {
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PRESSURE_SENSOR_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(MOTOR_A_EN, OUTPUT);
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(DOOR_SWITCH_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);

  sim800l.begin(9600);
  updateBasketStatus(0);
  updateDoorStatus();
}

void loop() {
  updateDoorStatus();
  int distance = getDistance();
  updateBasketStatus(distance);
  updateLCD(distance);
  delay(100);
}

void updateBasketStatus(int distance) {
  if (distance >= MAX_DISTANCE) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    basket_full = false;
  } else if (distance <= MIN_DISTANCE) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
    if (door_closed && !basket_full) {
      if (compression_count < MAX_COMPRESSION_COUNT) {
        compressTrash();
        compression_count++;
        delay(CLOSE_DELAY);
      } else {
        sendAlertMessage();
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
        basket_full = true;
      }
    }
  } else {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    basket_full = false;
  }
}

void compressTrash() {
  updatePressure();
  while (pressure < PRESSURE_THRESHOLD) {
    while (digitalRead(LIMIT_SWITCH_PIN) == HIGH) {
      digitalWrite(MOTOR_A_IN1, HIGH);
      digitalWrite(MOTOR_A_IN2, LOW);
      analogWrite(MOTOR_A_EN, MOTOR_A_SPEED);
    }
    digitalWrite(MOTOR_A_EN, LOW);
    delay(OPEN_DELAY);

    while (digitalRead(DOOR_SWITCH_PIN) == HIGH) {
      digitalWrite(MOTOR_A_IN1, HIGH);
      digitalWrite(MOTOR_A_IN2, LOW);
      analogWrite(MOTOR_A_EN, MOTOR_A_SPEED);
    }
    digitalWrite(MOTOR_A_EN, LOW);
    delay(CLOSE_DELAY);
    updatePressure();
  }
}

void updateDoorStatus() {
  door_closed = digitalRead(DOOR_SWITCH_PIN) == LOW;
}

void updatePressure() {
  pressure_value = analogRead(PRESSURE_SENSOR_PIN);
  pressure = (pressure_value / 1023.0) * MAX_PRESSURE;
}

void sendAlertMessage() {
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phoneNumber);
  sim800l.println("\"");
  delay(100);
  sim800l.print(message);
  delay(100);
  sim800l.write(26);
  delay(100);
}

int getDistance() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.0344 / 2;

  return distance;
}

void updateLCD(int distance) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fill Level: ");
  int fillPercentage = map(distance, MAX_DISTANCE, MIN_DISTANCE, 0, 100);
  lcd.print(fillPercentage);
  lcd.print("%");

  lcd.setCursor(0, 1);
  if (door_closed) {
    lcd.print("Door: CLOSED");
  } else {
    lcd.print("Door: OPEN");
  }
}
