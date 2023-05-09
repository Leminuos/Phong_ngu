/*
Phòng ngủ: 
+ Vi điều khiển: Arduino Uno R3
+ Tổng quan: 
1 đèn, 1 relay, 1 nút nhấn => 2 chân digital
1 màn hình lcd + 1 cảm biến nhiệt độ lm35
1 động cơ bước, 1 driver a4988
*/
// Khai báo thư viện
#include <Arduino.h>
#include <Servo.h>

// Cấu hình chân
#define buttonLed 2            // Nút nhấn đèn
#define buttonAir 3           // Nút nhấn điều hoà
#define buttonRem 4           // Nút nhấn rèm
#define ledPin 5                 // Đèn
#define servoPin 6  
#define stepPin 7
#define stepEnable 8
#define stepDir 9
#define CBAS A0
#define CBND A1

// Trạng thái các thiết bị
boolean ledState = 0;          // Trạng thái đèn phòng ngủ 
boolean airState = 0;          // Trạng thái điều hoà
boolean remState = 0;          // Trạng thái rèm
boolean fireState = 0;         // Trạng thái báo cháy

// Các biến truyền/ nhận dữ liệu
String inputString = "";                  // Dữ liệu nhận
boolean stringComplete = false;           // Kiểm tra dữ liệu đã được truyền hết chưa
String sendString = "";                  // Dữ liệu gửi

// Các biến tạm
float temperature;                // Biến lưu giá trị nhiệt độ từ cảm biến LM35
unsigned long stepTime;           // Biến lưu thời gian delay giữa các bước
byte pos;                         // Biến dùng lưu góc quay Servo

Servo rem;                              // Khởi tạo đối tượng servo

void ledAndAir();           // Hàm bật tắt quạt và đèn qua nút nhấn
void airToTemp();           // Hàm điều chỉnh tốc độ theo nhiệt độ được lấy từ cảm biến LM35

void setup()
{
  Serial.begin(9600);
  Serial.flush();
  pinMode(ledPin, OUTPUT);       // Cấu hình LED là ngõ ra
  pinMode(stepPin, OUTPUT);
  pinMode(stepEnable, OUTPUT);
  pinMode(stepDir, OUTPUT);
  /*
  1 xung = 1 bước 1.8'
  5 xung = 5 bước 1.8' x 5 = 9'
  1 vòng 360' = 360/1.8 = 200 bước
  */
  pinMode(buttonLed, INPUT_PULLUP); // Cấu hình nút nhấn là ngõ vào pull-up
  pinMode(buttonAir, INPUT_PULLUP);
  pinMode(buttonRem, INPUT_PULLUP);
  pinMode(CBAS, INPUT);
  pinMode(CBND, INPUT);

  digitalWrite(ledPin, HIGH);         // Ban đầu tắt đèn
  digitalWrite(stepEnable, HIGH);      // Ban đầu điều hoà tắt
  rem.attach(servoPin);
}

void loop()
{ 
  while(Serial.available() > 0){
    char inChar = (char) Serial.read();
    inputString += inChar;
    if(inChar == '\n'){
      stringComplete = true;
    }
    if(stringComplete){
      //Serial.print("Data nhận:");
      //Serial.println(inputString);
      //================================================
      // Xử lý dữ liệu
      if(inputString.indexOf("L1PN") >= 0){
        //Serial.println("Đèn phòng ngủ bật!");
        digitalWrite(ledPin, 0);
        ledState = 1;
      }
      else if(inputString.indexOf("L0PN") >= 0){
        //Serial.println("Đèn phòng ngủ tắt!");
        digitalWrite(ledPin, 1);
        ledState = 0;
      }
      if(inputString.indexOf("A1PN") >= 0){
        //Serial.println("Điều hoà phòng ngủ bật!");
        digitalWrite(stepEnable, 0);
        airState = 1;
      }
      else if(inputString.indexOf("A0PN") >= 0){
        //Serial.println("Điều hoà phòng ngủ tắt!");
        digitalWrite(stepEnable, 1);
        airState = 0;
      }
      inputString = "";
      stringComplete = false;
    }
  }
  //----------------------------------------------------------------
  attachInterrupt(digitalPinToInterrupt(buttonLed), ledAndAir, FALLING);  // Khi trạng thái button chuyển từ mức điện áp cao xuống mức
  attachInterrupt(digitalPinToInterrupt(buttonAir), ledAndAir, FALLING);  // điện áp thấp thì kích hoạt hàm ledAndAir
  temperature = 5.0 * (analogRead(CBND)) * 100.0 / 1023.0;
  if(airState == 1){
    digitalWrite(stepDir, 1);  // Chiều quay
    airToTemp();
  }
}
void ledAndAir(){
  if(digitalRead(buttonLed) == LOW){
    delay(50);
    digitalWrite(ledPin, ledState);
    ledState =! ledState;
    Serial.println("L" + String(ledState) + "PN");
  }
  if(digitalRead(buttonAir) == LOW){
    delay(50);
    digitalWrite(stepEnable, airState);
    airState =! airState;
    Serial.println("A" + String(airState) + "PN");
  }
}
void runSpeed(unsigned long time){
  for (int i = 0; i < 200; i++) { // Chạy 200 bước
    digitalWrite(stepPin, HIGH); // Tín hiệu Step lên
    delayMicroseconds(time); // Delay thời gian giữa các bước
    digitalWrite(stepPin, LOW); // Tín hiệu Step xuống
    delayMicroseconds(time); // Delay thời gian giữa các bước
  }
}
void airToTemp(){
  if (temperature < 25) {
    stepTime = 50; // Nếu nhiệt độ dưới 25 độ C, delay 50ms giữa mỗi 
    runSpeed(stepTime);
    return;
  } else if (temperature < 30) {
    stepTime = 30; // Nếu nhiệt độ từ 25-30 độ C, delay 20ms giữa mỗi bước
    runSpeed(stepTime);
    return;
  } else {
    stepTime = 10; // Nếu nhiệt độ trên 30 độ C, delay 10ms giữa mỗi bước
    runSpeed(stepTime);
    return;
  }
}
