float data;

void setup() {
  Serial.begin(250000);
}

void loop() {
  data = analogRead(A0);
  //Serial.print("Raw: \t");
  //Serial.print(analogRead(A0));
  //Serial.print(",\tcurrent: ");
  //Serial.print(", ");
  Serial.print(((512-data)*5)/(0.185*1024));
  Serial.print(",");
  Serial.println(millis());
  //Serial.println("");
}
