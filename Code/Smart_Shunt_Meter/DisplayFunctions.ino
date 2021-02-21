
/*************************************************************************
*  Title:    DisplayFunctions
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
*  Functions
*      void DisplayWrite(char *message);
*      byte get_keypress();
*      int get_float( float *num, const char *heading, const char *units, float low_limit, float high_limit, byte decimals )
*      int get_long_as_float( long *num, const char *heading, const char *units, long low_limit, long high_limit, byte decimals)
*      int get_long( long *num, const char *heading, const char *units, long low_limit, long high_limit)
*      int get_int( int *num, const char *heading, const char *units, int low_limit, int high_limit)
*      int get_item( byte *code, char *list[], const char *heading, const char *units, byte listsize )
*      int get_string( String *str, const char *heading, int strlength, int startPage, int endPage )
*      
*************************************************************************/


void DisplayWrite(char *message){

  Serial1.print(message);
  Serial1.write(0xff);          //Send three 0xff to terminate command
  Serial1.write(0xff);
  Serial1.write(0xff);
}

void UpdateDisplay (float Volts, float Amps) {

  dtostrf( Volts,9,5,Data_Text );
  sprintf( Message, "t0.txt=\"%s\"", Data_Text );
  DisplayWrite( Message );

  dtostrf( Amps,9,3,Data_Text );
  sprintf( Message, "t1.txt=\"%s\"", Data_Text );
  DisplayWrite( Message );
}
  
byte get_keypress(){

  int incomingByte = 0;             // for incoming serial data
  
  if (Serial1.available() > 0) {
        incomingByte = Serial1.read();
        //Serial.println(incomingByte,HEX);
    }
  return incomingByte;
}
  
int get_float( float *num, const char *heading, const char *units, float low_limit, float high_limit, byte decimals ){
  
     static byte state = 0;
     static byte k, retval, dpused, update_display, dec_counter;
     static char value[12];
     byte i;
     float temp;
     char numstring[48];
     char mystring[48];
     byte keypress;
  
     update_display = false;
  
     switch (state) {
  
        case 0:
  
           for ( i = 0; i < 12; i++ ) value[i] = '\0';  //Initialize value
           if (decimals<1) decimals = 1;
           if (decimals>6) decimals = 6;
           
           switch ( decimals ) {
             case 1:
                dtostrf( *num, 0, 1, numstring );
                break;
             case 2:
                dtostrf( *num, 0, 2, numstring );
                break;
             case 3:
                dtostrf( *num, 0, 3, numstring );
                break;
             case 4:
                dtostrf( *num, 0, 4, numstring );
                break;
             case 5:
                dtostrf( *num, 0, 5, numstring );
                break;
             case 6:
                dtostrf( *num, 0, 6, numstring );
                break;
            }
            
           sprintf( mystring, "t0.txt=\"%s\"", heading );
           DisplayWrite( mystring );
           
           sprintf( mystring, "t1.txt=\"%s %s\"", numstring, units );
           DisplayWrite( mystring );

           state++;
           k = 0;
           retval = 1;
           dpused = 0;
           dec_counter = 0;        // Counter for how many digits after decimal
           break;
  
        case 1:
  
            keypress = get_keypress();               //Get keypress if available
            
            switch(keypress){
  
              case ENTER:             // Enter Button
                 Beep();
                 state = 0;
                 if ( k != 0 )      // if k=0, Enter button acts like Cancel button
                 {
                    temp = atof ( value );
                    if ( temp < low_limit ) *num = low_limit;
                    else if ( temp > high_limit ) *num = high_limit;
                    else *num = temp; 
                 }
                 state = 0;
                 retval = 0;
                 break;
  
              case CANCEL:             // Cancel Button
                 Beep();
                 state = 0;
                 retval = 0;
                 break;
  
              case CLEAR:             // Clear Button
                 Beep();
                 for ( i = 0; i < 12; i++ ) value[i] = '\0';  //Initialize value
                 k = 0;
                 dpused = false;
                 dec_counter = 0;
                 update_display = true;
                 break;
  
              case DELETE:             // Delete Button
                 Beep();
                 if ( k > 0 ) {
                    k--;
                    if ( value[k] == '.' ) dpused = false;
                    if ( dec_counter > 0 ) dec_counter--;
                    value[k] = '\0';
                    update_display = true;   }
                 break;
  
              case '-':            // "-" Button ( Must be first character )
                 Beep();
                 if ( k == 0 ){
                    value[k] = '-';
                    k++;
                    update_display = true;   }
                 break;
  
              case '.':            // "." Button ( Can only be used once )
                 Beep();
                 if ( dpused == false && k < 11 ){
                    value[k] = '.';
                    dpused = true;
                    k++;
                    update_display = true;   }
                 break;
  
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
                 Beep();
                 if ( k < 11 && ( dec_counter < decimals )) {
                    value[k] = keypress;
                    if ( dpused )dec_counter++;
                    k++;
                    update_display = true;}
                 break;
  
                 }       // end of keypress switch
            }      // end state switch
  
      if (update_display) 
      {
         sprintf( mystring, "t1.txt=\"%s %s\"", value, units );
         DisplayWrite( mystring );
         update_display = false;
      }
  
      return retval;
  }  // ********************   end of function get_float ******************************
  
int get_long_as_float( long *num, const char *heading, const char *units, long low_limit, long high_limit, byte decimals){
 
  static byte state = 0;
  static byte retval, k, update_display;
  static byte dpused, dec_counter;
  static char value[12];
  static char value_nodp[12];  // same as value[11] except no decimal point in string
  char current_value[12];
  long temp;
  char numstring[48];
  char mystring[48];
  char keypress;
  byte length, i, j;
 
  update_display = false;
 
  switch (state) {
 
     case 0:
 
        for ( i = 0; i < 12; i++ ){       //Initialize value
           value[i] = '\0';
           value_nodp[i] = '\0';
        }
        
        sprintf(mystring,"%ld",*num);
        length = strlen(mystring);
        
        if (decimals<1) decimals = 1;
        if (decimals>6) decimals = 6;
        
        if ( *num < 0 && length <=(decimals+1)){         // ******  Negative Numbers Between -1 and Zero ******
           temp = abs(*num);
           sprintf(numstring,"%ld",temp);
           length = strlen(mystring);
           current_value[0]='-';
           current_value[1]='0';
           current_value[2]='.';
           for ( i=0; i<(decimals-length);i++ ){
              current_value[i+3] = '0';
           }
           for ( j=i; j < decimals; j++ ){
              current_value[j+3] = mystring[j-i];
           }
           current_value[j+3] = '\0';
        }
        else {                                       // ****** Positive Numbers Between 1 and Zero ******
           if ( length <= decimals ){
              current_value[0]='0';
              current_value[1]='.';
              for ( i=0; i<(decimals-length);i++ ){
                 current_value[i+2] = '0';
              }
              for ( j=i; j < decimals; j++ ){
                 current_value[j+2] = mystring[j-i];
              }
              current_value[j+2] = '\0';
           }
           else{                                      // ****** Mumbers less than -1 or greater than 1 ******
              for ( i=0; i<(length-decimals);i++ ){
                 current_value[i] = mystring[i];
              }
              current_value[i] = '.';
              for ( j=i; j < length; j++ ){
                 current_value[j+1] = mystring[j];
              }
              current_value[j+1] = '\0';
           }
        }
        
        sprintf( numstring, "%11s", current_value );

        sprintf( mystring, "t0.txt=\"%s\"", heading );
        DisplayWrite( mystring );
  
        sprintf( mystring, "t1.txt=\"%s %s\"", numstring, units );
        DisplayWrite( mystring );
 
        k = 0;
        dpused = 0;
        dec_counter = 0;        // Counter for how many digits after decimal
        state++;
        retval = 1;
        break;
 
     case 1:
 
         keypress = get_keypress();
 
         switch(keypress){
 
           case ENTER:             // Enter Button
              Beep();
              state = 0;
              if ( k != 0 )      // if k=0, Enter button acts like Cancel button
              {
                 temp = atol ( value_nodp );
                 for ( i=0; i<(decimals-dec_counter); i++ ){
                    temp = temp * 10;
                 }
                 if ( temp < low_limit ) *num = low_limit;
                 else if ( temp > high_limit ) *num = high_limit;
                 else *num = temp; 
              }
              retval = 0;
              break;
 
           case CANCEL:             // Cancel Button
              Beep();
              state = 0;
              retval = 0;
              break;
 
           case CLEAR:             // Clear Button
              Beep();
              for ( i = 0; i < 12; i++ ){       //Initialize value
                 value[i] = '\0';
                 value_nodp[i] = '\0';
              }
              k = 0;
              dpused = false;
              dec_counter = 0;
              update_display = true;
              break;
 
           case DELETE:             // Delete Button
              Beep();
              if ( k > 0 ) {
                 k--;
                 
                 if ( !dpused ) {
                    value_nodp[k]= '\0';
                 }
                 else {
                    if ( value[k] != '.' ) value_nodp[k-1]= '\0';
                 }
                 
                 if ( value[k] == '.' ) dpused = false;
                 if ( dec_counter > 0 ) dec_counter--;
                 value[k] = '\0';
                 update_display = true;   }
              break;

           case '-':            // "-" Button ( Must be first character )
              Beep();
              if ( k == 0 ){
                 value[k] = '-';
                 value_nodp[k] = '-';
                 k++;
                 update_display = true;   }
              break;
              
           case '.':            // "." Button ( Can only be used once )
              Beep();
              if ( dpused == false && k < 11 ){
                 value[k] = '.';
                 dpused = true;
                 k++;
                 update_display = true;   }
              break;
 
           case '0':
           case '1':
           case '2':
           case '3':
           case '4':
           case '5':
           case '6':
           case '7':
           case '8':
           case '9':
              Beep();
              if ( k < 11 && ( dec_counter < decimals )) {
                 value[k] = keypress;
                 if ( dpused ){
                    value_nodp[k-1] = keypress;
                    dec_counter++;
                 }
                 else value_nodp[k] = keypress;
                 k++;
                 update_display = true;}
              break;
 
              }       // end of keypress switch
         }      // end state switch
 
   if (update_display){
      sprintf( mystring, "t1.txt=\"%s %s\"", value, units );
      DisplayWrite( mystring );
      update_display = false;
   }
 
   return retval;
 }  // ********************   end of function get_long_as_float ******************************

int get_long( long *num, const char *heading, const char *units, long low_limit, long high_limit){
  
     static byte  state = 0;
     static byte  k, retval, update_display;
     static char  value[11];
     byte i;
     long temp;
     char mystring[48];
     char keypress;
  
     update_display = false;
  
     switch (state) {
  
        case 0:
  
           for ( i = 0; i < 11; i++ ) value[i] = '\0';  //Initialize value

           sprintf( mystring, "t0.txt=\"%s\"", heading );
           DisplayWrite( mystring );

           sprintf( mystring, "t1.txt=\"%ld %s\"", *num, units );
           DisplayWrite( mystring );
  
           state++;
           k = 0;
           retval = 1;
           break;
  
        case 1:
  
            keypress = get_keypress();
  
            switch(keypress){
  
              case ENTER:             // Enter Button
                 Beep();
                 state = 0;
                 if ( k != 0 )      // if k=0, Enter button acts like Cancel button
                 {
                    temp = atol ( value );
                    if ( temp < low_limit ) *num = low_limit;
                    else if ( temp > high_limit ) *num = high_limit;
                    else *num = temp; 
                 }
                 retval = 0;
                 break;
  
              case CANCEL:             // Cancel Button
                 Beep();
                 state = 0;
                 retval = 0;
                 break;
  
              case CLEAR:             // Clear Button
                 Beep();
                 for ( i = 0; i < 11; i++ ) value[i] = '\0';  //Initialize value
                 k = 0;
                 update_display = true;
                 retval = 1;
                 break;
  
              case DELETE:             // Delete Button
                 Beep();
                 if ( k > 0 ) {
                    k--;
                    value[k] = '\0';
                    update_display = true;   }
                 retval = 1;
                 break;
  
              case '-':            // "-" Button ( Must be first character )
                 Beep();
                 if ( k == 0 ){
                    value[k] = '-';
                    k++;
                    update_display = true;   }
                 retval = 1;
                 break;
  
              case UP:            // Up Button Pressed
                 Beep();
                 if ( k == 0 ) {
                     temp = *num + 1;
                    if ( temp <= high_limit ) {
                       sprintf(value, "%d", temp);
                       k = strlen ( value );
                       }
                    else {
                       sprintf(value, "%d", *num);
                       k = strlen ( value );
                       }
                    }
                  else {
                    temp = atol ( value ) + 1;
                    if ( temp <= high_limit ) {
                       sprintf(value, "%d", temp);
                       k = strlen ( value );
                       }
                    }
                 retval = 1;
                 update_display = true;
                 break;
  
              case DOWN:            // Down Button Pressed
                 Beep();
                 if ( k == 0 ) {
                     temp = *num - 1;
                    if ( temp >= low_limit ) {
                       sprintf(value, "%d", temp);
                       k = strlen ( value );
                       }
                    else {
                       sprintf(value, "%d", *num);
                       k = strlen ( value );
                       }
                    }
                  else {
                    temp = atol ( value ) - 1;
                    if ( temp >= low_limit ) {
                       sprintf(value, "%d", temp);
                       k = strlen ( value );
                       }
                    }
                 retval = 1;
                 update_display = true;
                 break;
  
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
                 Beep();
                 if ( k < 10 ) {
                    value[k] = keypress;
                    k++;
                    update_display = true;}
                 retval = 1;
                 break;
  
                 }       // end of keypress switch
            }      // end state switch
  
      if (update_display){
         sprintf( mystring, "t1.txt=\"%s %s\"", value, units );
         DisplayWrite( mystring );
         update_display = false;
      }
  
      return retval;
  }  // ********************   end of function get_long ******************************
  
int get_int( int *num, const char *heading, const char *units, int low_limit, int high_limit){
  
     static byte  state = 0;
     static byte  k, retval, update_display;
     static char value[11];
     byte i;
     long temp;
     char mystring[48];
     char keypress;
  
     update_display = false;
  
     switch (state) {
  
        case 0:
  
           for ( i = 0; i < 11; i++ ) value[i] = '\0';  //Initialize value

           sprintf( mystring, "t0.txt=\"%s\"", heading );
           DisplayWrite( mystring );

           sprintf( mystring, "t1.txt=\"%d %s\"", *num, units );
           DisplayWrite( mystring );
  
           state++;
           k = 0;
           retval = 1;
           break;
  
        case 1:
  
            keypress = get_keypress();               //Get keypress if available
  
            switch(keypress){
  
              case ENTER:             // Enter Button
                 Beep();
                 state = 0;
                 if ( k != 0 )      // if k=0, Enter button acts like Cancel button
                 {
                    temp = atol ( value );
                    if ( temp < low_limit ) *num = low_limit;
                    else if ( temp > high_limit ) *num = high_limit;
                    else *num = (signed int) temp; 
                 }
                 retval = 0;
                 break;
  
              case CANCEL:             // Cancel Button
                 Beep();
                 state = 0;
                 retval = 0;
                 break;
  
              case CLEAR:             // Clear Button
                 Beep();
                 for ( i = 0; i < 11; i++ ) value[i] = '\0';  //Initialize value
                 k = 0;
                 update_display = true;
                 retval = 1;
                 break;
  
              case DELETE:             // Delete Button
                 Beep();
                 if ( k > 0 ) {
                    k--;
                    value[k] = '\0';
                    update_display = true;   }
                 retval = 1;
                 break;
  
              case '-':            // "-" Button ( Must be first character )
                 Beep();
                 if ( k == 0 ){
                    value[k] = '-';
                    k++;
                    update_display = true;   }
                 retval = 1;
                 break;
  
              case UP:            // Up Button Pressed
                 Beep();
                 if ( k == 0 ) {
                     temp = *num + 1;
                    if ( temp <= high_limit ) {
                       sprintf(value, "%d", temp);
                       k = strlen ( value );
                       }
                    else {
                       sprintf(value, "%d", *num);
                       k = strlen ( value );
                       }
                    }
                  else {
                    temp = atol ( value ) + 1;
                    if ( temp <= high_limit ) {
                       sprintf(value, "%d", temp);
                       k = strlen ( value );
                       }
                    }
                 retval = 1;
                 update_display = true;
                 break;
  
              case DOWN:            // Down Button Pressed
                 Beep();
                 if ( k == 0 ) {
                     temp = *num - 1;
                    if ( temp >= low_limit ) {
                       sprintf(value, "%d", temp);
                       k = strlen ( value );
                       }
                    else {
                       sprintf(value, "%d", *num);
                       k = strlen ( value );
                       }
                    }
                  else {
                    temp = atol ( value ) - 1;
                    if ( temp >= low_limit ) {
                       sprintf(value, "%d", temp);
                       k = strlen ( value );
                       }
                    }
                 retval = 1;
                 update_display = true;
                 break;
  
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
                 Beep();
                 if ( k < 10 ) {
                    value[k] = keypress;
                    k++;
                    update_display = true;}
                 retval = 1;
                 break;
  
                 }       // end of keypress switch
            }      // end state switch
  
      if (update_display){
         sprintf( mystring, "t1.txt=\"%s %s\"", value, units );
         DisplayWrite( mystring );
         update_display = false;
      }
  
      return retval;
  }  // ********************   end of function get_int ******************************
  
int get_item( byte *code, char *list[], const char *heading, const char *units, byte listsize )  {
  
     static byte state=0;
     static byte retval, update_display;
     char mystring[48];
     byte keypress;
  
     update_display = false;
  
     switch (state) {
  
        case 0:
           if ( *code >= listsize ) *code = 0;    // if *code out of range, set to zero.

           sprintf( mystring, "t0.txt=\"%s\"", heading );
           DisplayWrite( mystring );

           sprintf( mystring, "t1.txt=\"%s %s\"", list[*code], units );
           DisplayWrite( mystring );
  
           state++;
           retval = 1;
           break;
  
        case 1:
  
            keypress = get_keypress();               //Get keypress if available
  
            switch(keypress){
  
              case ENTER:
                 Beep();
                 state = 0;
                 retval = 0;
                 break;
  
               case UP:                       // Next Item in List
                 Beep();
                 if ( *code < ( listsize - 1 )) *code+=1;
                 else *code = 0;
                 update_display = true;
                 retval = 1;
                 break;
  
               case DOWN:                       // Previous Item in List
                 Beep();
                 if ( *code > 0 ) *code-=1;
                 else *code = listsize - 1;
                 update_display = true;
                 retval = 1;
                 break;
  
               default:
                 retval = 1;
                 break;
  
             }       // end switch(keypress)   
      }      // end switch (state)
      
      if (update_display) {
         sprintf( mystring, "t1.txt=\"%s %s\"", list[*code], units );
         DisplayWrite( mystring );
         update_display = false;
      }
  
      return retval;
  }  // ********************   end of function get_item ******************************
  
int get_string( String *str, const char *heading, int strlength, int startPage, int endPage )  {
  
     static byte  state = 0;                  // First time in function
     static byte  k, retval, update_display;
     static char value[32];
     static int page;
     byte i;
     char mystring[48], oldstring[48];
     char keypress;
     String CurrentString;
  
     update_display = false;
  
     switch (state) {
  
        case 0:                                         // First time through function
  
           for ( i = 0; i < 32; i++ ) value[i] = '\0';  //Initialize value
           
           CurrentString = *str;
           CurrentString.toCharArray(oldstring, strlength);

           page = startPage;
           sprintf( mystring, "page %d", page);
           DisplayWrite( mystring );

           sprintf( mystring, "t0.txt=\"%s\"", heading );
           DisplayWrite( mystring );

           sprintf( mystring, "t1.txt=\"%s\"", oldstring );
           DisplayWrite( mystring );
  
           state++;  
           k=0;
           retval = 1;
           break;
  
        case 1:
  
            keypress = get_keypress();
  
            switch(keypress){

              case 0:                 // No keypress
                break;
  
              case ENTER:             // Enter Button
                 Beep();
                 state = 0;
                 if ( k != 0 )      // if k=0 (number of characters = 0), Enter button acts like Cancel button
                 {
                   //strncpy( str, value, length+1);
                   //sprintf( *str, "%s", value );
                   *str = value; 
                 }
                 retval = 0;
                 break;
  
              case CANCEL:             // Cancel Button
                 Beep();
                 state = 0;
                 retval = 0;
                 break;
  
              case CLEAR:             // Clear Button
                 Beep();
                 for ( i = 0; i < 21; i++ ) value[i] = '\0';  //Initialize value
                 k = 0;
                 update_display = true;
                 retval = 1;
                 break;
  
              case DELETE:             // Delete Button
                 Beep();
                 if ( k > 0 ) {
                    k--;
                    value[k] = '\0';
                    update_display = true;   }
                 retval = 1;
                 break;

              case NEXT:                // Next Screen Button
                 Beep();
                 
                 if ( page < endPage ) page++;
                 else page = startPage;
                 sprintf( mystring, "page %d", page);
                 DisplayWrite( mystring );
                 
                 sprintf( mystring, "t0.txt=\"%s\"", heading );
                 DisplayWrite( mystring );
      
                 sprintf( mystring, "t1.txt=\"%s\"", value );
                 DisplayWrite( mystring );
                 retval = 1;
                 break;

              case PREV:                // Previous Screen Button
                 Beep();
                 
                 if ( page > startPage ) page--;
                 else page = endPage;
                 sprintf( mystring, "page %d", page);
                 DisplayWrite( mystring );
                 
                 sprintf( mystring, "t0.txt=\"%s\"", heading );
                 DisplayWrite( mystring );
      
                 sprintf( mystring, "t1.txt=\"%s\"", value );
                 DisplayWrite( mystring );
                 retval = 1;
                 break;

              case 0x5C:              // Backslash \ and Quote doesn't work with Nextion, ignore
              case 0x22:
                 Beep();
                 delay(100);
                 Beep();
                 DisplayWrite("t0.txt=\"No Backslash or Quote on Nextion\"");
                 delay(1500);
                 sprintf( mystring, "t0.txt=\"%s\"", heading );
                 DisplayWrite( mystring );
                 break;
  
              default:
                 Beep();
                 if ( k < strlength-1 ) {
                    value[k] = keypress;
                    k++;
                    update_display = true;}
                 retval = 1;
                 break;
  
                 }       // end of keypress switch
            }      // end state switch
  
      if (update_display){
         sprintf( mystring, "t1.txt=\"%s\"", value );
         DisplayWrite( mystring );
         update_display = false;
      }
  
      return retval;
  }  // ********************   end of function get_string ******************************
