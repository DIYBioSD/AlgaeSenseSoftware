// Version 2.03
// #include <Time.h>
/*  Wiring Diagram
PH sensors
  Probe 1 set to A1
  Probe 2 set to A2
Optical Sensors
   in line max 3 sensors, different addresses depending on ADDR pin floating, grounded or VCC'd
   SDA/SCL pins 20/21 
Temp Sensors DS1820B
   - in line single pin = D2
DHT Temp/Humidity Sensor
  Set to D3
*/
#include <SD.h> // needed to write to card
#include <OneWire.h> //needed for onewire temp DS18B20 temp sensor
#include <DallasTemperature.h> //needed for onewire DS18B20 temp sensor
#include "DHT.h" //needed for External temp/humidity sensor
//#include "ST7565.h" //needed for LCD
#include <Wire.h>  // for optical sensor lib
//#include "TSL2561.h" //optical sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <EEPROM.h>  // to read unique ID assigned to each arduino

// Set the output pin to 15 for the relay to control the heating pad (120VAC extesion cord)
// And set the min/max water temps
const byte HEATER_SWITCH_PIN= 11;
int curHeaterSetting = HIGH;  // start with heater on
const int maxWaterTemp = 26;
const int minWaterTemp = 24;

/* TSL2561 optical sensor MEGA2560 setup!
// connect GREEN  SCL to analog 21
// connect YELLOW SDA to analog 20
// connect RED VDD to 3.3V DC
// connect BLACK GROUND to common ground
   I2C Address
   ===========
   The address will be different depending on whether you leave
   the ADDR pin floating (addr 0x39), or tie it to ground or vcc. 
   The default addess is 0x39, which assumes the ADDR pin is floating
   (not connected to anything).  If you set the ADDR pin high
   or low, use TSL2561_ADDR_HIGH (0x49) or TSL2561_ADDR_LOW
   (0x29) respectively.
    You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example)
*/
int arduino_ID = 0;
uint16_t broadband = 0;
uint16_t infrared = 0;
Adafruit_TSL2561_Unified tsl_0 = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_TSL2561_Unified tsl_2 = Adafruit_TSL2561_Unified(TSL2561_ADDR_HIGH, 12345);
Adafruit_TSL2561_Unified tsl_1 = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 54321);

/*
// LCD Screen declarations
int ledPin =  13;    // LED connected to digital pin 13
// the LCD backlight is connected up to a pin so you can turn it on & off
// pin 9 - Serial data out (SID)
// pin 8 - Serial clock out (SCLK)
// pin 7 - Data/Command select (RS or A0)
// pin 6 - LCD reset (RST)
// pin 5 - LCD chip select (CS)
ST7565 glcd(9, 8, 7, 6, 5);
const byte LOGO16_GLCD_HEIGHT =16; 
const byte LOGO16_GLCD_WIDTH  =16;
*/

// DHT External Temp/Humidity Sensor Declarations
const byte DHTPIN =3;     // Digital pin for DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);
float dhtHumidReading;
float dhtTempReading;

// Onewire DS18B20 Temp sensor declarations (internal in tank)
const byte ONE_WIRE_BUS= 2;  // D2  pin for One Wire DS18B20 temp sensor;
float tempReading1, tempReading2;
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature lib.
DallasTemperature sensors(&oneWire);

// the logging file
File dataFile;
char fileName[] = "DATA00.txt";
const byte chipSelect = 4; // for the SD card to write

// pH sensor declarations
const byte PHprobe1 = 1; // set probe 1 to A1
const byte PHprobe2 = 2; // set probe 2 to A2
float waterTemp1 = 0; //not byte
float waterTemp2 = 0; //not byte
int rawSensorValue1 = 0; // was float
float sensorValue1 = 0;
int rawSensorValue2 = 0; // was float
float sensorValue2 = 0;
float pH_Calculated1 = 0;
float pH_Calculated2 = 0;
//float pH_CalcTemp = 0;
const byte numSamples = 10;

// Optical Sensor declarations
uint32_t luminosity;
uint16_t irLum1, irLum2, irLum3, fullSpectrumLum1, fullSpectrumLum2, fullSpectrumLum3, lux1, lux2, lux3, outsideFullSpectrum, outsideLux, outsideIR;

// Other declarations  
long timeFromStart=0; // used to track time 
const long timeDelay= 600000; //10000// milliseconds for delay
long hoursPassed=0;
long minutesPassed=0;
long secondsPassed=0;


void setup(void)
{
  arduino_ID = EEPROM.read(0);
  // initialize serial comm
  serCommInit();

  // initialize sensor DHT22 External/Outside Temp/Humidity Sensor Declarations
  pinMode(DHTPIN, OUTPUT);
  dht.begin();
  // Init the HEATER_SWITCH_PIN for the heating pad
  pinMode(HEATER_SWITCH_PIN, OUTPUT);
  digitalWrite(HEATER_SWITCH_PIN, HIGH); //turn heater on as default
  
  /*
  // initialize LCD display and set the contrast to 0x18
  glcd.begin(0x18);
  glcd.clear(); 
  //  glcd.display();
*/
  // Initalize Onewire DS18B20 Temp sensor for internal tank temp  3.0V ~ 5.5V
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature mesurement

    // Initialize the optical Sensor
  if (!tsl_0.begin()) { 
   // Serial.println("Optical sensor unavailable");  
  }  
  /* You can also manually set the gain or enable auto-gain support */
   tsl_0.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
   tsl_1.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
   tsl_2.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */  
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
// tsl.enableAutoGain(true);          /* Auto-gain ... switches automatically between 1x and 16x */
//  tsl_0.enableAutoGain(true);          /* Auto-gain ... switches automatically between 1x and 16x */
//  tsl_1.enableAutoGain(true);          /* Auto-gain ... switches automatically between 1x and 16x */  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
 //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
//  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
  tsl_0.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
  tsl_1.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
  tsl_2.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
}

void loop(void)
{
//  Serial.print("Free Ram:");
//  Serial.println(freeRAM(), DEC);
  //clear variables
  rawSensorValue1 = 0;
  sensorValue1=0;
  rawSensorValue2 = 0;
  sensorValue2=0;
  hoursPassed=(timeFromStart/3600);
  minutesPassed=(timeFromStart-(hoursPassed*3600))/60;
  secondsPassed=(timeFromStart-(hoursPassed*3600)-(minutesPassed*60));

  //check Internal DS18B20 temp Sensor
  readOnewire();

  //check pH Sensor
  readpHsensor();

  // Check External/Outside DHT temp/humidity sensor reading 
  readDHT();

  // Optical sensor collection
  readOpticalSensor();

  // Output to the serial port and the SD card
  outputToCard();

 // gLCDoutput();    

  delay(timeDelay);
  // store the current time in time variable t
//  timeFromStart =now();
}


void serCommInit()
{
  Serial.begin(9600); 
  // SD Card init  
//  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to output, even if you don't use it: See if the card is present and can be initi
lized:
  if (!SD.begin(chipSelect)) { 
 //   Serial.println("Card failed, or not present");
  }
  else
  {
    dataFile = SD.open(fileName, FILE_WRITE);  
    if (! dataFile) {
 //     Serial.println("Unable to create file");
      return; 
    } 
    else  { 
      dataFile.close();   
    }
    return;
  }
}

void readOnewire()
{
  // Onewire DS18B20 Temp sensor to issue a global temperature request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
//  Serial.print("Temperature for Device 1 is: ");
  tempReading1 = sensors.getTempCByIndex(0); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  tempReading2 = sensors.getTempCByIndex(1);
//  Serial.println(tempReading);
  waterTemp1 =tempReading1;
  
  // Now turn the heating pad on or off depending on the internal tank temp
  if (waterTemp1 > maxWaterTemp && curHeaterSetting == HIGH) { 
    digitalWrite(HEATER_SWITCH_PIN, LOW);
    curHeaterSetting = LOW;
  }
  else {
     if (waterTemp1 < minWaterTemp && curHeaterSetting == LOW) { 
       digitalWrite(HEATER_SWITCH_PIN, HIGH);
       curHeaterSetting = HIGH;
     }
    }
}

void readpHsensor()
{
  byte i;
  // Get pH reading from analog pin PHprobe
  for(i = 1;i <= numSamples;i++) {
    rawSensorValue1 = analogRead(PHprobe1);
   // Serial.println(rawSensorValue);
    sensorValue1=sensorValue1 + rawSensorValue1; ///4.095;
    delay(100);
  } 
  // Now calculate the pH - average over the number of reads
  sensorValue1 = sensorValue1/numSamples; 

  /*  sensorValue = SensorValue/4.095;
  // pH_Calculated = 0.0178 * sensorValue - 1.889;
  // Formula above came from vendor, but I tested against
 //  known samples and used the following website to calc a 
  // Linear Regression formula
  // http://easycalculation.com/statistics/regression.php
  // X value should be the sensorvalue; y value = the correct ph; then use the formula but need to 
  incorporate the temperature into the slope factor (so if it shows 0.18x, then = 0.18/watertemp; use that
  number for the factor in the equation below
  Serial.println(waterTemp);
  Serial.println(sensorValue);
 
  // as of 1/4/14, this is the only equation needed cuz it incorporates the temperature into the calc
  //pH_Calculated = (-2.251+(sensorValue*0.0009473*waterTemp)); //revised 1/6/14 Didn't work. Bad when temp changes from 20!
  //pH_Calculated = 7 - ((2.5-(sensorValue/200))/(0.257179+0.000941468 * waterTemp));
  // 1/7/14 - can't seem to figure out how to do this forumla correctly. I just used values I calculated and placed in their formula
  // see http://www.phidgets.com/phorum/viewtopic.php?f=7&t=5673
//  pH_CalcTemp = 7 - ((2.5-(sensorValue/200))/(0.2251+0.0009473 * waterTemp)); // revised 1/6/14
 // pH_Calculated = 7 - ((2.5-(sensorValue/200))/(0.257179+0.0009473 * waterTemp)); // revised 1/6/14  USING THIS ONE!!
 */
 pH_Calculated1 = 0.98*(0.0178 * sensorValue1 - 1.889);  // Using this one 96% of value FUDGE FACTOR!
 
 // Second Probe
// Only read if the board is not 111 or 222
if (arduino_ID != 111 & arduino_ID != 222)
 {
 for(i = 1;i <= numSamples;i++) {
    rawSensorValue2 = analogRead(PHprobe2);
    sensorValue2=sensorValue2 + rawSensorValue2; ///4.095;
    delay(100);
  } 
    // Now calculate the pH - average over the number of reads
  sensorValue2 = sensorValue2/numSamples; 
  pH_Calculated2 = 0.98*(0.0178 * sensorValue2 - 1.889);  // Using this one 96% of value FUDGE FACTOR!
 }
else  {
   sensorValue2=0;
   pH_Calculated2 = 0;
    Serial.print("");
   }   
}

void readDHT()
{
   dhtHumidReading = dht.readHumidity();
   dhtTempReading = dht.readTemperature();
  // check if returns are valid, if they are NaN (not a number) then something went wrong!     
 /* if (isnan(dhtTempReading) || isnan(dhtHumidReading)) {
 //   Serial.println("Failed to read from DHT");
  }
  else { 
    return;
  }*/
}
void readOpticalSensor()
{

 /* Get a new sensor event */ 
 // sensors_event_t event;
 // tsl.getEvent(&event);
   /* Get a new sensor event */ 
  sensors_event_t event1, event2, event3;
  tsl_0.getEvent(&event1);
  tsl_1.getEvent(&event2);
  tsl_2.getEvent(&event3);
  /* Display the results (light is measured in lux) */
  if (event1.light)
  {
    lux1=event1.light;
  }
  else {
  
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
   // Serial.println("No Data optical 1");
  }
    /* Display the results (light is measured in lux) */
  if (event2.light)
  {
    lux2=event2.light;
  }
  else {
  
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
   // Serial.println("No Data optical 2");
  }
      /* Display the results (light is measured in lux) */
  if (event3.light)
  {
    lux3=event3.light;
  }
 /* // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum. That way you can do whatever math and comparions you want!
  luminosity = tsl.getFullLuminosity();
  irLum = luminosity >> 16;
  fullSpectrumLum = luminosity & 0xFFFF;
*/
/* Populate broadband and infrared with the latest values */
//
tsl_0.getLuminosity (&irLum1, &fullSpectrumLum1);
tsl_1.getLuminosity (&irLum2, &fullSpectrumLum2);
tsl_2.getLuminosity (&irLum3, &fullSpectrumLum3);
/*if (!(irLum2==65535 && fullSpectrumLum2==65535 && lux2 == 0)) {
  outsideIR=irLum2;
  outsideFullSpectrum=fullSpectrumLum2;
  outsideLux=lux2;
  } */
}

/*
void gLCDoutput()
{
  char tempTxt[6]; // Arbitrary size, but make sure it is large enough to hold the string you are creating and a one byte null-terminator.
  char charBuf[6]=""; 
  glcd.clear();
  glcd.display();
  
glcd.drawstring(15, 0, " pH = ");
  dtostrf(pH_Calculated,5,2,tempTxt);
  sprintf(charBuf, "%u", tempTxt);
  glcd.drawstring(40, 0, tempTxt);
 
  glcd.drawstring(20, 1, "Out Temp");
  dtostrf(dhtTempReading,2,0,tempTxt);
  sprintf(charBuf, "%u", tempTxt);
  glcd.drawstring(80, 1, tempTxt);
  
  glcd.drawstring(20, 2, "Tank Temp");
  dtostrf(tempReading,4,2,tempTxt);
  sprintf(charBuf, "%u", tempTxt);
  glcd.drawstring(75, 2, tempTxt);
  
  glcd.drawstring(20, 3, "IR Light: ");
  dtostrf(irLum,4,0,tempTxt);
  sprintf(charBuf, "%u", tempTxt);
  glcd.drawstring(80, 3, tempTxt);
 
  glcd.drawstring(20, 4, "Full Light: ");
  dtostrf(fullSpectrumLum,4,0,tempTxt);
  sprintf(charBuf, "%u", tempTxt);
  glcd.drawstring(85, 4, tempTxt);
  
  glcd.drawstring(20, 5, "Vis Light: ");
  dtostrf(fullSpectrumLum - irLum,4,0,tempTxt);
  sprintf(charBuf, "%u", tempTxt);
  glcd.drawstring(85, 5, tempTxt);
  
  glcd.drawstring(20, 6, "Lux: ");
  dtostrf(tsl.calculateLux(fullSpectrumLum, irLum),4,0,tempTxt);
  sprintf(charBuf, "%u", tempTxt);
  glcd.drawstring(75, 6, tempTxt);
  glcd.display();
}
*/

void outputToCard()
{
  // Print other readings to the serial window and the card
  Serial.print(",ID:");
  Serial.print(arduino_ID, DEC);
  Serial.print("  Ext. Temp:");
  Serial.print(dhtTempReading);
  Serial.print(" Humidity:");
  // 
  Serial.print(dhtHumidReading);
  Serial.print(" Tank Temps 1:");
  Serial.print(tempReading1);
  Serial.print(" 2:");
  Serial.print(tempReading2);
 /* Serial.print(" Last Raw Sensor Value:  ");
  Serial.print(rawSensorValue);
  Serial.print("\n");*/
  Serial.print(" pH Vals  1:");
  Serial.print(sensorValue1);
 Serial.print(" 2:");
  Serial.print(sensorValue2);
  Serial.print(" pH 1:");
  Serial.print(pH_Calculated1);
   Serial.print(" 2:");
  Serial.print(pH_Calculated2);
 // Serial.print("\n");
 // Serial.print("pH 2nd Forumula:  ");
 // Serial.print(pH_CalcTemp);
//  Serial.print("\n");
/*  Serial.print(" Elapsed Time=");
  Serial.print(hoursPassed);
  Serial.print( ":");
  Serial.print(minutesPassed);
  Serial.print( ":");
 // Serial.println(secondsPassed);
 Serial.print(secondsPassed);
 */
  Serial.print(" IR 1:"); 
  Serial.print(irLum1);   
  Serial.print(" Full 1:"); 
  Serial.print(fullSpectrumLum1);   
  Serial.print(" Lux 1:"); 
  Serial.print(lux1);
  Serial.print(" IR 2:"); 
  Serial.print(irLum2);   
  Serial.print(" Full 2:"); 
  Serial.print(fullSpectrumLum2);   
  Serial.print(" Lux 2:"); 
  Serial.print(lux2);
  Serial.print(" IR 3:"); 
  Serial.print(irLum3);   
  Serial.print(" Full 3:"); 
  Serial.print(fullSpectrumLum3);   
  Serial.print(" Lux 3:"); 
  Serial.print(lux3);
  
 // Now print just the data, comma delimited
  Serial.print(",");
  Serial.print(arduino_ID, DEC);
  Serial.print(",");
  Serial.print(dhtTempReading);
  Serial.print(",");
  Serial.print(dhtHumidReading);
  Serial.print(",");
  Serial.print(tempReading1);
  Serial.print(",");
  Serial.print(sensorValue1);
  Serial.print(",");
  Serial.print(pH_Calculated1);
  Serial.print(",");
  Serial.print(irLum1);   
  Serial.print(",");
  Serial.print(fullSpectrumLum1);   
  Serial.print(",");
  Serial.print(lux1);
  Serial.print(",");
  Serial.print(irLum2);   
  Serial.print(",");
  Serial.print(fullSpectrumLum2);   
  Serial.print(",");
  Serial.print(lux2);
  Serial.print(",");
  Serial.print(tempReading2);
  Serial.print(",");
  Serial.print(sensorValue2);
  Serial.print(",");
  Serial.print(pH_Calculated2);
  Serial.print(",");
  Serial.print(irLum3);   
  Serial.print(",");
  Serial.print(fullSpectrumLum3);   
  Serial.print(",");
  Serial.println(lux3);
  /*
  if (irLum2==65535 && fullSpectrumLum2==65535 && lux2==0) Serial.print(outsideIR);
    else Serial.print(irLum2);
      Serial.print(",");
  if (irLum2==65535 && fullSpectrumLum2==65535 && lux2==0) Serial.print(outsideFullSpectrum);
    else Serial.print(fullSpectrumLum2);
      Serial.print(",");
  if (irLum2==65535 && fullSpectrumLum2==65535 && lux2==0) Serial.println(outsideLux);
    else Serial.println(lux2);  
    */
 /* dataFile = SD.open(fileName, FILE_WRITE); 
  if (dataFile) {
    dataFile.print("Time Elapsed = ");
    dataFile.print(hoursPassed);
    dataFile.print(":");
    dataFile.print(+minutesPassed);
    dataFile.print(":");
    dataFile.print(secondsPassed);
    dataFile.print("\t");
    dataFile.print("  Outside Temp: ");
    dataFile.print(dhtTempReading);
    dataFile.print(" / Humidity: ");
    dataFile.print(dhtHumidReading);
    dataFile.print("   Tank Temp: ");
    dataFile.print(tempReading);
    dataFile.print("   pH = ");
    dataFile.print(pH_Calculated);
    dataFile.print("/");
    dataFile.println(pH_CalcTemp);
    dataFile.print("Light:  IR - "); 
    dataFile.print(irLum);   
    dataFile.print("\t\t");
    dataFile.print("Full - "); 
    dataFile.print(fullSpectrumLum);   
    dataFile.print("\t");
    dataFile.print("Visible - "); 
    dataFile.print(fullSpectrumLum - irLum);   
    dataFile.print("\t");
    dataFile.print("Lux - "); 
    dataFile.println(tsl.calculateLux(fullSpectrumLum, irLum));
  }
  dataFile.close(); 
  */
 // Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++");
}

int freeRAM () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

