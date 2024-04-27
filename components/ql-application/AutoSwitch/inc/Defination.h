#ifndef __DEFINATION_H__
#define __DEFINATION_H__

                    //Customer/release/features/patch/MinorBugFix 
#define  SOFT_VER    "01.02.07.000.000"
//01----customer  01 ---general customer Samruddhi
//01---Features   01-- SMS ,
//01---SMSCALL
//000 ---- Bug fix
//000 ---- Minor Bug Fix
//----------------HARDWARE CONFIGURATION---------------//
#define ACTIVE_SIM     0


//---------------------MCU UART----------------//
#define MCU_UART  	QL_UART_PORT_1
//---------------------DEBUG UART-------------//
#define DEBUG_UART  QL_UART_PORT_2


#define MAX_PH_SAVE	   5

#define MANUAL_MODE  0
#define AUTO_MODE    1

 #define SEPRATOR ':'      // seprator for sms code

 #define MIN_POD  10   // Min POD time setting
 #define MAX_POD  300  // Max POS time setting
 #define MIN_SDT  2   // Min SDT time setting
 #define MAX_SDT  60  // Max SDT time setting
 #define MIN_HV   350   // Min HIGH voltage setting
 #define MAX_HV   460  // Max HIGH voltage setting
 #define MIN_LV   200   // Min LOW voltage setting
 #define MAX_LV   350  // Max LOW voltage setting
 #define MIN_OC   0   // OVER CURRENT  setting
 #define MAX_OC   1000  // OVER CURRENT setting
 #define MIN_DC   0   // OVER CURRENT  setting
 #define MAX_DC   1000  // OVER CURRENT setting
 #define MIN_OLR  0   // MIN OVER LOAD RESTAT TIME  setting
 #define MAX_OLR  480  // MAX OVER LOAD RESTAT TIME  setting
 #define MIN_DRR  0   // MIN DRY RUN RESTAT TIME  setting
 #define MAX_DRR  480  // MAX DRY RUN RESTAT TIME  setting


#define PHONE_NU_CHECK    1

//------------FUNCTION--------------------//
//#define MQTT_ENABLE  		1
#define POWER_SAVING_EN 	1
//#define TTS_EN				1
//-------------enums---------------//
typedef enum
{
	OUTGOING = 0,
	INCOMMING = 1,
	NO_CALL = 2,
}Call_State;

typedef enum
{
	SMS_MODE = 0,
	MQTT_MODE = 1,
}reply_mode;

enum Faults
{
	
	NO_FAULT=0,
	LV_FAULT,
	HV_FAULT,
	PSL_FAULT,
	SPP_FAULT,
	PHASE_REV_FAULT,
	PWR_OFF_FAULT,
	DR_FAULT,
	OL_FAULT,
	UN_FAULT,
	PRE_PWR_OFF_FAULT,
}FaultState;

enum REPLY_MODE
{	
	SMS_REPLY=0,
	CALL_REPLY=1,
	NO_REPLY=2,
};

enum eventId
{
	
	ASW_FLASH_WRITE=QL_EVENT_APP_START,
	ASW_FLASH_READ,
	ASW_MQTT_OPEN,
	ASW_MQTT_PUB,
	ASW_PBK_SAVE,  //
	ASW_PBK_READ,
	ASW_CALL_HANG,
	ASW_AUDIO_WELCOME,
	ASW_AUDIO_TTS_VOLT,
	ASW_AUDIO_TTS_BAL,

}asw_event_id;
//-------- Flags Variables--------------//
struct Flag
{
	int MotorOn;
	int Power;
	int VFault;
	int CFault;
	int Dummy;
	int PreVFault;
	int PreCFault;
	int StausSend;
	int CallState;
	int HangUpcall;
	int PonStsSend;
	int MemoryRead;
	int BalSmsReply;
	
};

struct Setting
{
	uint16_t PodSetVal;
	uint16_t SdtSetVal;
	uint16_t OlSetVal;
	uint16_t DrSetVal;
	uint16_t OvSetVal;
	uint16_t UvSetVal;
	uint16_t OlRstTime;
	uint16_t DrRstTime;
	
	uint16_t Mode;
	uint16_t TestChar;
	uint16_t ReplyMode;
	
};

#endif //__DEFINATION_H__