 #define CONDUCTPIN A1
 #define SERIESRESISTOR 10000
  
  void setup(void)
  {
    Serial.begin(9600);
  }
  void loop(void)
  {
  //Read value
  float average;
  average = analogRead(CONDUCTPIN);
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  Serial.print("Thermistor resistance "); 
  Serial.println(average);
 
  float conduct;
  conduct = (1 / average) * 1000000;
  Serial.print("Conductivity in microSiemens");
  Serial.println(conduct);
  delay(2000);
  }
 
