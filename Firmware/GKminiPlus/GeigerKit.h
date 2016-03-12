//----------------------------------------------------------------------------------------------+
//               PIN MAP for  Arduino Pro-Mini - Each I/O pin used is defined . . .
//----------------------------------------------------------------------------------------------+

//              NUST MATCH SCHEMATIC !!!!!!!!!!!
// PIN MAP - Each I/O pin (used or unused) is defined . . .
// PWM pins are: 3, 5, 6, 9, 10, and 11. (tone interfears with PWM on D3 & D11)
// NOTE: A6 & A7 are analog in pins ONLY

#define BATTERY           A2             // battery input voltage
#define HV_POT            A6             // HV adjustment pot (analog only)
#define HV_IN             A7             // 1/10 of HV
//                        A5             // RESERVED for I2C
//                        A4             // RESERVED for I2C
#define LED_PIN           13             // 1mS flash / event - also flashes 5X at startup
#define TONE_PIN          10             // output to speaker for click or tone mode
#define PWM_PIN            9             // Timer1 PWM pin - ATmega328: 9&10 / Atmage1284: 12&13
#define ALARM_PIN          A3             // outputs HIGH when Alarm triggered

#define UP_BUTTON          5             // UP
#define DOWN_BUTTON        7             // DOWN
#define SELECT_BUTTON      6             // SELECT

//----------------------------------------------------------------------------------------------+
//                                 other defines . . .
//----------------------------------------------------------------------------------------------+

#define LOW_VCC            4200 //mV    // if Vcc < LOW_VCC give low voltage warning
#define ONE_MIN_MAX          12         // elements in the oneMinute accumulater array
#define LONG_PER_MAX  (10*ONE_MIN_MAX)  // elements in the longPeriod accumulater array (pegged to 10x ONE_MIN_MAX)
#define ONE_SEC_MAX          20         // elements in the oneSecond accumulator array
#define TIMERBASE_RC5      1778         // 1 bit time of RC5 element
#define SILENCE_ALARM_PERIOD 30000      // mS the alarm will be silenced for
#define DEBOUNCE_MS          50         // buttom debounce period in mS
#define POT_HYSTERESIS       4          // amount of change in potentiometer value needed to trigger a recalculation of HV (keeps the value from bouncing around)
#define TONE_MIN_FREQ        31         // minimum frequency that will be generated by the CPMtoTone function - 31Hz is the lowest supported by the Arduino @16MHz
#define TONE_MAX_FREQ        4000       // maximum frequency that will be generated by the CPMtoTone function
#define AVGBGRAD_uSv         0.27       // global average background radiation level in uSv/h
#define AVBGRAD_uR           10.388     // global average background radiation level in uR/h
#define AVBGRAD_mR           0.010388   // global average background radiation level in mR/h
#define INFINITY             65534      // if scalerPeriod is set to this value, it will just do a cumulative count forever
#define HV_MIN_PWM           51         // minimum PWM duty - 51/1023 = 5%
#define HV_MAX_PWM           819        // maximum PWM duty - 819/1023 = 80% (HV lower after 80%)
#define HV_MIN               50
#define HV_MAX               1000
#define BATVOLRATIO          0.004988047
#define HVVOLRATIO           0.677981651

//----------------------------------------------------------------------------------------------+
//                           Menu configuration parameters 
//----------------------------------------------------------------------------------------------+

// Configuration parameter minimum/maximum settings (where applicable)
#define DOSE_RATIO_MAX       20000      // maximum value for the CPM to dose unit conversion ratio
#define MAX_ALARM            60000      // max the alarm can be set for
#define BARGRAPH_SCALE_MIN   2          // minimum value of the full scale CPM for the bargraph
#define BARGRAPH_SCALE_MAX   60000      // maximum value of the full scale CPM for the bargraph
#define SCALER_PER_MIN       2          // minimum allowed value for the long period scaler
#define SCALER_PER_MAX       90         // maximum allowed value for the long period scaler

// These are DEFAULTS! - only used if menu has not been run
#define DISP_PERIOD     1500            // defaults to 5 sec sample & display
#define RATIO        175.43         // defaults to SBM-20 ratio
#define ALARM_POINT      100            // CPM for Alarm defaults to
#define SCALER_PERIOD     20            // default scaler period
#define FULL_SCALE      1000            // max CPM for all 6 bars 
#define DEFAULT_PWM    300
#define DEFAULT_PID      400

// EEPROM Address for menu inputs
#define DISP_PERIOD_ADDR  0  // unsigned int - 2 bytes
#define LOG_PERIOD_ADDR   2  // unsigned int - 2 bytes
#define ALARM_SET_ADDR    6  // unsigned int - 2 bytes
#define DOSE_UNIT_ADDR    8 // byte - 1 byte 
#define ALARM_UNIT_ADDR   10 // boolean - 1 byte
#define SCALER_PER_ADDR   12 // unsigned int - 2 bytes
#define BARGRAPH_MAX_ADDR 14 // unsigned int - 2 bytes
#define PIEZO_SET_ADDR    40 // boolean - 1 byte

#define TUBE_ADDR        58 // uint 2 bytes
#define BASE_TUBE_ADDR   60 // uint 2 bytes
#define TUBE_ADR_WIDTH   12 //12 bytes
#define TUBE_RATIO_ADDR  BASE_TUBE_ADDR + t * TUBE_ADR_WIDTH     // float - 4 bytes
#define HV_ADDR          BASE_TUBE_ADDR + t * TUBE_ADR_WIDTH + 4 // float - 4 bytes
#define CYCLE_ADDR       BASE_TUBE_ADDR + t * TUBE_ADR_WIDTH + 8 // unsigned int - 2 bytes
#define HVMODE_ADDR      BASE_TUBE_ADDR + t * TUBE_ADR_WIDTH + 10 // unsigned int - 2 bytes

//----------------------------------------------------------------------------------------------+
//                                     Globals
//----------------------------------------------------------------------------------------------+

// These hold the local values that have been read from EEPROM
float doseRatio;                        // holds the rate selected by jumper
unsigned int temp_doseRatio;
unsigned int AlarmPoint, temp_AlarmPoint;                // alarm if > than this setting

boolean scalerParam;                    // flag indicating whether the scaler screen should be on or off
unsigned int scalerPeriod, temp_scalerPeriod;              // period for the > 1 minute scaler
unsigned int tube, temp_tube;
unsigned int cycle, temp_cycle;
unsigned int hiV, temp_hiV;
unsigned int hvM, temp_hvM;
byte doseUnit, temp_doseUnit;                          // 0 - uSv/H, 1 - uR/H, 2 - mR/H
boolean alarmInCPM;                     // 1 = CPM, 0 - DisplayUnit

boolean lowVcc = false;                 // true when Vcc < LOW_VCC
boolean PiezoOn = false;                 // preset to piezo = ON
// variables for counting periods and counts . . .
unsigned long dispPeriodStart, dispPeriod; // for display period
unsigned long dispCnt;                  // to count and display CPM
boolean AlarmOn = false;                // CPM > set alarm
boolean alarmSilence = false;           // true if alarm has been silenced

float uSv = 0.0;                      // display CPM converted to VERY APPROXIMATE uSv
unsigned long dispCPM,oneSec;         // display CPM

volatile unsigned long fastCnt;
unsigned long fastCountStart;           // counter for bargraph refresh period
unsigned int bargraphMax, temp_bargraphMax;

unsigned long oneMinCountStart;         // timer for running average
volatile unsigned long oneMinCnt;       // counter for running averages
volatile unsigned long longPeriodCnt;   // counter for the long period scaler
unsigned long longPeriodStart;          // start time for long period scaler
unsigned long alarmSilenceStart;        // timer for silencing the alarm is Select pressed

unsigned long oneMinute[ONE_MIN_MAX];   // array holding counts for 1 minute running average
unsigned long longPeriod[LONG_PER_MAX]; // array holding counts for 10 minute running average
unsigned long oneSecond[ONE_SEC_MAX];   // array holding counts for 1 second running average
byte oneMinuteIndex = 0;                // index to 1 minute array
byte longPeriodIndex = 0;               // index to 10 minute array

boolean dispOneMin = false;             // indicates 1 minute average is available
boolean dispLongPeriod = false;         // indicates 10 minute average is available
unsigned long currentDispCPM = 0;       // holds the current CPM value on the display - used with the button to set the null point
unsigned long lastVCC;                  // MAKE ONLY 1 CALL TO readVcc FOR EFFICENCY - TO FINISH
float batteryV,HV;
unsigned int PWMval;
unsigned int bat,hvin;

//double consKp=1, consKi=0.05, consKd=0.25;
//double Setpoint, Input, Output;
//PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);

/* UI */
#define SMALL_FONT_HEIGHT 9

#define KEY_NONE 0
#define KEY_PREV 1
#define KEY_NEXT 2
#define KEY_SELECT 3

uint8_t uiKeyCodeFirst = KEY_NONE;
uint8_t uiKeyCodeSecond = KEY_NONE;
uint8_t uiKeyCode = KEY_NONE;

#define SCREENS 5
#define MAIN_SCREEN 0
#define SCALER_SCREEN 2
#define GRAPH_SCREEN 1
#define UI_SCREEN 3
#define TUBE_SCREEN 4
#define SLOW_GRAPH 0
#define FAST_GRAPH 1
uint8_t current_screen = 0;
uint8_t current_menu = 0;
uint8_t in_menu = false, in_item = false;
uint8_t last_key_code = KEY_NONE;
uint8_t menu_redraw_required = true;
uint8_t graph[128];
uint8_t sgraph[128];
uint8_t graph_pointer = 0;
uint8_t sgraph_pointer = 0;
uint8_t graph_type = FAST_GRAPH;


const uint8_t battery_bitmap[] U8G_PROGMEM  = {
  B00110000,
  B11111100,
  B10000100,
  B11111100,
  B10000100,
  B11111100,
  B10000100,
  B11111100,
};

const uint8_t highvoltage_bitmap[] U8G_PROGMEM  = {
  B00001100,
  B00011000,
  B00110000,
  B01100000,
  B00011000,
  B00110000,
  B01100000,
  B11000000,
};

const uint8_t pwm_bitmap[] U8G_PROGMEM  = {
  B11110010,
  B10010010,
  B10010010,
  B10010010,
  B10010010,
  B10010010,
  B10010010,
  B10011110,
};

const uint8_t mute_bitmap[] U8G_PROGMEM  = {
  B00000010,B00000000,
  B00000110,B01000001,
  B01111010,B00100010,
  B01010010,B00010100,
  B01010010,B00001000,
  B01111010,B00010100,
  B00000110,B00100010,
  B00000010,B01000001,
};

const uint8_t save_bitmap[] U8G_PROGMEM  = {
  B00000000,
  B00000001,
  B00000011,
  B00000110,
  B10001100,
  B11011000,
  B01110000,
  B00100000,
};

const uint8_t cancel_bitmap[] U8G_PROGMEM  = {
  B00000000,
  B10000010,
  B01000100,
  B00101000,
  B00010000,  
  B00101000,
  B01000100,
  B10000010,
};

//----------------------------------------------------------------------------------------------+
//                                    For menu
//----------------------------------------------------------------------------------------------+




#define MENU_RATIO        5
#define MENU_TUBE         4
#define MENU_HVMODE       6
#define MENU_POT          7
#define MENU_CYCLE        8
#define MENU_PID          9
#define MENU_ALARM        1
#define MENU_UNITS        0
#define MENU_SCALER       2
#define MENU_BARGRAPH     3

#define SAVE              4
#define CANCEL            5
#define HVMODE_POS        3

const char intro_0[] PROGMEM   = "Geigercounter V2";
const char intro_1[] PROGMEM   = "by Ivan Ignath";
const char intro_2[] PROGMEM   = "based on";
const char intro_3[] PROGMEM   = "GK Mini Ver. 1.4";
const char intro_4[] PROGMEM   = __DATE__;
const char intro_5[] PROGMEM   = __TIME__;

const char label_0[] PROGMEM   = "User Interface";
const char label_1[] PROGMEM   = "Tube Settings";
const char * const menu[] PROGMEM = {label_0, label_1};

#define MAX_TUBE  2
const char tube_0[] PROGMEM   = "SBM-20";
const char tube_1[] PROGMEM   = "SBT-10";
const char tube_2[] PROGMEM   = "SBM-19";
const char * const tube_label[] PROGMEM = {tube_0, tube_1, tube_2};


#define MAX_HVMODE  1
const char hvs_0[] PROGMEM   = "Pot";
const char hvs_1[] PROGMEM   = "PWM";
const char hvs_2[] PROGMEM   = "PID";
const char * const hvmode_label[] PROGMEM = {hvs_0, hvs_1, hvs_2};

#define MAX_MENU  4
const char string_0[] PROGMEM   = "Dose units";
const char string_1[] PROGMEM   = "Alarm point";
const char string_2[] PROGMEM   = "Scaler period";
const char string_3[] PROGMEM   = "Bargraph max";
const char string_4[] PROGMEM   = "Tube";
const char string_5[] PROGMEM   = "Ratio";
const char string_6[] PROGMEM   = "HV mode";
const char string_7[] PROGMEM   = "Voltage";
const char string_8[] PROGMEM   = "Cycle duty";
const char string_9[] PROGMEM   = "PID voltage";
const char * const menu_items[] PROGMEM = {string_0, string_1, string_2, string_3, string_4, string_5, string_6, string_7, string_8, string_9};

#define MAX_UNIT  2

// unit strings used for logging - use u instead of mu since nobody interprets chars above 127 consistently
const char unit_0[] PROGMEM = "uSv/h";
const char unit_1[] PROGMEM = "uR/h";
const char unit_2[] PROGMEM = "mR/h";

const char * const unit_table[] PROGMEM = // PROGMEM array to hold unit strings
{
  unit_0,
  unit_1,
  unit_2
};



