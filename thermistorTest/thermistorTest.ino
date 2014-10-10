 #define THERMISTORPIN A1
 #define SERIESRESISTOR 10000
  
  void setup(void)
  {
    Serial.begin(9600);
  }
  void loop(void)
  {
  //Read value
  float average;
  average = analogRead(THERMISTORPIN);
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  Serial.print("Thermistor resistance "); 
  Serial.println(average);
 
 
 
 
  float steinhart;
  steinhart = average / 10000;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= 3950;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (25 + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 
  Serial.print("Temperature "); 
  Serial.print(steinhart);
  Serial.println(" *C");
  delay(5000);
  }
 
