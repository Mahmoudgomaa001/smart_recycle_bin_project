  Arduino due عدل الكود ليعمل على

 
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// تعريف المتغيرات
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 3;
const int PRESSURE_SENSOR_PIN = A0;
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;

// تعريف التوصيلات المستخدمة لتوصيل المحرك باستخدام L2930
const int MOTOR_A_EN = 6; // توصيلة تمكين المحرك A
const int MOTOR_A_IN1 = 7; // توصيلة إشارة الاتجاه 1 للمحرك A
const int MOTOR_A_IN2 = 8; // توصيلة إشارة الاتجاه 2 للمحرك A

const int GREEN_LED_PIN = 7;
const int YELLOW_LED_PIN = 8;
const int RED_LED_PIN = 9;
const int LIMIT_SWITCH_PIN = 10;
const int DOOR_SWITCH_PIN = 11;
const int SIM800L_TX_PIN = 12;
const int SIM800L_RX_PIN = 13;

const int LCD_RS = A1;
const int LCD_E = A2;
const int LCD_D4 = A3;
const int LCD_D5 = A4;
const int LCD_D6 = A5;
const int LCD_D7 = A6;

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

SoftwareSerial sim800l(SIM800L_RX_PIN, SIM800L_TX_PIN); // تعريف كائن SoftwareSerial للتواصل مع وحدة SIM800L

String phoneNumber = "+1234567890"; // رقم الهاتف المستلم
String message = "تم ملء السلة"; // نص الرسالة

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// تعريف الدوال
void updateBasketStatus(int distance);
void compressTrash();
void updateDoorStatus();
void updatePressure();
void sendAlertMessage();
void updateLCD(int distance);

void setup() {
  // تعريف المنافذ اللازمة
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PRESSURE_SENSOR_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(DOOR_SWITCH_PIN, INPUT_PULLUP);

  lcd.begin(16, 4);

  // بدء التواصل مع وحدة SIM800L
  sim800l.begin(9600);
  // تحديث حالة السلة والباب
  updateBasketStatus(0);
  updateDoorStatus();
}

void loop() {
  // تحديث حالة الباب والسلة
  updateDoorStatus();
  int distance = getDistance();
  updateBasketStatus(distance);
  updateLCD(distance);

  // تأخير لتفادي الاستجابة الزائدة
  delay(100);
}

// دالة لتحديث حالة السلة
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

// دالة للضغط على المحتويات
void compressTrash() {
  updatePressure();
  while (pressure < PRESSURE_THRESHOLD) {
    // تحريك المحرك لأسفل حتى يصل إلى حد حساس نهاية الشوط
    while (digitalRead(LIMIT_SWITCH_PIN) == HIGH) {
      digitalWrite(MOTOR_PIN, HIGH);
    }
    digitalWrite(MOTOR_PIN, LOW);
    delay(OPEN_DELAY);

    // تحريك المحرك لأعلى حتى يصل إلى حد حساس الباب
    while (digitalRead(DOOR_SWITCH_PIN) == HIGH) {
      digitalWrite(MOTOR_PIN, HIGH);
    }
    digitalWrite(MOTOR_PIN, LOW);
    delay(CLOSE_DELAY);
    updatePressure();
  }
}

// دالة لتحديث حالة الباب
void updateDoorStatus() {
  door_closed = digitalRead(DOOR_SWITCH_PIN) == LOW;
}

// دالة لتحديث قيمة الضغط
void updatePressure() {
  pressure_value = analogRead(PRESSURE_SENSOR_PIN);
  pressure = (pressure_value / 1023.0) * MAX_PRESSURE;
}


// دالة لإرسال رسالة تنبيه
void sendAlertMessage() {

  // إرسال أمر إدخال رقم الهاتف
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phoneNumber);
  sim800l.println("\"");
  delay(100);
  // إرسال نص الرسالة
  sim800l.print(message);
  delay(100);
  // إرسال حرف Ctrl+Z لإنهاء الرسالة وإرسالها
  sim800l.write(26);
  delay(100);
}

// دالة لقراءة المسافة بين المستشعر والمحتويات
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

// دالة لتحديث شاشة العرض
void updateLCD(int distance) {
  lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print("Distance: ");
//  lcd.print(distance);
//  lcd.print("cm");

  //  lcd.setCursor(0, 1);
  //  lcd.print("Pressure: ");
  //  lcd.print(pressure);
  //  lcd.print(" bar");

  lcd.setCursor(0, 0);
  if (door_closed) {
    lcd.print("Door is CLOSED");
  } else {
    lcd.print("Door is OPEN");
  }

  lcd.setCursor(0, 1);
  if (basket_full) {
    lcd.print("Basket is FULL");
  } else {
    lcd.print("Basket is OK");
  }

}


void setup() {
  // تعيين التوصيلات كإخراج
  pinMode(MOTOR_A_EN, OUTPUT);
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
}

void loop() {
  // تشغيل المحرك في اتجاه واحد
  digitalWrite(MOTOR_A_IN1, HIGH);
  digitalWrite(MOTOR_A_IN2, LOW);
  analogWrite(MOTOR_A_EN, 255); // تشغيل المحرك بالسرعة القصوى
  delay(2000); // انتظار لمدة 2 ثانية

  // إيقاف المحرك
  digitalWrite(MOTOR_A_EN, LOW);
  delay(2000); // انتظار لمدة 2 ثانية
}
