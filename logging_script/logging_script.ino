
/*
Conductivity Meter
Mark Hagemann and Jennifer Dargin
Modified from code on Riffle-ito github page,
SD card data logger
*/

// Libraries

#include <SPI.h>
#include <stdlib.h>
#include <Wire.h>
#include <SD.h>
#include <RTClib.h>


// Define pins
#define CONDUCTPIN A1
#define THERMISTORPIN A0
#define SERIESRESISTOR 10000
#define LOG_INTERVAL  3000 // mills between entries
#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

RTC_DS1307 RTC; // define the Real Time Clock object
 
 
// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;
 
// the logging file
File logfile;
 
void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  while(1);
}


void setup(void)
  {
  // Bridge.begin();
  Serial.begin(9600);
  //FileSystem.begin();
  Serial.println();
  
  
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);
  
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  
 
  logfile.println("millis,time,cond,temp");    
#if ECHO_TO_SERIAL
  Serial.println("millis,time,cond,temp");
#endif
// attempt to write out the header to the file
  //if (logfile.writeError || !logfile.sync()) {
    //error("write header");
  //}
  
 
   // If you want to set the aref to something other than 5v
  //analogReference(EXTERNAL);

}


//////////////////// LOOOP //////////////
  
void loop(void) 
  {
  
  DateTime now;
 
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  
  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");    
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(", ");  
#endif
  
  
  // fetch the time
  now = RTC.now();
  // log time
  logfile.print(now.secondstime()); // seconds since 2000
  logfile.print(", ");
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
#if ECHO_TO_SERIAL
  Serial.print(now.secondstime()); // seconds since 2000
  Serial.print(", ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
#endif //ECHO_TO_SERIAL
  
  // READ VALUES, CALCULATE OUTPUT
  //Read thermistor value
  float thermval;
  thermval = analogRead(THERMISTORPIN);
  // convert the value to resistance
  thermval = 1023 / thermval - 1;
  thermval = SERIESRESISTOR / thermval;
  Serial.println("Thermistor resistance:"); 
  Serial.println(thermval);
 
  // Calculate temperature
  float steinhart;
  char tstr [6];
  steinhart = thermval / 10000;     // (R/Ro)
  steinhart = log(steinhart);       // ln(R/Ro)
  steinhart /= 3950;                // 1/B * ln(R/Ro)
  steinhart += 1.0 / (25 + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;      // Invert
  steinhart -= 273.15;              // convert to C
  dtostrf(steinhart, 5, 2, tstr);
  Serial.println("Temperature (deg C)");
  Serial.println(steinhart);
 
  //Read conductivity meter value
  float condval;
  condval = analogRead(CONDUCTPIN);
  // convert the value to resistance
  condval = 1023 / condval - 1;
  condval = SERIESRESISTOR / condval;
  Serial.println("Conductivity meter resistance:"); 
  Serial.println(condval);
  
  // Adjust conductivity value based on temperature
  float tempdiff; // difference between temperature and 25 deg C
  float conduct;
  char condstr [6];
  conduct = (1 / condval) * 1000000;
  tempdiff = 25.0 - steinhart;
  conduct = conduct * (1 + tempdiff / 50);
  Serial.println("Specific Conductance in microSiemens per cm at 25 C:");
  Serial.println(conduct);
  dtostrf(conduct, 5, 2, condstr);
  
  logfile.print(", ");    
  logfile.print(condstr);
  logfile.print(", ");    
  logfile.println(tstr);
#if ECHO_TO_SERIAL
 // Serial.print(", ");   
  //Serial.print(condstr);
  //Serial.print(", ");    
  //Serial.println(tstr);
#endif //ECHO_TO_SERIAL
 
  
  }

 
 /* / This function return a string with the time stamp
String getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time 
  // in different formats depending on the additional parameter 
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
                                //             T for the time hh:mm:ss    
  time.run();  // run the command

  // read the output of the command
  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }

  return result;
}
*/
