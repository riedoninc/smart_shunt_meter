
/*************************************************************************
*  Title:    MeterFunctions
*  Author:   Philip Ebbert
*  License:  GNU General Public License v3
*  
*  Program written for the Arduino UNO WiFi Rev2, NX8048T050 Nextion display, and ARD-LTC2499
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

void Beep() {     // Beep Function - Send square wave to output pin

  //tone(BEEPER, 2048, 60);             //Use this command if using a beeper that requires square wave signal
  digitalWrite(BEEPER, HIGH);           //Use these commands if using a beeper that requires continuous signal
  delay(60);
  digitalWrite(BEEPER,LOW);
  
}

void ReadADC(){

  if ( digitalFilter == 0 ) {                               // Mean Average Digital Filter Mode
    filteredAdc = 0;

    for ( int k = 0; k < averageNumber; k++) {
      adcVal = ard2499board1.ltc2499Read();
      Vadc = (adcVal * 2.048 * adcScalar) / 16777216.0;
    
      Vadc += shuntOffset;
      filteredAdc += Vadc;
    }

    filteredAdc /= averageNumber;
  }

  if ( digitalFilter == 1 ) {                               // Exponential Digital Filter Mode
    adcVal = ard2499board1.ltc2499Read();
    Vadc = (adcVal * 2.048 * adcScalar) / 16777216.0;
  
    Vadc += shuntOffset;
  
    if ( ( abs(filteredAdc) < abs(0.99 * Vadc)) || ( abs(filteredAdc) > abs(1.01 * Vadc)) ) { //If greater than 1% difference, reset filter to improve response
      if ( abs(filteredAdc) > 0.002 ) {                     // reading less than 2mV, don't reset filter
        filteredAdc = Vadc;
      }
    }
    filteredAdc += (Vadc - filteredAdc) * filterWeight;     // Exponential data filter
  }
}

void listenForWifiClients() {

  float Amps;

  char rBuffer[16];
  int i=0;

  WiFiClient client = server.available();
  
  if (client.connected()) {

    i=0;

    while (client.connected()) {
      
      if (client.available()) {
        char c = client.read();
        if (c != '\r' && i<15 ) rBuffer[i++] = c;
        if ( i >= 15 ) break;                             //Invalid command, close connection
        if (c == '?' ) {
          rBuffer[i] = '\0';                              //Terminate rBuffer
          
          if(strcmp(rBuffer, "READ?") == 0) {             //Return Results
            Amps = filteredAdc / VoltsPerAmp;
            client.println(Amps,3);
            //client.println("Hello");
          }
          break;
        }
      }
    }
    delay(1);                                               // give the client time to receive the data
    client.stop();                                          // close the connection:
    //Serial.println("Client Disconnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void ProcessKeypress(char wKey) {

  int    i=0, oldInt;
  float  oldFloat;
  byte   oldByte;
  long   oldLong;
  String oldString;
  char   wifiStatus[300];
  byte   mac[6];

  switch (wKey){

    case 'Z':           // Zero Meter
        Beep();
        DisplayWrite("t0.txt=\"9.99999\"");
        DisplayWrite("t1.txt=\"999.999\"");
        shuntOffset -= filteredAdc;
        j = EEPROM_writeAnything(22, shuntOffset);                // write shuntOffset to EEPROM
        delay(1000);
        break;

    case '9':           // Enter Shunt Settings
       Beep();
       DisplayWrite("page 1");
       while(get_int( &pass, "Enter Password ( Default 4444 )" , " " , 0, 9999));
       if ( pass != password ) break;
       pass = 0;

       Beep();
       oldFloat = shuntOffset;
       while(get_float( &shuntOffset, "- Enter Offset Voltage -", "V", -.01, .01, 5));
       if ( oldFloat != shuntOffset ) j = EEPROM_writeAnything( 22, shuntOffset);        // write shuntOffset to EEPROM
       
       Beep();
       oldFloat = VoltsPerAmp;
       while(get_float( &VoltsPerAmp, "- Enter Volts per Amp -", " ", 0.0001, 2.5, 5));
       if ( oldFloat != VoltsPerAmp ) j = EEPROM_writeAnything( 10, VoltsPerAmp);       // Write VoltsPerAmp to EEPROM

       Beep();
       oldFloat = adcScalar;
       while(get_float( &adcScalar, "- Enter ADC Scalar -", " ", 0.50, 1.50, 6));
       if ( oldFloat != adcScalar ) j = EEPROM_writeAnything( 18, adcScalar);            // Write adcScalar to EEPROM

       Beep();
       oldByte = digitalFilter;
       while(get_item( &digitalFilter, filtering_list, "- Digital Filter Mode -", " ", ARRAYCOUNT(filtering_list)));
       if ( oldByte != digitalFilter ) j = EEPROM_writeAnything( 34, digitalFilter);     // Write digitalFilter to EEPROM

       if ( digitalFilter == 0 ) {
          Beep();
          oldInt = averageNumber;
          while(get_int( &averageNumber, "- Enter Number of Averages -", " ", 1, 128 ));
          if ( oldFloat != averageNumber ) j = EEPROM_writeAnything( 38, averageNumber);  // write averageNumber to EEPROM
       }

       if ( digitalFilter == 1 ) {
          Beep();
          oldFloat = filterWeight;
          while(get_float( &filterWeight, "- Enter ADC filterWeight -", " ", 0.01, 1.0, 3));
          if ( oldFloat != filterWeight ) j = EEPROM_writeAnything( 26, filterWeight);    // write filterWeight to EEPROM
       }
       
       break;

    case '8':           // View Network Settings

       Beep();
       DisplayWrite("page 2");

       IPAddress ip = WiFi.localIP();
       long rssi = WiFi.RSSI();
       WiFi.macAddress(mac);

       sprintf( wifiStatus, "t0.txt=\"SSID: %s\\r"
                                     "IP Address: %d.%d.%d.%d:%ld\\r"
                                     "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \\r"
                                     "Signal Strength (RSSI): %ld dBm\"", 
                                     WiFi.SSID(), ip[0], ip[1], ip[2], ip[3], wifiPort, mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], rssi);
       DisplayWrite( wifiStatus );

       do{
        keypress = get_keypress ();                               //Get keypress if available
       } while(keypress!=ENTER);

       Beep();
       DisplayWrite("page 1");
       while(get_int( &pass, "Enter Password ( Default 4444 )" , " " , 0, 9999));
       if ( pass != password ) break;
       pass = 0;

       Beep();
       oldString = strSSID;
       while(get_string( &strSSID, "- Enter WiFi Password -", 32, 3, 6));
       if ( oldString != strSSID ) {
        for ( i = 0; i < strSSID.length(); i++ ) {     // Write strSSID to EEPROM
            EEPROM.write( 100+i, strSSID.charAt(i));
          }
          EEPROM.write( 100+i, '\0');                   // Terminate String
       }

       Beep();
       oldString = strWifiPass;
       while(get_string( &strWifiPass, "- Enter WiFi Password -", 32, 3, 6));
       if ( oldString != strWifiPass ) {
        for ( i = 0; i < strWifiPass.length(); i++ ) {     // Write strSSID to EEPROM
            EEPROM.write( 140+i, strWifiPass.charAt(i));
          }
          EEPROM.write( 140+i, '\0');                   // Terminate String
       }

       DisplayWrite("page 1");

       Beep();
       oldLong = wifiPort;
       while(get_long( &wifiPort, "- Enter WiFI Listening Port -", " ", 1, 65535));
       if ( oldLong != wifiPort ) j = EEPROM_writeAnything( 30, wifiPort);            // Write wifiPort to EEPROM

       break;

  } // End switch (wKey)

   DisplayWrite("page 0");
  
} // End ProcessKeypress
