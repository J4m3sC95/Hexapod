#include <Servo.h>

Servo servo;

unsigned long start_time = 0;
unsigned long end_time = 0;
unsigned long move_time = 0;

void setup() {
  // put your setup code here, to run once:
  servo.attach(5);
  servo.write(0);
  pinMode(12, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(digitalRead(12));
  start_time = millis();
  servo.write(180);
  while(!digitalRead(12));
  end_time = millis();

  move_time = end_time - start_time;

  Serial.println(move_time);
  delay(1000);
  servo.write(0);
  
}
