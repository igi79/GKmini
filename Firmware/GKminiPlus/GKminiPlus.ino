/* GK-Mini sketch  IDE:1.0.6 Board: Pro Mini / Nano w 328   22386/865 bytes  bHogan     5/22/15
 * Compiles under Arduino IDE v1.0.6, - also w/ v1.6.x NOT FIELD TESTED. board = Pro Mini
 * This sketch was written for the DIYGeigerCounter Kit sold here:
 *      https://sites.google.com/site/diygeigercounter/home
 * DIY Geiger invests a lot time and resources in providing this open source code. Please support it 
 * by considering what knock-off versions of the hardware really are.
 *
 * FEATURES / NOTES:
 * Features documentation and other info is available DIYGeigerCounter web site:
 * http://sites.google.com/site/diygeigercounter/gk-mini
 * - Unlike GK-B5, sketch creates the HV osc, click, and LED. Uses Pro-Mini
 * - Tested with external 1kHz = 59993 CPM. Live test at 85,000 CPM w reasonable accuracy
 *
 * NEW THIS VERSION:
 * - First public release
 *
 * TODO: 
 * - add click divider?
 *
 * THIS PROGRAM AND IT'S MEASUREMENTS IS NOT INTENDED TO GUIDE ACTIONS TO TAKE, OR NOT
 * TO TAKE, REGARDING EXPOSURE TO RADIATION. THE GEIGER KIT AND IT'S SOFTWARE ARE FOR
 * EDUCATIONAL PURPOSES ONLY. DO NOT RELY ON THEM IN HAZARDOUS SITUATIONS!
 *
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2.1 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * Do not remove information from this header.
 */
//----------------------------------------------------------------------------------------------+
//                        User setup #defines (others in GeigerKit.h)
//----------------------------------------------------------------------------------------------+
#define DEBUG          false            // if true, shows available memory and other debug info
#define SELF_TEST      false            // rough simulation of 360CPM - count loop only, not using interrupt

//----------------------------------------------------------------------------------------------+
//                                   Includes
//----------------------------------------------------------------------------------------------+
#include <Arduino.h>
#include <EEPROM.h>                     // for storing setup params
#include <digitalWriteFast.h>           // MODIFIED FOR 1.0.X - fast I/O for click & flash
#include <TimerOne.h>                   // provides osc for HV circuit
#include "U8glib.h"
//#include <PID_v1.h>
//----------------------------------------------------------------------------------------------+
//                                      Functions
//----------------------------------------------------------------------------------------------+

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);
#include "GeigerKit.h"                  // .h tab


void setup(){
  
  attachInterrupt(0,GetEvent,FALLING);  // Geiger event on pin 2 triggers interrupt
  pinMode(LED_PIN,OUTPUT);              // setup LED pin
  pinMode(TONE_PIN,OUTPUT);             // setup Tone pin
  digitalWrite(TONE_PIN, LOW);
  pinMode(ALARM_PIN, OUTPUT);           // setup Alarm pin
  digitalWrite(ALARM_PIN, HIGH);
  pinMode(UP_BUTTON,INPUT_PULLUP);           // null point set button
  pinMode(DOWN_BUTTON,INPUT_PULLUP);           // null point set button
  pinMode(SELECT_BUTTON,INPUT_PULLUP);           // null point set button
  pinMode(HV_POT,INPUT);
  pinMode(BATTERY,INPUT);
 
  
  Timer1.initialize(150);               // init with initial freq in uS - i.e 400 = 2.5kHz
  setHV();                         // check the HV pot to set PWM duty for HV


  Blink(LED_PIN,4);                     // show it's alive

  initSettings();
}

void initSettings(void){
  Get_Settings();

  dispPeriodStart = millis();           // start timing display CPM
  oneMinCountStart = dispPeriodStart;   // start 1 min scaler timer
  fastCountStart = dispPeriodStart;     // start bargraph timer
  longPeriodStart = dispPeriodStart;    // start long period scaler timer
  dispCnt = 0;                          // start with fresh totals
  oneMinCnt = 0;
  longPeriodCnt = 0;
  fastCnt = 0;
  temp_tube = tube;
  temp_doseUnit = doseUnit;
  temp_AlarmPoint = AlarmPoint;
  temp_scalerPeriod = scalerPeriod;
  temp_bargraphMax = bargraphMax;
  temp_doseRatio = readCPMtoDoseRatio();
  temp_cycle = cycle;
  temp_hiV = hiV;
  temp_hvM = hvM;
}


//----------------------------------------------------------------------------------------------+
//                                 MAIN FUNCTIONS
//----------------------------------------------------------------------------------------------+


void uiStep(void) {
  uiKeyCodeSecond = uiKeyCodeFirst;
  if ( digitalRead(UP_BUTTON) == LOW )
    uiKeyCodeFirst = KEY_PREV;
  else if ( digitalRead(DOWN_BUTTON) == LOW )
    uiKeyCodeFirst = KEY_NEXT;
  else if ( digitalRead(SELECT_BUTTON) == LOW )
    uiKeyCodeFirst = KEY_SELECT;
  else 
    uiKeyCodeFirst = KEY_NONE;
  
  if ( uiKeyCodeSecond == uiKeyCodeFirst )
    uiKeyCode = uiKeyCodeFirst;
  else
    uiKeyCode = KEY_NONE;
}

void updateScalerCounts(void){
    if (millis() >= oneMinCountStart + 60000/ONE_MIN_MAX){ // Collect running counts every x sec.
    oneMinCount(oneMinCnt);             // add counts
    oneMinCnt = 0;                      // reset counts
    oneMinCountStart = millis();        // reset the period time
  }

  if (millis() >= longPeriodStart + (scalerPeriod*60000)/LONG_PER_MAX && scalerPeriod < INFINITY) {
    longPeriodCount(longPeriodCnt);
    longPeriodCnt = 0;
    longPeriodStart = millis();
  }
}

void controller(void) {
  if ( uiKeyCode != KEY_NONE && last_key_code == uiKeyCode ) {
    return;
  }
  last_key_code = uiKeyCode;

  switch ( current_screen ) {
    case MAIN_SCREEN:
      switch ( uiKeyCode ) {
        case KEY_NEXT:
          current_screen++;
          break;
        case KEY_PREV:
          PiezoOn = !PiezoOn;
          break;
        case KEY_SELECT:
          break;
      }
      break;
    case SCALER_SCREEN:
      switch ( uiKeyCode ) {
        case KEY_NEXT:
          current_screen++;
          break;
        case KEY_PREV:
          resetOneMinCount();
          break;
        case KEY_SELECT:
          resetLongPeriodCount();
          break;
      }
      break;
    case GRAPH_SCREEN:
      switch ( uiKeyCode ) {
        case KEY_NEXT:
          current_screen++;
          break;
        case KEY_PREV:
          graph_type = graph_type == FAST_GRAPH ? SLOW_GRAPH : FAST_GRAPH;
          break;
        case KEY_SELECT:

          break;
      }
      break;      
    case UI_SCREEN:
      menuController(uiKeyCode,0);
      break;
      
    case TUBE_SCREEN:
      menuController(uiKeyCode,1);
      break;
  }

  if(current_screen >= SCREENS) current_screen = 0;
}

 void menuController(byte kc, byte menu){
    uint8_t x=0;
    if(menu == 1 && current_menu == HVMODE_POS) x = temp_hvM;
      if(in_item){
        switch ( kc ) {
          case KEY_NEXT:
            decrementMenuSetting(menu, x);
            break;
          case KEY_PREV:  
            incrementMenuSetting(menu, x);      
            break;
          case KEY_SELECT:
            in_item = false;
            break;
        }
      } else if(in_menu){
        switch ( kc ) {
          case KEY_NEXT:
            if(current_menu == MAX_MENU + 1){
              current_menu = 0;
            } else current_menu++;
            break;
          case KEY_PREV:        
            if(current_menu == 0 ){
              current_menu = MAX_MENU + 1;
            } else current_menu--;
            break;
          case KEY_SELECT:
            if(current_menu < MAX_MENU){
              in_item = true;
            }
            if(current_menu == SAVE){
              saveMenuSetting(menu);
              initSettings();
            }
            if(current_menu == CANCEL){

            }
            
            if(current_menu == SAVE || current_menu == CANCEL){
              in_menu = false;
              current_menu = 0;              
              temp_tube = tube;
              temp_doseUnit = doseUnit;
              temp_AlarmPoint = AlarmPoint;
              temp_scalerPeriod = scalerPeriod;
              temp_bargraphMax = bargraphMax;
              temp_doseRatio = readCPMtoDoseRatio();               
              temp_cycle = cycle;
              temp_hiV = hiV;
              temp_hvM = hvM;             
            }
            
            break;
        }
      } else 
      switch ( kc ) {
        case KEY_NEXT:
          current_screen++;
          break;
        case KEY_PREV:
          break;
        case KEY_SELECT:
          current_menu = 0;
          in_menu = true;
          break;
      }
      if ( uiKeyCode != KEY_NONE) menu_redraw_required = true;   
 }


//----------------------------------------------------------------------------------------------+
//                                 MAIN LOOOOOP
//----------------------------------------------------------------------------------------------+
void loop(){
  checkHVin();
  setHV();                         // check the HV pot to set PWM duty for HV
  checkBattery();
  uiStep();
  updateScalerCounts();

  if (millis() >= fastCountStart + 1000/ONE_SEC_MAX){ // refresh bargraph and alarm if in main display
    oneSecCount(fastCnt);
    fastCnt=0;                          // reset counts
    fastCountStart = millis();          // reset the period time
    oneSec = getOneSecCount();
    graph[graph_pointer] =  oneSec;
    if(graph_pointer < 127){
      graph_pointer++;
    } else graph_pointer = 0;
  }
  if (millis() >= dispPeriodStart + dispPeriod){             // DISPLAY PERIOD
    doseRatio = readCPMtoDoseRatio();                        // check to see if TUBE_SEL has been changed
    calcCounts(dispCnt);                                     // period is over - display counts          
    dispCnt = 0;                                             // reset counter
    dispPeriodStart = millis();                              // reset the period time  
    sgraph[sgraph_pointer] =  doseRatio;
    if(sgraph_pointer < 127){
      sgraph_pointer++;
    } else sgraph_pointer = 0;
  }

  switch(current_screen){
    case MAIN_SCREEN:
      drawMain();     
      break;
    case UI_SCREEN:
      if(menu_redraw_required)drawMenu(0);
      break;
    case TUBE_SCREEN:
      if(menu_redraw_required)drawMenu(1);
      break;
    case SCALER_SCREEN:
      drawScaler();
      break;
    case GRAPH_SCREEN:
      drawGraph();
      break;
  }

  controller();
}
//----------------------------------------------------------------------------------------------+
//                                END OF MAIN LOOOOOP
//----------------------------------------------------------------------------------------------+

void drawGraph(void){
  u8g.firstPage(); 
  do {
    u8g.setFont(u8g_font_fub14);
    u8g.setPrintPos(0,16);
    u8g.print(uSv); 
    u8g.setPrintPos(64,16);
    u8g.print(dispCPM); 
    uint8_t i,x=0;
    if(graph_type == FAST_GRAPH){
      for(i = graph_pointer; i<128; i++,x++)  u8g.drawVLine(x,64-graph[i],2);
      for(i = 0; i<graph_pointer; i++,x++)    u8g.drawVLine(x,64-graph[i],2);
    } else {
      for(i = sgraph_pointer; i<128; i++,x++)  u8g.drawVLine(x,64-sgraph[i],2);
      for(i = 0; i<sgraph_pointer; i++,x++)    u8g.drawVLine(x,64-sgraph[i],2);      
    }
    
  } while( u8g.nextPage() ); 
}



void drawMain(){
  u8g.firstPage(); 
  do {

/* bar */
    u8g.setFont(u8g_font_fub14);
    int w = map(oneSec,0,50,0,129);
    if (w <= 128){
      u8g.drawBox(0,0,w,7);
      u8g.setFont(u8g_font_helvB08);
      u8g.setPrintPos(w,16);
      u8g.print(oneSec);
    } else {
      u8g.setPrintPos(24,16);
      u8g.print("ALARM!");
    }
    
/* uSv */
    u8g.setFont(u8g_font_helvB08);
    u8g.setPrintPos(0,31);
    u8g.print("uSv");
    u8g.setFont(u8g_font_fub14);
    u8g.setPrintPos(32,31);
    u8g.print(uSv);  
    
/* CPM */ 
    u8g.setFont(u8g_font_helvB08);
    u8g.setPrintPos(0,48);
    u8g.print("CPM");
    u8g.setFont(u8g_font_fub14);
    u8g.setPrintPos(32,48);
    u8g.print(dispCPM);
    
/* battery voltage */
    u8g.drawBitmapP( 0, 56, 1, 8, battery_bitmap);
    u8g.setFont(u8g_font_helvB08);
    u8g.setPrintPos(8,64);
    u8g.print(batteryV);
    
/* high voltage */
    u8g.drawBitmapP( 40, 56, 1, 8, highvoltage_bitmap);
    u8g.setPrintPos(48,64);
    u8g.print((int)HV);
    u8g.drawBitmapP( 76, 56, 1, 8, pwm_bitmap);
    u8g.setPrintPos(86,64);
    u8g.print((int)PWMval);
    
/* mute */
    if(!PiezoOn)u8g.drawBitmapP( 112, 56, 2, 8, mute_bitmap);
  } while( u8g.nextPage() ); 
}

void calcCounts(unsigned long dcnt){    // calc and display predicted CPM & uSv/h

  static float avgCnt;                  // holds the previous moving average count
  static byte sampleCnt;                // the number of samples making up the moving average
  byte maxSamples = (60000 / dispPeriod) / 2;   // number of sample periods in 30 seconds                     

  sampleCnt++;                                  // inc sample count - must be at least 1
  avgCnt += (dcnt - avgCnt) / sampleCnt;        // CALCULATE AVERAGE COUNT - moving average
  dispCPM = (avgCnt * 60000.0) / dispPeriod;    // convert to CPM

  //handle reset of sample count - sample is for 1/2 min and reset. Options for reset value are:
  // "0" - throw away last average, "1" - keeps last average, "maxSamples -1" - keeps running avg.
  if (sampleCnt >= maxSamples) sampleCnt = 0;   // start a fresh average every 30 sec.

  // The following line gives a faster response when counts increase or decrease rapidly 
  // It resets the running average if the rate changes by  +/- 35% (previously it was 9 counts)
  if ((dcnt - avgCnt) > (avgCnt * .35) || (avgCnt - dcnt) > (avgCnt * .35)) sampleCnt = 0;
  uSv = float(dispCPM) / doseRatio;     // make dose rate conversion

  currentDispCPM = dispCPM;             // save the current CPM display in case the user sets the null point
  if (millis() > alarmSilenceStart + SILENCE_ALARM_PERIOD)alarmSilence = false ; 
  if (AlarmPoint > 0) {
    if (alarmInCPM) {                   // Alarm set to CPM
      if (dispCPM > AlarmPoint)  {
        AlarmOn = true;                 // for ALARM display
        // set alarm pin to HIGH if out of silence period 
        if (!alarmSilence) digitalWrite(ALARM_PIN, HIGH); 
      }
      if (dispCPM < AlarmPoint) {
        digitalWrite(ALARM_PIN, LOW);   // turn off alarm (set alarm pin to Gnd)
        AlarmOn = false;  
        alarmSilence = false ;          // reset for next alarm
      }
    } 
    else {                              // Alarm Set to Units
      if (uSv > AlarmPoint)  {          
        AlarmOn = true;                 // for ALARM display
        // set alarm pin to HIGH if out of silence period 
        if (!alarmSilence) digitalWrite(ALARM_PIN, HIGH);      
      }
      if (uSv < AlarmPoint) {
        digitalWrite(ALARM_PIN, LOW);   // turn off alarm (set alarm pin to Gnd)
        AlarmOn = false;  
        alarmSilence = false ;          // reset for next alarm
      }
    }
  }
}

void drawMenu(uint8_t menuId){
  u8g.firstPage();  
  char buffer[16];
  do {
    u8g.setFont(u8g_font_helvB08);
    strcpy_P(buffer, (char*)pgm_read_word(&(menu[menuId])));
    u8g.drawStr(0,14,buffer);
    uint8_t i, offset, x;
    offset = menuId * MAX_MENU;
    if(current_menu == MAX_MENU)u8g.drawBitmapP( 120, 0, 1, 8, save_bitmap);
    if(current_menu == MAX_MENU+1)u8g.drawBitmapP( 120, 0, 1, 8, cancel_bitmap);   
    for( i = 0; i < MAX_MENU; i++ ) {
      u8g.setDefaultForegroundColor();
      if ( i == current_menu && in_menu) {
        u8g.drawBox(0, 16+i*11, 128, 11);
        u8g.setDefaultBackgroundColor();
       if(in_item)u8g.drawStr(74, 25+i*11, F(">"));
      }
      x = 0;
      if(i + offset > MENU_HVMODE){
        x = temp_hvM;
      }
      strcpy_P(buffer, (char*)pgm_read_word(&(menu_items[i + offset + x])));
      u8g.drawStr(1, 25+i*11, buffer);
      
      if(i<MAX_MENU){
        switch(i + offset + x){
          case MENU_UNITS:
            strcpy_P(buffer, (char*)pgm_read_word(&(unit_table[temp_doseUnit])));    
            u8g.drawStr(80, 25+i*11, buffer);        
            break;
          case MENU_ALARM:
            String(temp_AlarmPoint).toCharArray(buffer,16);
            break;
          case MENU_SCALER:
            String(temp_scalerPeriod).toCharArray(buffer,16);
            break;
          case MENU_BARGRAPH:
            String(temp_bargraphMax).toCharArray(buffer,16);
            break;            
          case MENU_TUBE:
            strcpy_P(buffer, (char*)pgm_read_word(&(tube_label[temp_tube])));    
            break;      
          case MENU_RATIO:
            dtostrf(temp_doseRatio, 3, 2, buffer);
            break;       
          case MENU_HVMODE:
            strcpy_P(buffer, (char*)pgm_read_word(&(hvmode_label[temp_hvM])));    
            break;      
          case MENU_CYCLE:
            String(temp_cycle).toCharArray(buffer,16);
            break;        
          case MENU_POT:
            dtostrf(HV, 3, 2, buffer);
            break;                        
          case MENU_PID:
            String(temp_hiV).toCharArray(buffer,16);
            break;        
        }
        u8g.drawStr(80, 25+i*11, buffer);
      }
      
      u8g.setDefaultForegroundColor();
    }    
  } while( u8g.nextPage() );
  menu_redraw_required = false;
}

void drawScaler(){ // create the screen that shows the running counts
  float tempSum;                        // for summing running count
  float temp_uSv;                       // for converting CPM to uSv/h for running average
  unsigned int secLeft;

  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_helvB08);
    u8g.drawStr(0,14,F("SCALER"));
    
    // 1 MINUTE DISPLAY LINE . . .
    tempSum = getOneMinCount();
    temp_uSv = tempSum / doseRatio;       // calc uSv/h
    if (!dispOneMin) {
      tempSum += oneMinCnt;
    }
  
    currentDispCPM = tempSum;             // save the currently displayed CPM in case the user sets the null point
    u8g.setPrintPos(0,24);
    u8g.print("1m:");
    
    u8g.setPrintPos(48,24);
    u8g.print(tempSum,0);                 // display 1 minute CPM or running count
    
    u8g.setPrintPos(74,24);
    if (dispOneMin) {
      u8g.print(temp_uSv);                // display 1 minute uSv, right justified
    } else {
      secLeft = 60 - (oneMinuteIndex*60/ONE_MIN_MAX);
      u8g.print(secLeft,DEC);             // show seconds left
      u8g.drawBox(0,27,map(secLeft,0,60,0,128),4);
    }
    
    // 10 MINUTE DISPLAY LINE . . .
    u8g.setPrintPos(0,48);
    if (scalerPeriod == INFINITY) {
      u8g.print("\xf3");                // if scalerPeriod is set to INFINITY, write the symbol for infinity to the lcd
    } else {
      u8g.print(scalerPeriod);
    }  
    
    tempSum = getLongPeriodCount();
    if (dispLongPeriod) {
      tempSum /= (float)scalerPeriod;   // sum over 10 minutes so divide by that when CPM is displayed
    } 
    else {
      tempSum += longPeriodCnt; // period hasn't finished yet add the current counts to the total
    }
    temp_uSv = tempSum / doseRatio;
    u8g.setPrintPos(48,48);
    u8g.print(tempSum,0);                  // display long period CPM
    
    u8g.setPrintPos(74,48);
    if (dispLongPeriod) {
      u8g.print(temp_uSv);         // display long period dose rate, right justified
    }
    if (!dispLongPeriod && scalerPeriod < INFINITY) {
  
      // Type casting needed to prevent unsigned int overflow at longPeriodIndex==110 when scaler period is 10 minutes (109*10*60=65400)
      secLeft = (scalerPeriod * 60) - (((unsigned long)longPeriodIndex*(unsigned long)scalerPeriod*(unsigned long)60)/(unsigned long)LONG_PER_MAX);
      u8g.print(secLeft,DEC);             // show seconds left  
      u8g.drawBox(0,51,map(secLeft,0,scalerPeriod*60,0,128),4);
    }
    
  } while( u8g.nextPage() );
}

unsigned long getOneSecCount() {
  unsigned long tempSum = 0;
  for (int i = 0; i <= ONE_SEC_MAX-1; i++){ // sum up 1 second counts
    tempSum = tempSum + oneSecond[i];
  }
  return tempSum;
}


unsigned long getOneMinCount() {
  unsigned long tempSum = 0;
  for (int i = 0; i <= ONE_MIN_MAX-1; i++){ // sum up 1 minute counts
    tempSum = tempSum + oneMinute[i];
  }
  return tempSum;
}


unsigned long getLongPeriodCount() {
  unsigned long tempSum = 0;
  for (int i = 0; i <= LONG_PER_MAX-1; i++){ // sum up long period counts
    tempSum = tempSum + longPeriod[i];
  }
  return tempSum;
}

void oneSecCount(unsigned long dcnt) {
  static byte oneSecondIndex = 0;

  oneSecond[oneSecondIndex++] = dcnt;
  if(oneSecondIndex >= ONE_SEC_MAX) {
    oneSecondIndex = 0;
  }
}


void resetOneMinCount() {  // clears out the one minute count
  memset(oneMinute, 0, sizeof(oneMinute));  // zero the entire array
  oneMinuteIndex=0;                     // reset index to 0
  dispOneMin = false;                   // clear the flag
  oneMinCnt = 0;                        // reset the running count
  oneMinCountStart = millis();          // reset the running count start time
}


static void oneMinCount(unsigned long dcnt){ // Add CPM of period to 1M 
  oneMinute[oneMinuteIndex] = dcnt;
  if(oneMinuteIndex >= ONE_MIN_MAX-1) {
    oneMinuteIndex = 0;
    if (!dispOneMin) {
      dispOneMin = true;                // indicate that average is available
    }
  }
  else oneMinuteIndex++;
}


void resetLongPeriodCount() {  // resets the one long period count
  memset(longPeriod, 0, sizeof(longPeriod));  // zero the entire array
  longPeriodIndex=0;                    // reset index to 0
  dispLongPeriod = false;               // clear the flag
  longPeriodCnt = 0;                    // reset the running count
  longPeriodStart = millis();           // reset the running count start time
}


static void longPeriodCount(unsigned long dcnt){ // Add CPM of period to 1M 
  longPeriod[longPeriodIndex] = dcnt;
  if(longPeriodIndex >= LONG_PER_MAX-1) {
    longPeriodIndex = 0;
    dispLongPeriod = true;              // indicate that average is available
  }
  else longPeriodIndex++;
}


unsigned long readVcc() { // SecretVoltmeter from TinkerIt
  unsigned long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  lastVCC = result;           // TO DO - NOW MAKING ENOUGH CALLS TO WARRENT THIS - USING W/ METER
  return result;
}

// rolling your own map function saves a lot of memory
unsigned long lmap(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max){
  return x>in_max ? out_max : (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//----------------------------------------------------------------------------------------------+
//                                      Utilities
//----------------------------------------------------------------------------------------------+

void Blink(byte led, byte times){ // just to flash the LED
  for (byte i=0; i< times; i++){
    digitalWrite(led,HIGH);
    delay (150);
    digitalWrite(led,LOW);
    delay (100);
  }
}


// variables created by the build process when compiling the sketch
extern int __bss_end;
extern void *__brkval;

int AvailRam(){ 
  int freeValue;
  if ((int)__brkval == 0)
    freeValue = ((int)&freeValue) - ((int)&__bss_end);
  else
    freeValue = ((int)&freeValue) - ((int)__brkval);
  return freeValue;
} 


byte getLength(unsigned long number){
  byte length = 0;
  unsigned long t = 1;
  do {
    length++;
    t*=10;
  } 
  while(t <= number);
  return length;
}

void clickFlash(){
  digitalWriteFast(LED_PIN, HIGH); 
  delayMicroseconds(100);                     //flash LED
  digitalWriteFast(LED_PIN, LOW); 
  if (PiezoOn){
    digitalWriteFast(TONE_PIN, HIGH); 
    delayMicroseconds(110);                   //110 us pulse
    digitalWriteFast(TONE_PIN, LOW); 
  }
}

void setHV (){
  
  switch(hvM){
    case 0: //pot
      static unsigned int lastPotVal = 0;
      unsigned int potVal; 
      potVal = analogRead(HV_POT - 14);
      if (potVal > lastPotVal + POT_HYSTERESIS || potVal < lastPotVal - POT_HYSTERESIS) {  
        PWMval = lmap(potVal,0,1023,HV_MIN_PWM,HV_MAX_PWM); 
        lastPotVal=potVal;
      }
      break;
    case 1: //pwm
      PWMval = cycle;
      break;
/*      
    case 2: //pid
      Input = HV;
      myPID.Compute();
      PWMval = Output;
      break;
*/
  }

  
  
  Timer1.pwm(PWM_PIN,PWMval);
}

void checkBattery(){
  //signed int 
  bat = analogRead(BATTERY);
  batteryV =  bat * BATVOLRATIO;
}

void checkHVin(){
  //signed int 
  hvin = analogRead(HV_IN);
  HV = hvin * HVVOLRATIO;
}

//----------------------------------------------------------------------------------------------+
//                                        ISR
//----------------------------------------------------------------------------------------------+

void GetEvent(){   // ISR triggered for each new event (count)
  dispCnt++;
  oneMinCnt++;
  longPeriodCnt++;
  fastCnt++;
  clickFlash();
}





