#include <LiquidCrystal.h>
#include <DFRobot_HX711.h>

// Define the pin numbers for the components
const int TRIGGER_PIN = 12;
const int ECHO_PIN = 13;

const int LOADCELL_DOUT_PIN = 7;
const int LOADCELL_SCK_PIN = 6;
const int MOTOR_A_EN = 10;
const int MOTOR_A_IN1 = 18;
const int MOTOR_A_IN2 = 20;
const int motorSpeed = 255;

const int GREEN_LED_PIN = 16;
const int YELLOW_LED_PIN = 10;
const int RED_LED_PIN = 17;
const int LIMIT_SWITCH_PIN = 14;
const int DOOR_SWITCH_PIN = 15;
const int SIM800L_TX_PIN = 19;
const int SIM800L_RX_PIN = 18;
const int LCD_RS = 5;
const int LCD_E = 4;
const int LCD_D4 = 3;
const int LCD_D5 = 2;
const int LCD_D6 = 1;
const int LCD_D7 = 0;

// Define the values for the distance and pressure thresholds, as well as the maximum compression count and delay times
const int MAX_DISTANCE = 75;
const int MIN_DISTANCE = 30;
const int MAX_PRESSURE = 150;
const int PRESSURE_THRESHOLD = 1;
const int MAX_COMPRESSION_COUNT = 3;
const int OPEN_DELAY = 5000;
const int CLOSE_DELAY = 5000;

// Declare variables to keep track of the system state
int compression_count = 0;
bool door_closed = false;
bool basket_full = false;
bool is_compressing = false;
int pressure_value = 0;
int pressure = 0;
int count = 10;


DFRobot_HX711 MyScale(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);


// Define the phone number and message for the alert message
String phoneNumber = "+1234567890";
String message = "تم ملء السلة";

// Create an instance of the LiquidCrystal library
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Declare function prototypes
void updateBasketStatus(int distance);
void compressTrash();
void updateDoorStatus();
void updatePressure();
void sendAlertMessage();
void updateLCD(int distance);

void setup() {
  // Initialize the pin modes for the components
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  //  pinMode(PRESSURE_SENSOR_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(MOTOR_A_EN, OUTPUT);
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(DOOR_SWITCH_PIN, INPUT_PULLUP);

  // Initialize the LCD display
  lcd.begin(16, 2);

  // Initialize the Serial1 communication for the SIM800L module
  Serial1.begin(9600);

  // Update the initial system state
  updateBasketStatus(0);
  updateDoorStatus();
}

void loop() {
  // Continuously update the system state
  updateDoorStatus();
  int distance = getDistance();
  updateBasketStatus(distance);
  updateLCD(distance);
  delay(100);
}

void updateBasketStatus(int distance) {
  // Update the system state based on the distance value
  if (distance >= MAX_DISTANCE) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    basket_full = false;
    is_compressing = false;
  } else if (distance <= MIN_DISTANCE) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
    if (door_closed && !basket_full) {
      if (compression_count < MAX_COMPRESSION_COUNT) {
        compressTrash();
        compression_count++;
        is_compressing = true;
        delay(CLOSE_DELAY);
      } else {
        sendAlertMessage();
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
        basket_full = true;
        is_compressing = false;
      }
    }
  } else {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    basket_full = false;
    is_compressing = false;
  }
}

void compressTrash() {
  // Define the variables for the motor speed and direction

  int motorDir1 = HIGH;
  int motorDir2 = LOW;

  // Compress the trash and update the pressure value
  updatePressure();

  if (!is_compressing) {
    Serial.println("Compressing trash...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Compressing trash");
    is_compressing = true;
  }

  // Set the motor direction and speed based on the pressure value
  if (pressure < MAX_PRESSURE) {
    `
    motorDir1 = HIGH;
    motorDir2 = LOW;
    //    motorSpeed = 150;  // Set the motor speed to 150 (out of 255)
  } else {
    motorDir1 = LOW;
    motorDir2 = LOW;
    
    return;
    //    motorSpeed = 0;  // Stop the motor
  }

  // Set the motor direction
  digitalWrite(MOTOR_A_IN1, motorDir1);
  digitalWrite(MOTOR_A_IN2, motorDir2);

  // Set the motor speed using PWM
  analogWrite(MOTOR_A_EN, motorSpeed);

  // Wait for the limit switch to be pressed
  //  while (digitalRead(LIMIT_SWITCH_PIN) == HIGH) {
  //    // Do nothing
  //  }

  // Reverse the motor direction
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, HIGH);
  delay(1000);

  // Set the motor speed using PWM
  analogWrite(MOTOR_A_EN, motorSpeed);

  // Wait for the limit switch to be released
  //  while (digitalRead(LIMIT_SWITCH_PIN) == LOW) {
  //    // Do nothing
  //  }

  // Stop the motor
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  digitalWrite(MOTOR_A_EN, LOW);

  // Update the pressure value
  updatePressure();

  // Reset the compression count
  compression_count = 0;

  // Update the LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Trash compressed");
  lcd.setCursor(0, 1);
  lcd.print("Pressure: ");
  lcd.print(pressure);
  lcd.print(" kPa");

  // Wait for 5 seconds
  delay(5000);

  // Update the system state
  is_compressing = false;
  updateBasketStatus(getDistance());
}
void updateDoorStatus() {
  // Update the status of the door
  if (digitalRead(DOOR_SWITCH_PIN) == LOW) {
    door_closed = true;
  } else {
    door_closed = false;
  }
  door_closed = true;
}

void updatePressure() {
  // Update the pressure value
  int pressure_sum = 0;

  for (int i = 0; i < count; i++) {
    pressure_value = MyScale.readWeight();
    pressure_sum += pressure_value;
  }

  pressure = pressure_sum / count;
}

void sendAlertMessage() {
  // Send an alert message
  Serial1.println("AT+CMGF=1");
  delay(100);

  Serial1.print("AT+CMGS=\"");
  Serial1.print(phoneNumber);
  Serial1.println("\"");
  delay(1000);

  Serial1.print(message);
  delay(100);

  Serial1.write(26);
  delay(1000);
}

void updateLCD(int distance) {
  // Calculate the fill percentage
  int fillPercentage = map(distance, 0, MAX_DISTANCE, 0, 100);

  // Update the LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fill Percentage:");
  lcd.setCursor(0, 1);
  lcd.print(fillPercentage);
  lcd.print("%");
}

int getDistance() {
  // Get the distance value from the ultrasonic sensor
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration / 58;

  return distance;
}
