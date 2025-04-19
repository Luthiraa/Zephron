#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVO0 0
#define SERVO1 1
#define SERVO2 2
#define SERVO3 3
#define SERVO4 4

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


int servo0Pos = 410;
int servo1Pos = 365;
int servo2Pos = 310;
int servo3Pos = 500;
int servo4Pos = 710;


const int STEP = 15;
const int DELAY_TIME = 10; 


const int servo0Min = 310, servo0Max = 500;
const int servo1Min = 305, servo1Max = 700;
const int servo2Min = 270, servo2Max = 500;
const int servo3Min = 470, servo3Max = 760;
const int SERVO_MIN = 150, SERVO_MAX = 600;  

void setup() {
  Serial.begin(9600);
  
  pwm.begin();
  pwm.setPWMFreq(60); 
  delay(10);  
    pwm.setPWM(SERVO0, 0, servo0Pos);
  pwm.setPWM(SERVO1, 0, servo1Pos);
  pwm.setPWM(SERVO2, 0, servo2Pos);
  pwm.setPWM(SERVO3, 0, servo3Pos);
  pwm.setPWM(SERVO4, 0, servo4Pos);
  
  pinMode(2, INPUT);   
  pinMode(3, INPUT);   
  pinMode(6, INPUT);   
  pinMode(4, INPUT);   
  pinMode(7, INPUT);   
  pinMode(5, INPUT);   
  pinMode(10, INPUT);  
  pinMode(9, INPUT);   
  pinMode(12, INPUT);  
  pinMode(11, INPUT);  
}

void loop() {
  int servo0Inc = digitalRead(5);
  int servo0Dec = digitalRead(4);
  int servo1Inc = digitalRead(6);
  int servo1Dec = digitalRead(3);
  int servo2Inc = digitalRead(7);
  int servo2Dec = digitalRead(2);
  int servo3Inc = digitalRead(9);
  int servo3Dec = digitalRead(10);
  int servo4Inc = digitalRead(11);
  int servo4Dec = digitalRead(12);

  if (servo0Inc == HIGH) {
    servo0Pos = constrain(servo0Pos + STEP, servo0Min, servo0Max);
    pwm.setPWM(SERVO0, 0, servo0Pos);
    delay(DELAY_TIME);
  }
  if (servo0Dec == HIGH) {
    servo0Pos = constrain(servo0Pos - STEP, servo0Min, servo0Max);
    pwm.setPWM(SERVO0, 0, servo0Pos);
    delay(DELAY_TIME);
  }
  if (servo1Inc == HIGH) {
    servo1Pos = constrain(servo1Pos + STEP, servo1Min, servo1Max);
    pwm.setPWM(SERVO1, 0, servo1Pos);
    delay(DELAY_TIME);
  }
  if (servo1Dec == HIGH) {
    servo1Pos = constrain(servo1Pos - STEP, servo1Min, servo1Max);
    pwm.setPWM(SERVO1, 0, servo1Pos);
    delay(DELAY_TIME);
  }
  if (servo2Inc == HIGH) {
    servo2Pos = constrain(servo2Pos + STEP, servo2Min, servo2Max);
    pwm.setPWM(SERVO2, 0, servo2Pos);
    delay(DELAY_TIME);
  }
  if (servo2Dec == HIGH) {
    servo2Pos = constrain(servo2Pos - STEP, servo2Min, servo2Max);
    pwm.setPWM(SERVO2, 0, servo2Pos);
    delay(DELAY_TIME);
  }
  if (servo3Inc == HIGH) {
    servo3Pos = constrain(servo3Pos + STEP, servo3Min, servo3Max);
    pwm.setPWM(SERVO3, 0, servo3Pos);
    delay(DELAY_TIME);
  }
  if (servo3Dec == HIGH) {
    servo3Pos = constrain(servo3Pos - STEP, servo3Min, servo3Max);
    pwm.setPWM(SERVO3, 0, servo3Pos);
    delay(DELAY_TIME);
  }
  if (servo4Inc == HIGH) {
    servo4Pos = constrain(servo4Pos + STEP, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(SERVO4, 0, servo4Pos);
    delay(DELAY_TIME);
  }
  if (servo4Dec == HIGH) {
    servo4Pos = constrain(servo4Pos - STEP, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(SERVO4, 0, servo4Pos);
    delay(DELAY_TIME);
  }
}
