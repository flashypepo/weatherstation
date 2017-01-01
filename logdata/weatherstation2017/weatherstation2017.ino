/*
 * WeatherStation version 2017...
 * Shields: OLED display,  SHT30 temperature and humidity, SD-card
 * using NTP-time for RTC and SparcFun microOLED library
 * 
 * based upon WeMOS shields code from Sensorslot (Andreas Spies) 2016
 * 2016-1231 PePo new
 * work in progress: ! time-data must be expressed in two characters for h,m and s
 *                   √ include batterij-shield
 *                   √ datalogger on CD-card, frequency 1x 10 seconds
 *                   datalogger online (replacing existing WeatherStation 2016)
 *                   
 * WeMos SHT30 Shield uses: I2C
 * WeMos OLED Shield uses:  I2C 
 * WeMos Micro SD Shield uses: D5, D6, D7, D8, 3V3 and G
 *   The microSD shield uses SPI bus pins:
 *   D5 = CLK
 *   D6 = MISO
 *   D7 = MOSI
 *   D8 = CS
 */
#include <Wire.h>           // Include Wire if you're using I2C
#include <SNTPtime.h>       // get NTP-time
#include <WEMOS_SHT3X.h>    // SHT30-library
#include <SPI.h>            // SPI for microCD
#include <SD.h>             // SD-card library
#include <SFE_MicroOLED.h>  // Include the SparcFun SFE_MicroOLED library
#include "credentials.h"    // my credentials for access to Wifi network

// MicroOLED object declaration
#define PIN_RESET 255  //
#define DC_JUMPER 0  // I2C Addres: 0 - 0x3C, 1 - 0x3D
MicroOLED oled(PIN_RESET, DC_JUMPER);  // I2C Example

// SHT30 object declaration
SHT3X sht30(0x45);

// microSD declarations
const int chipSelect = D8;
File root;
String logRecord = ""; // datalogger data-record
String logFilename="sht30.txt";

// SNTP NL-time object
SNTPtime NTPnl("nl.pool.ntp.org"); // NL pool
/*
   The structure contains following fields:
   struct strDateTime
   {
   byte hour;
   byte minute;
   byte second;
   int year;
   byte month;
   byte day;
   byte dayofWeek;
   boolean valid;
   };
*/
strDateTime dateTime;

// Use these variables to set the year, month, day, hour, minute, seconds
// really necessay? why not directly from dateTime-struct
//int year = 2016;
//int month = 12;
//int day = 31;
//int hour = 17;
//int minute = 36;
//int second = 00;

// How fast do you want the time is updated?
// Set this to 10000 to get _about_ 1 second timing.
// Temperature measurement: 10 x CLOCK_SPEED -> _about_ 10 seconds interval
const int CLOCK_SPEED = 1000;
unsigned long lastDraw = 0; // for timing updates
unsigned long lastMeasurement = 0; // for temperature measurement

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("WeatherStation 2017 ...");
  
  oled.begin();     // Initialize the OLED
  oled.clear(PAGE); // Clear the display's internal memory
  oled.clear(ALL);  // Clear the library's display buffer
  oled.display();   // Display what's in the buffer (splashscreen)  

  // connect to WiFi
  connectToWifi(mySSID, myPASSWORD);

  // get the NTPtime...
  while (!NTPnl.setSNTPtime()) Serial.print("!"); // set internal clock
  Serial.println();
  Serial.println("Time set");

  dateTime = NTPnl.getTime(1.0, 1); // get time from internal clock
  NTPnl.printDateTime(dateTime);

  // SD-card initialization...
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  // TEST: print files on SD-card
  //displayFiles("/");

  oled.clear(PAGE); // Clear the display's internal memory
  oled.clear(ALL);  // Clear the library's display buffer
  oled.display();   // Display what's in the buffer (splashscreen)
}

// loop:
// current time is running every second
// T and H are displayed every 2 seconds
void loop() 
{
  // every CLOCK_SPEED ms... update date and time
  if (lastDraw + CLOCK_SPEED < millis()) {
    lastDraw = millis(); // save current time
    updateTimeNet();    // update time, if necessary...
    displayTime(dateTime.year, dateTime.month, dateTime.day
              , dateTime.hour, dateTime.minute, dateTime.second);
  }
    
  // every _about_ 10 seconds: new T and H measurement
  if (lastMeasurement + 10*CLOCK_SPEED < millis()) {
      lastMeasurement = millis(); // save current time
      sht30.get();        // get new SHT30 data...
      displaySensorData(sht30.cTemp, sht30.humidity); // display SHT30 data
      
      // data via the Serial port....
      Serial.print("Temperature in Celsius : ");
      Serial.println(sht30.cTemp);
      Serial.print("Temperature in Fahrenheit : ");
      Serial.println(sht30.fTemp);
      Serial.print("Relative Humidity : ");
      Serial.println(sht30.humidity);
      Serial.println();

      // save data in log file...
      logRecord = printDateTime(dateTime);
      logRecord += printSHT30(sht30.cTemp, sht30.fTemp, sht30.humidity);
      //Serial.println(logRecord);
      saveDataOn(logFilename,logRecord);
  } // endof last measurement

}

// write dataString to filename
// postcondition: dataString is written in file, file is closed
void saveDataOn(String filename, String dataString)
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(filename, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.print("Saved record to file: ");
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.print("Error opening: ");
    Serial.println(filename);
  }

}

// display time on OLED display...
// pre-condition: all time-data expressed in 2 chars.
void displayTime(int y, int mo, int d, int h, int mi, int s)
{
  // display time
  oled.setFontType(0);  // font5*17 i.e. 12 chars/row

  // format: day.month.year, day and month: each 2 chars width
  oled.setCursor(0, 5);
  if (d<10) oled.print(0);
  oled.print(d); oled.print(".");
  if (mo<10) oled.print(0);
  oled.print(mo); oled.print(".");
  oled.print(y);

  // format: hour:minutes:seconds, each 2 chars width
  oled.setCursor(0, 13);
  oled.print("  "); // align time at right side...
  if (h<10) oled.print(0);
  oled.print(h); oled.print(":");  
  if (mi<10) oled.print(0);
  oled.print(mi); oled.print(":"); 
  if (s<10) oled.print(0);
  oled.print(s);
    
  // Draw to the screen
  oled.display();
}

// display sensor-data on OLED display...
void displaySensorData(float t, float h)
{
  // display temperature: "20.95 C"
  oled.setFontType(1);  // font8*16 i.e. 8 chars/row, 3 rows)
  oled.setCursor(0, 23);
  oled.print(t); oled.print(" C");

  // display humidity: "44.14 %"
  oled.setFontType(1);  // font8*16 i.e. 8 chars/row, 3 rows)
  oled.setCursor(0, 36);
  oled.print(h); oled.print(" %");
  
  // Draw to the screen
  oled.display();
}

void connectToWifi(const char* ssid, const char* passwrd)
{
  //Serial.print("Connecting to Wifi..."); Serial.println(ssid);
  oled.clear(PAGE);
  oled.setFontType(0);  // font5*17 i.e. 12 chars/row, 3 rows)
  oled.setCursor(0,5);
  oled.print("Wifi...");
  oled.setCursor(0, 13);
  oled.print(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, passwrd);
  oled.setCursor(0, 23);
  while (WiFi.status() != WL_CONNECTED) {
    oled.print(".");
    oled.display();
    //Serial.print(".");
    delay(500);
  }
  oled.setCursor(0, 36);
  oled.print("connected!");
  oled.display();
  //Serial.println();
  //Serial.println("WiFi connected");
  // TODO: localIP
  delay(1000); // wait to show mesages on OLED clearly
}

void updateTimeNet() {
  dateTime = NTPnl.getTime(1.0, 1); // get time from internal clock
  /*
  year = dateTime.year;
  month = dateTime.month;
  day = dateTime.day;
  hour = dateTime.hour;
  minute = dateTime.minute;
  second = dateTime.second;
  */
}

// helper functions to make data-records for the datalogger...
String printDateTime(strDateTime _dateTime) {
  char buffer[30];
  //               y    mo  d   wd  h   mi  s
  sprintf(buffer, "%4d,%2d,%2d,%1d,%2d,%2d,%2d"
          , _dateTime.year, _dateTime.month,_dateTime.day,_dateTime.dayofWeek
          , _dateTime.hour, _dateTime.minute,_dateTime.second);
  return String(buffer);
}

// A float to string: thanks to http://forum.arduino.cc/index.php?topic=85523.0 
// from #include<stdlib.h>
String printSHT30(float cT, float fT, float h)
{
  String ds = ","; // start with a delimiter
  char buffer[10];
  ds += dtostrf(cT,4,2,buffer);
  ds += ",";
  ds += dtostrf(fT,4,2,buffer);
  ds += ",";
  ds += dtostrf(h,4,2,buffer);
  return ds;
}

