//----------------------------------------------------------------------------------------------+
//                                    Menu Functions
//----------------------------------------------------------------------------------------------+


///////////////////////////////////////////////////////////////////////
//  Menu helper functions
///////////////////////////////////////////////////////////////////////


static void saveMenuSetting (byte menu) {
  switch(menu) {
  case 1:
    if (temp_tube > MAX_TUBE) temp_tube = MAX_TUBE;
    writeParam(temp_tube, TUBE_ADDR);
    if (temp_doseRatio > DOSE_RATIO_MAX) temp_doseRatio = DOSE_RATIO_MAX;
    writeCPMtoDoseRatio(temp_doseRatio);
    if (temp_cycle > HV_MAX_PWM) temp_cycle = HV_MAX_PWM;
    if (temp_cycle < HV_MIN_PWM) temp_cycle = HV_MIN_PWM;
    if (temp_hiV > HV_MAX) temp_hiV = HV_MAX;    
    if (temp_hiV < HV_MIN) temp_hiV = HV_MIN;    
    unsigned int t;
    t = temp_tube;
    writeParam(temp_hvM, HVMODE_ADDR);
    writeParam(temp_cycle, CYCLE_ADDR);    
    writeFloatParam(temp_hiV, HV_ADDR);    
    break;
  case 0:
    if (temp_AlarmPoint > MAX_ALARM) temp_AlarmPoint = MAX_ALARM;
    writeParam(temp_AlarmPoint, ALARM_SET_ADDR);
    writeParam(temp_doseUnit, DOSE_UNIT_ADDR);
    if (temp_scalerPeriod >= SCALER_PER_MAX && temp_scalerPeriod != INFINITY) temp_scalerPeriod = SCALER_PER_MAX;
    else if (temp_scalerPeriod <= SCALER_PER_MIN) temp_scalerPeriod = SCALER_PER_MIN;
    else if (temp_scalerPeriod != INFINITY) {
      while (temp_scalerPeriod < SCALER_PER_MAX && (60000 * (unsigned int)temp_scalerPeriod) % LONG_PER_MAX != 0) {  // have to make sure that the interval is evenly divisible by the number of elements in the array (if LONG_PER_MAX is left at 120, this is always false and optimized out by the compiler)
        temp_scalerPeriod++;
      }
    }
    writeParam(temp_scalerPeriod, SCALER_PER_ADDR);
    resetLongPeriodCount();        // reset the long period count because we changed the period
    if (temp_bargraphMax > BARGRAPH_SCALE_MAX) temp_bargraphMax = BARGRAPH_SCALE_MAX;
    else if (temp_bargraphMax < BARGRAPH_SCALE_MIN) temp_bargraphMax = BARGRAPH_SCALE_MIN;
    writeParam(temp_bargraphMax, BARGRAPH_MAX_ADDR);
    break;
  }
}

static float incrementMenuSetting (byte menu, byte x) {
  switch(x + current_menu + menu * MAX_MENU) {
    case MENU_ALARM:
      if (temp_AlarmPoint<MAX_ALARM)temp_AlarmPoint++;
      break;
    case MENU_UNITS:
      if (temp_doseUnit>=MAX_UNIT) temp_doseUnit = 0;
      else temp_doseUnit++;
      break;
    case MENU_SCALER:
      if (temp_scalerPeriod==SCALER_PER_MAX) temp_scalerPeriod = INFINITY;
      else if (temp_scalerPeriod==INFINITY) temp_scalerPeriod = SCALER_PER_MIN;
      else {
        do {
          temp_scalerPeriod++;
        } 
        while (temp_scalerPeriod < SCALER_PER_MAX && (60000 * (unsigned int)temp_scalerPeriod) % LONG_PER_MAX != 0);  // have to make sure that the interval is evenly divisible by the number of elements in the array (if LONG_PER_MAX is left at 120, this is always false and optimized out by the compiler)
      }
      break;
    case MENU_BARGRAPH:
      if (temp_bargraphMax>=BARGRAPH_SCALE_MAX) temp_bargraphMax = BARGRAPH_SCALE_MIN;
      else temp_bargraphMax++;
      break;
    case MENU_TUBE:
      if (temp_tube>=MAX_TUBE) temp_tube = 0;
      else temp_tube++;
      readTempTube(temp_tube);
      break;
    case MENU_RATIO:
      temp_doseRatio++;
      break;
    case MENU_CYCLE:
      if (temp_cycle >= HV_MAX_PWM) temp_cycle = HV_MAX_PWM;
      else temp_cycle++;
      break;
    case MENU_PID:
      if (temp_hiV >= HV_MAX) temp_hiV = HV_MAX;
      else temp_hiV++;
      break;    
    case MENU_HVMODE:
      if (temp_hvM >= MAX_HVMODE) temp_hvM = 0;
      else temp_hvM++;
      break;
  }
}

static float decrementMenuSetting(byte menu, byte x) {
  
  switch(x + current_menu + menu * MAX_MENU) {
    case MENU_ALARM:
      if (temp_AlarmPoint>0)temp_AlarmPoint--;
      break;
    case MENU_UNITS:
      if (temp_doseUnit==0) temp_doseUnit = MAX_UNIT;
      else temp_doseUnit--;
      break;
    case MENU_SCALER:
      if (temp_scalerPeriod==SCALER_PER_MIN) temp_scalerPeriod = INFINITY;
      else if (temp_scalerPeriod==INFINITY) temp_scalerPeriod = SCALER_PER_MAX;
      else {
        do {
          temp_scalerPeriod--;
        } 
        while ((60000 * (unsigned int)temp_scalerPeriod) % LONG_PER_MAX != 0);  // have to make sure that the interval is evenly divisible by the number of elements in the array (if LONG_PER_MAX is left at 120, this is always false and optimized out by the compiler)
      }
      break;
    case MENU_BARGRAPH:
      if (temp_bargraphMax==BARGRAPH_SCALE_MIN) temp_bargraphMax = BARGRAPH_SCALE_MAX;
      else temp_bargraphMax--;
      break;
    case MENU_TUBE:
      if (temp_tube==0) temp_tube = MAX_TUBE;
      else temp_tube--;
      readTempTube(temp_tube);
      break;
    case MENU_RATIO:
      temp_doseRatio--;
      break;  
    case MENU_CYCLE:
      if (temp_cycle <= HV_MIN_PWM) temp_cycle = HV_MIN_PWM;
      else temp_cycle--;
      break;
    case MENU_PID:
      if (temp_hiV <= HV_MIN) temp_hiV = HV_MIN;
      else temp_hiV--;
      break;     
    case MENU_HVMODE:
      if (temp_hvM==0) temp_hvM = MAX_HVMODE;
      else temp_hvM--;
      break;      
  }
}

///////////////////////////////////////////////////////////////////////
// Functions to read settings from / write settings to EEPROM
///////////////////////////////////////////////////////////////////////

void readTempTube(unsigned int t){
  temp_doseRatio = readFloatParam(TUBE_RATIO_ADDR);
  temp_cycle = readParam(CYCLE_ADDR);
  if(temp_cycle > HV_MAX_PWM || temp_cycle < HV_MIN_PWM  || isnan(temp_cycle)) temp_cycle = DEFAULT_PWM;
  temp_hiV = readFloatParam(HV_ADDR);
  if(temp_hiV > HV_MAX || temp_hiV < HV_MIN) temp_hiV = DEFAULT_PID;
  temp_hvM = readParam(HVMODE_ADDR);
  if(temp_hvM > MAX_HVMODE || isnan(temp_hvM)) temp_hvM = 0;
}

void Get_Settings(){ // read setting out of EEPROM and set local variables
  // set defaults if EEPROM has not been used yet
  dispPeriod = 1500;
  
  tube = readParam(TUBE_ADDR);
  if (tube > MAX_TUBE){
    writeParam(0,TUBE_ADDR);
    tube = 0;
  }
  
  unsigned int t;
  t = tube;
  
  doseRatio = readCPMtoDoseRatio();
  
  cycle = readParam(CYCLE_ADDR);
  if(cycle > HV_MAX_PWM || cycle < HV_MIN_PWM || isnan(cycle)){
    writeParam(CYCLE_ADDR, DEFAULT_PWM);
    cycle = DEFAULT_PWM;
  }
  
  hiV = readFloatParam(HV_ADDR);
  if(hiV > HV_MAX || hiV < HV_MIN){
    writeFloatParam(HV_ADDR, DEFAULT_PID);
    hiV = DEFAULT_PID;
  }
  
  hvM = readParam(HVMODE_ADDR);
  if(hvM > MAX_HVMODE || isnan(hvM)){
    writeParam(HVMODE_ADDR,0);
    hvM = 0;
  }
/*
  Setpoint = hiV;
  myPID.SetOutputLimits(HV_MIN_PWM, HV_MAX_PWM);
  myPID.SetMode(hvM == 2 ? AUTOMATIC : MANUAL);
*/
  AlarmPoint = readParam(ALARM_SET_ADDR);        // if zero - no alarm
  if (AlarmPoint > MAX_ALARM){                   // defult if > ALARM_MAX CPM
    writeParam(ALARM_POINT,ALARM_SET_ADDR);      // write EEPROM
    AlarmPoint = ALARM_POINT;
  }

  doseUnit = readParam(DOSE_UNIT_ADDR);          // get the saved value for the dose unit
  if (doseUnit > MAX_UNIT) {
    writeParam(0, DOSE_UNIT_ADDR);               // default to uSv
    doseUnit = 0;
  }

  alarmInCPM = (boolean)readParam(ALARM_UNIT_ADDR);

  scalerPeriod = readParam(SCALER_PER_ADDR);     // get the saved value for the long period scaler
  if (scalerPeriod < SCALER_PER_MIN || scalerPeriod > SCALER_PER_MAX || (60000 * scalerPeriod) % LONG_PER_MAX != 0) { // discard the value if over the max or if not divisible by the number of elements in the array
    if (scalerPeriod != INFINITY) {
      writeParam(SCALER_PERIOD, SCALER_PER_ADDR);  // write default value
      scalerPeriod = SCALER_PERIOD;
    }
  }

  bargraphMax = readParam(BARGRAPH_MAX_ADDR);      // get the CPM value that will put the bargraph at full scale - if not previously set, use the default
  if (bargraphMax > BARGRAPH_SCALE_MAX) {
    writeParam(FULL_SCALE, BARGRAPH_MAX_ADDR);
    bargraphMax = FULL_SCALE;
  }

  dispPeriodStart = 0;                  // start timing over when returning to loop
}

void writeParam(unsigned int value, unsigned int addr){ // Write menu entries to EEPROM
  unsigned int a = value/256;
  unsigned int b = value % 256;
  EEPROM.write(addr,a);
  EEPROM.write(addr+1,b);
}

unsigned int readParam(unsigned int addr){ // Read previous menu entries from EEPROM
  unsigned int a=EEPROM.read(addr);
  unsigned int b=EEPROM.read(addr+1);
  return a*256+b; 
}

void writeFloatParam(float value, unsigned int addr) {
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(addr++, *p++);
  return;
}

static float readFloatParam(unsigned int addr) {
  float value;
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(addr++);
  return value;
}

float readCPMtoDoseRatio() {
  unsigned int t;
  float ratio;

  t = readParam(TUBE_ADDR);
  ratio = readFloatParam(TUBE_RATIO_ADDR);
  if (ratio == 0 || ratio > DOSE_RATIO_MAX || isnan(ratio)) {  // defult if 0 or > 2000
    writeFloatParam(RATIO,TUBE_RATIO_ADDR);       // write EEPROM
    ratio = RATIO;                     // set the default
  }
  return ratio;
}

void writeCPMtoDoseRatio(float ratio) {
  unsigned int t;
  t = readParam(TUBE_ADDR);
  writeFloatParam(ratio, TUBE_RATIO_ADDR);     // write to the primary address
  resetOneMinCount();                           // reset the counts since we're changing the ratio - otherwise, they'll be off
  resetLongPeriodCount();
}




