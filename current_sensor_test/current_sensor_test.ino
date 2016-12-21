float data1, data2;

void setup() {
  Serial.begin(115200);
}

void loop() {
  data1 = analogRead(A0);
  data2 = analogRead(A1);
  Serial.print(millis());
  Serial.print(",");
  Serial.print(((512-data1)*5)/(0.185*1024));
  Serial.print(",");
  Serial.print(((512-data2)*5)/(0.185*1024));
  Serial.println("");
  
  //Serial.println("");
}
