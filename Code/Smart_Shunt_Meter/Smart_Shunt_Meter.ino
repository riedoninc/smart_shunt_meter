/*************************************************************************
*  Title:    Smart Shunt Meter
*  Author:   Philip Ebbert
*  License:  GNU General Public License v3
*  
*  Program written for the Arduino UNO WiFi Rev2, NX8048T050 Nextion display, and ARD-LTC2499
*  Startup Guide for UNO Wifi Rev2:  https://www.arduino.cc/en/Guide/ArduinoUnoWiFiRev2
*  WiFiNINA library version 1.8.3, firmware version 1.4.3
*  Update WiFiNINA library and firmware here: https://github.com/arduino/WiFi101-FirmwareUpdater-Plugin
*  
*  Default is to connect to WiFi via DHCP
*  If you wish to use static IP address uncomment #define STATIC_IP and set ipaddr to desired value
*  
*  WiFi connection is automatically closed after sending a reading.
*  Send command READ? to IP address and port.
*  
*  Default Password 4444 for changing settings
*  
*  When setting up a new Arduino UNO WiFi Rev2, EEPROM is blank. Uncomment //#define write_EEPROM_initial and
*  upload sketch to write default values to EEPROM. Then comment out for normal operation.
*  
*  If using regular Arduino UNO, the display serial pins will nee to be ulplugged during programming as they 
*  are shared with the programming port. The UNO WiFi Rev2 has a separate serial port for programming.
*  
*  LICENSE:
*    Copyright (C) 2021 Philip Ebbert
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*  
*************************************************************************/

#include <Wire.h>
#include "Ard2499.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include "EEPROM_Anything.h"            // Read & Write to EEPROM 

#define BEEPER   4                      // Beeper PIN
//#define STATIC_IP                     // Uncomment to set static IP address, otherwise DHCP
//#define write_EEPROM_initial          // Uncomment to write initial values to EEPROM

#ifdef STATIC_IP
  IPAddress ipaddr(192, 168, 8, 200);   // Set to Static IP desired
#endif

#define ENTER       0x0D                // Defines for Display Function Control Buttons
#define CANCEL      0x18
#define DELETE      0x08
#define CLEAR       0x1B
#define UP          0x11
#define DOWN        0x12
#define NEXT        0x13
#define PREV        0x14

#define ARRAYCOUNT(x)  (sizeof(x)/sizeof(x[0]))

char    ssid[32];                       // network SSID (name)
char    wifipass[32];                   // network password (use for WPA, or use as key for WEP)
String  strWifiPass;
String  strSSID;
int     keyIndex = 0;                   // your network key index number (needed only for WEP)

float   filteredAdc = 10;               // Set to 10 to force reset first pass
byte    keypress = 0;
long    adcVal;
float   Vadc;
float   VoltsPerAmp;                    // 0.0125 SSA-100, 0.005 SSA-250, 0.0025 SSA-500, 0.00125 SSA-1000
float   adcScalar;                      // Used to calibrate ADC
float   shuntOffset;                    // Used to zero reading 
float   filterWeight;                   // Used for exponential filtering of ADC, typically 0.05-0.20 (5-20%)
int     averageNumber;
byte    digitalFilter;
const char  *filtering_list[] = {"Mean Average", "Exponential"};

char    Data_Text[32];
char    Message[32];

long    wifiPort;
int     pass = 0, password = 4444;

Ard2499 ard2499board1; 

int status = WL_IDLE_STATUS;
int j = EEPROM_readAnything(30, wifiPort);         // Read Ethernet Port from EEPROM

WiFiServer server(wifiPort);

void setup() {

  //Initialize EEPROM Values

#ifdef write_EEPROM_initial

  VoltsPerAmp = 0.005;                             // 0.0125 SSA-100, 0.005 SSA-250, 0.0025 SSA-500, 0.00125 SSA-1000
  adcScalar = 1.00;                                // No scaling ADC value
  shuntOffset = 0.0;                               // Zero shunt offset
  filterWeight = 0.12;                             // 12% filterWeight on exponential filter 
  wifiPort = 5025;                                 // Listen on Port 5025
  digitalFilter = 0;                               // Mean Average mode
  averageNumber = 5;                               // Average 5 readings on Mean Average mode                             
    
  j = EEPROM_writeAnything(10, VoltsPerAmp);       // write VoltsPerAmp to EEPROM location 10, 4 bytes
  j = EEPROM_writeAnything(18, adcScalar);         // write adcScalar to EEPROM location 18, 4 bytes
  j = EEPROM_writeAnything(22, shuntOffset);       // write shuntOffset to EEPROM location 22, 4 bytes
  j = EEPROM_writeAnything(26, filterWeight);      // write filterWeight to EEPROM location 26, 4 bytes
  j = EEPROM_writeAnything(30, wifiPort);          // write VoltsPerAmp to EEPROM location 30, 4 bytes
  j = EEPROM_writeAnything(34, digitalFilter);     // write digitalFilter to EEPROM location 34, 1 byte
  j = EEPROM_writeAnything(38, averageNumber);     // write averageNumber to EEPROM location 34, 4 bytes

#endif
  
  j = EEPROM_readAnything(10, VoltsPerAmp);           // Read VoltsPerAmp from EEPROM
  j = EEPROM_readAnything(18, adcScalar);             // Read adcScalar from EEPROM
  j = EEPROM_readAnything(22, shuntOffset);           // Read shuntOffset from EEPROM
  j = EEPROM_readAnything(26, filterWeight);          // Read filterWeight from EEPROM
  j = EEPROM_readAnything(34, digitalFilter);         // Read digitalFilter from EEPROM
  j = EEPROM_readAnything(38, averageNumber);         // Read averageNumber from EEPROM

  for ( j = 0; j < 32; j++ ){                         // Read SSID from EEPROM
    ssid[j] = EEPROM.read( 100+j );
    if ( ssid[j] == '\0' ) break;
  }
  for ( j = 0; j < 32; j++ ){                         // Read WiFi Password from EEPROM
    wifipass[j] = EEPROM.read( 140+j );
    if ( wifipass[j] == '\0' ) break;
  }

  strWifiPass = wifipass;                             // Used to enter new WiFi SSID and Password
  strSSID = ssid;
  
  pinMode(BEEPER, OUTPUT);
  digitalWrite(BEEPER,LOW);

  // initialize serial communications
  Serial.begin(9600);                                 // start the serial monitor port
  Serial1.begin(115200);                              // Start display serial port at 115200 baud

  delay(200);
  DisplayWrite("dim=100");                            // Set brightness to 100%
  DisplayWrite("page 0");                             // Switch Display to Page 0
  delay(20);

  DisplayWrite("t0.txt=\"v1.0 \"");                   // Display Code Version
  DisplayWrite("t1.txt=\" \"");
  delay(1500);
  DisplayWrite("t0.txt=\"WiFi \"");

  Wire.begin();
  ard2499board1.begin(ARD2499_ADC_ADDR_ZZZ, ARD2499_EEP_ADDR_ZZ);
  ard2499board1.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ);
  ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_DIFF_1P_0N);

  // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  #ifdef STATIC_IP
    WiFi.config(ipaddr);
  #endif
  status = WiFi.begin(ssid, wifipass);

  // wait for connection:
  j = 0;
  do {
    delay(1500);
    if ( j++ > 5 ) break;
  } while ( WiFi.status() != WL_CONNECTED );
  
  if ( WiFi.status() == WL_CONNECTED ) {
    server.begin();
    printWifiStatus();
    Beep();
    DisplayWrite("t1.txt=\"ON  \"");
    Serial.println("Connected");
  }
  else {
    Beep();
    DisplayWrite("t1.txt=\"OFF  \"");
    Serial.println("WiFi not connected");
  }
  
  delay(2000);

  DisplayWrite("t0.txt=\"0.00000\"");
  DisplayWrite("t1.txt=\"0.00000\"");
}

void loop() {

  keypress = get_keypress ();                                 //Get keypress if available
  if ( keypress > 0 ) ProcessKeypress(keypress);

  ReadADC();

  UpdateDisplay(filteredAdc, filteredAdc/VoltsPerAmp);

  if ( WiFi.status() == WL_CONNECTED ) listenForWifiClients();

}
