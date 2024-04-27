#ifndef __SMS_H__
#define __SMS_H__

#define  MAX_RECV_MSG_LENGTH   100
#define  MAX_PH_LEN       12
#define  MAX_PH_SAVE      5

#define COMPANY_NUMBER "8600687322\0"
#define COMPANY_NUMBER_INDEX MAX_PH_SAVE+1


typedef struct {
    char  keyword[MAX_RECV_MSG_LENGTH];
    void  (* handler)(const char* MsgText, void* reserved);
}ST_SMS_HDLENTRY;

#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))


typedef struct PhoneNum
{
   char PhoneNumb[MAX_PH_LEN];
}ST_SaveNumber;

struct SavePhone
{
   ST_SaveNumber SaveNumbers[MAX_PH_SAVE];
   char ICCID_Array[22];
};

typedef struct {
	char  FaultString[20];
}ST_FAULT_STRING;

QlOSStatus AutoSwitch_sms_app_init(void);
char* StrToUpper(char* str);


void MsgHandler(const char *MsgText, void *reserved);
void PhoneNumberList(const char *MsgText, void *reserved);
void PhoneNumberSave(const char *MsgText, void *reserved);
void PhoneNumberDelete(const char *MsgText, void *reserved);
void SetPodValue(const char *MsgText, void *reserved);
void SetSdtValue(const char *MsgText, void *reserved);
void SetHighVoltageValue(const char *MsgText, void *reserved);
void SetLowVoltageValue(const char *MsgText, void *reserved);
void SetOverCurrentValue(const char *MsgText, void *reserved);
void SetDryCurrentValue(const char *MsgText, void *reserved);
void SetOverLoadRestart(const char *MsgText, void *reserved);
void SetDryRunRestart(const char *MsgText, void *reserved);
void GetSettings(const char *MsgText, void *reserved);
void GetStatus(const char *SmsText, void *reserved);
void SetAutoMode(const char *SmsText, void *reserved);
void SetManualMode(const char *SmsText, void *reserved);
void SetAutoCurrent (const char *SmsText, void *reserved);
void SetReplyMode(const char *SmsText, void *reserved);
void MotorOn(const char *SmsText, void *reserved);
void MotorOff(const char *SmsText, void *reserved);
void DeviceActivation(const char *SmsText, void *reserved);
void ResetSetting(const char *SmsText, void *reserved);
void FactoryReset(const char *SmsText, void *reserved);
void CheckBal(const char *SmsText, void *reserved);
void FotaUpdate(const char *SmsText, void *reserved);
void GetVersion(const char *SmsText, void *reserved);
void GetIMEI(const char *SmsText, void *reserved);
void PsrSetting(const char *SmsText, void *reserved);
void CurCalibration(const char *MsgText, void *reserved);
void VoltCalibration(const char *MsgText, void *reserved);

void SendFaultSms(uint8 VFault,uint8 Cfault);
void SendPowerOnSms(void);

void ReplyToUser(char *MobNumber, char *Text, void *mode);
void ReplyToUser2(char *MobNumber, char *Text);
uint16_t GetVal(char *Str, char ch);
uint8_t CheckMobNo(char* MobNo);

#endif  //__SMS_H__

