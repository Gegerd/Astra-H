#include "HardwareCAN.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  ASTRA H DEFINITIONS                                //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Define the values of the MS CAN identifiers
#define MS_MEDIA_BUTTONS_ID 0x201
#define MS_BTN_BC 0x01
#define MS_BTN_1 0x31
#define MS_BTN_2 0x32
#define MS_BTN_3 0x33
#define MS_BTN_4 0x34
#define MS_BTN_5 0x35
#define MS_BTN_6 0x36
#define MS_BTN_7 0x37
#define MS_BTN_8 0x38
#define MS_BTN_9 0x39
#define MS_BTN_OK 0x6F
#define MS_BTN_RIHGT 0x6C
#define MS_BTN_LEFT 0x6D
#define MS_BTN_CD_RADIO 0xE0
#define MS_BTN_SETTINGS 0xFF
#define MS_WHEEL_BUTTONS_ID 0x206
#define MS_BTN_STATION 0x81
#define MS_BTN_MODE 0x82
#define MS_BTN_LEFT_WHEEL_TURN 0x83
#define MS_BTN_LEFT_WHEEL 0x84
#define MS_BTN_NEXT 0x91
#define MS_BTN_PREV 0x92
#define MS_MEDIA_ID 0x6C1
#define MS_ECC_ID 0x548
#define MS_BATTERY 0x07
#define MS_ENGINE_TEMP 0x10
#define MS_RPM_SPEED 0x10
#define MS_IGNITION_STATE_ID 0x450
#define MS_IGNITION_NO_KEY 0x00
#define MS_IGNITION_KEY_PRESENT 0x04
#define MS_IGNITION_1 0x05
#define MS_IGNITION_START 0x06
#define MS_SPEED_RPM_ID 0x4E8
#define MS_BACKWARDS_BIT 0x04
#define MS_WAKEUP_ID 0x697
// Define buttons state bytes
#define BTN_PRESSED 0x01
#define BTN_RELEASED 0x00
#define WHEEL_PRESSED 0x08
#define WHEEL_TURN_DOWN 0xFF
#define WHEEL_TURN_ 0x01
// Limit time to flag a CAN error
#define CAN_TIMEOUT 100
#define CAN_DELAY 0 // 10        // ms between two processings of incoming messages
#define CAN_SEND_RATE 10 // 200   // ms between two successive sendings
#define PC13ON 0
#define PC13OFF 1
#define DELAY 250
//#define DEBUG  //Debug activation will increase the ant of memory by ~16k

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                         ASTRA H VARIABLES AND FUNCTIONS                             //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

//****************************************Variables************************************//
bool alarm = 0;
bool Blink = 0;
bool AUX_mode = 1;
int VOLTAGE = 131;
int T_ENG = 1000;
int SPEED = 0;
int RPM = 0;
bool Sound = false;
uint32_t btn=0;
uint32_t time_request_ecc = 0;          //Variable for the parameter request timer from ECC
uint32_t time_send = 0;                 //Variable for the burst transfer timer in CAN
uint32_t Time_USART = 0;                //Variable for the USART buffer fill timer
uint32_t Time_Update_Message = 0;       //Variable for return to the main message after receiving a USART message
uint32_t Pause_Update_DIS = 0;          //Variable to pause the DIS update for the duration of the data transfer EHU
String Prev_Message;
String Message_USART;
String message = "Starting Shild";

//********************************Tab function prototypes*****************************//
//Announcement of function prototypes from other tabs for correct function call.

void message_to_DIS (String);
String Bold(String);
String Normal(String);
String Right(String);
String Central(String);
void CAN_message_process(CanMsg*);
String data_to_str(int, int);

//************************************************************************************//
//********************Filling an array with USART characters**************************
String Data_USART() {
  String Buffer_USART;
  char u;
  while (Serial2.available() > 0 && u != '\n') { //read serial buffer until \n
    char u = Serial2.read();
    if (u != 0xD) Buffer_USART += u;  // skip \r
  }
#ifdef DEBUG
  Serial2.print(Buffer_USART);
#endif
  return Buffer_USART;
}
//*********************Generate alarm text******************************************
String Alarm(bool event) {
  if (event) return (">>TEMP ENGINE-" + data_to_str(T_ENG, 0) + "°C<<");
  else       return ("--TEMP ENGINE-" + data_to_str(T_ENG, 0) + "°C--");
}
//*************************************************************************************//
//Function prototypes and variables for working with CAN bus, function body in the "Init CAN" tab
#define CAN_RX_QUEUE_SIZE 36
HardwareCAN canBus(CAN1_BASE);
void CANSetup(void);
void SendCANmessage(long, byte, byte, byte, byte, byte, byte, byte, byte, byte);
//*************************************************************************************//

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  SETUP FUNCTIONS                                    //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void setup() {
  Serial2.begin(115200); // USART2 on A2-A3 pins
  Serial2.println("Starting Astra-H CAN-Shild");
  CANSetup() ;
  pinMode(PC13, OUTPUT); // LED
  pinMode(PC14, OUTPUT); // LED ERR
  digitalWrite(PC13, PC13ON);
  digitalWrite(PC13, PC13OFF);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                   LOOP FUNCTIONS                                    //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
void loop() {
  //*******************************Request for data from the ECC************************
  if ((millis() - time_request_ecc) > 1000) {
    SendCANmessage(0x0248, 8, 0x06, 0xAA, 0x01, 0x01, 0x07, 0x10, 0x11, 0x00);
    time_request_ecc = millis();
  }
  //*******************Generate an engine temperature alarm*****************************
  if (T_ENG > 1080) {
    alarm = 1;
    message = Alarm(Blink);
  }
  else alarm = 0;
  //*************************Receiving a message with USART2****************************
  if (millis() - Time_USART > 200) {   //delay needed to fillup buffers
    Message_USART = Data_USART();
    Time_USART = millis();
  }
  if ((Message_USART != "") && !alarm) {
    message = Central(Bold(Message_USART));
    Time_Update_Message = millis();
  }
  //******************************* Parameter display **********************************
  if (((millis() - Time_Update_Message) > 500) && !alarm) {
    message = Normal(data_to_str(T_ENG, 0)) + "°C" + "/" + data_to_str(VOLTAGE, 1) + "V";
    message += Right(Bold("by Gegerd"));
    Time_Update_Message = millis();  //To return to the main message after receiving a USART message
  }
    //***************Check CAN message buffer and process message*************************
  while(canBus.available() > 0)
    { CAN_message_process(canBus.recv());
     canBus.free();
    }
//******************************* Update display **********************************
 if (AUX_mode && ((millis() - time_send) > 350) &&
        (((millis() - Pause_Update_DIS) > 50))&& ((millis() - Pause_Update_DIS) < 4700)) { //Update display
    message_to_DIS(message);
    time_send = millis();
    if (Blink) Blink = 0;
    else Blink = 1;
#ifdef DEBUG
    Serial2.println(message);
#endif
  }
}
