#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_common.h"
#include "ql_api_osi.h"
#include "ql_api_sms.h"
#include "ql_type.h"
#include "ql_log.h"
#include "ql_uart.h"
#include "ql_api_voice_call.h"
//----------FOTA FILE----------------//
#include "ql_api_datacall.h"
#include "ql_fs.h"
#include "ql_power.h"
#include "ql_api_dev.h"
#include "ql_api_fota.h"
#include "ql_ftp_client.h"
#include "ql_app_feature_config.h"
#include "fota_ftp.h"

#include "Defination.h"
#include "sms.h"
#include "uart.h"
#include "ql_api_sim.h"
#include "embedded_flash.h"
#include "phonebook.h"
#include "audio.h"
#include <ctype.h>
#include "ql_api_rtc.h"
#include "core_string.h"
#include "ql_api_ss.h"

ql_task_t sms_task = NULL;
ql_sem_t  sms_init_sem = NULL;
ql_sem_t  sms_list_sem = NULL;

extern ql_task_t pbk_task;
extern ql_event_t pbk_event;

extern ql_task_t embed_flash_task;
extern ql_event_t embed_flash_event;

char *SendSmsBuff = NULL;
char SmsRevMob[15];
char LastMob[15];
uint16_t TempVal = 0;
struct SavePhone SavePhNum;
struct Setting Settings;
struct Flag Flags;

int RY=0,YB=0,BR=0,Current=0;

uint8_t RevMobileIndex=0;

extern char DBG_BUFFER[];
extern char MCU_BUFFER[];


extern uint8_t ql_ussd_buffer[];
extern struct Audi Audio;

//-------------------------SMS CODES-----------------------------//
//-------------------------PHONE NUMBER SMS----------------------//
const char PHONE[] = "*PH\0";        // SAVE PHONE NUMBER
const char PHONE_LIST[] = "*PHL#\0"; // TO LIST THE PHONE NUMBER
const char PHONE_DEL[] = "*DELPH\0"; // DELETE PHONE NUMBER
//-------------------------SETTING SMS--------------------------//
const char POD_DELAY[] = "*DELAY1:\0";   // SET POD DELAY
const char SDT_DELAY[] = "*DELAY2:\0";   // SET SDT DELAY
const char HIGH_VOLT[] = "*VHI:\0";      // SET HIGH VOLTAGE
const char LOW_VOLT[] = "*VLO:\0";       // SET LOW VOLT
const char OVER_CURR[] = "*OLP:\0";      // SET OVER LOAD CURRENT
const char DRY_CURR[] = "*DRP:\0";       // SET DRY RUN CURRENT
const char OL_RESET[] = "*OLREST:\0";    // SET OVERLOAD RESTART TIME
const char DR_RESET[] = "*DRREST:\0";    // SET DRY RUN RESTART TIME
const char READ_SET[] = "*GETSET#\0";    // GET ALL SETTINGS
const char READ_STS[] = "*GETSTS#\0";    // GET STATUS
const char AUTO_MODE_SET[] = "*AUTO#\0"; // SET AUTO MODE
const char MAN_MODE[] = "*MAN#\0";       // MANUAL MODE
const char DEV_ACT[]="*DEVACT\0";         //DEVICE ACTIVATION
const char SET_RST[]="*RESET#\0";         //SETTING RESET
const char SET_FCT[]="*FACTORYRST*1234#";  //FACTORY RESET
const char GET_BAL[]="*BAL#";
const char SET_REPLYMOD[] = "*SETREPLY:\0"; // SET REPLY MODE
const char UPDATE[]="*UPDATE#\0";           // EC200U FOTA 
const char GET_VER[]="*GETVER#\0";          //VERSION OF EC200U_FW
const char GET_IMEI[]="*GETIMEI#\0";        //IMEI OF THE MODULE
const char PSR_ON[]="*PSR_ON#\0";           //PHASE REVERSAL ON   
const char PSR_OFF[]="*PSR_OFF#\0";         //PHASE REVERSAL OFF
const char CUR_CAL[]="*CUR_CAL:";   //CALIBRATE THE CURRENT
const char VOLT_CAL[]="*VOLT_CAL:";  //VOLT CALIBRATION

//----------------EC200U--->MCU CODES------------------------//
const char MOTOR_ON[] = "\r\nMOTOR_ON\r\n";
const char MOTOR_OFF[] = "\r\nMOTOR_OFF\r\n";
const char MODE_AUTO[] = "\r\nMODE_AUTO\r\n";
const char MODE_MAN[] = "\r\nMODE_MAN\r\n";
const char DRY_SET[] = "\r\nSET_DRY:";
const char OL_SET[] = "\r\nSET_OL:";
const char POD_SET[] = "\r\nSET_POD:";
const char SDT_SET[] = "\r\nSET_SDT:";
const char HV_SET[] = "\r\nSET_HV:";
const char LV_SET[] = "\r\nSET_LV:";
const char DRR_SET[] = "\r\nSET_DRR:";
const char OLR_SET[] = "\r\nSET_OLR:";
const char GET_SET[]="\r\nGET_SET\r\n";
const char GET_STATUS[]="\r\nGET_STATUS\r\n";
const char RESET[]="\r\nRESET\r\n";
const char AUTO_CURR[]="\r\nAUTO_CURR\r\n";

const char CALL_CONN[]="\r\nCALL_OPEN\r\n";
const char CALL_CUT[]="\r\nCALL_CLOSE\r\n";
const char PSR_EN[]="\r\nPSR_ON\r\n";
const char PSR_DIS[]="\r\nPSR_OFF\r\n";

const char CAL_CUR[]="\r\nCUR_CAL:";   //CALIBRATE THE CURRENT
const char CAL_VOLT[]="\r\nVOLT_CAL:";   //CALIBRATE THE VOLTAGE

const ST_SMS_HDLENTRY m_SmsHdlEntery[] = {
	{"*PHL#", PhoneNumberList},
	{"*PH", PhoneNumberSave},
	{"*DELPH", PhoneNumberDelete},
    {"*DELAY1:", SetPodValue},
    {"*DELAY2:", SetSdtValue},
	{"*VHI:", SetHighVoltageValue},
    {"*VLO:", SetLowVoltageValue},
    {"*OLP:", SetOverCurrentValue},
    {"*DRP:", SetDryCurrentValue},
    {"*OLREST:", SetOverLoadRestart},
    {"*DRREST:", SetDryRunRestart},
    {"*GETSET#", GetSettings},
    {"*GETSTS#", GetStatus},
    {"*AUTO#", SetAutoMode},
    {"*MAN#", SetManualMode},
    {"*ON#", MotorOn},
    {"*OFF#", MotorOff},
    {"*AUTOCURR#", SetAutoCurrent},
    {"*DEVACT", DeviceActivation},
    {"*RESET#", ResetSetting},
    {"*FACTORYRST*1234#", FactoryReset},
    {"*BAL#", CheckBal},
    {"*SETREPLY:",SetReplyMode},
    {"*UPDATE#",FotaUpdate},
    {"*GETVER#",GetVersion},
    {"*GETIMEI#",GetIMEI},
    {"*PSR_",PsrSetting},
    {"*CUR_CAL:",CurCalibration},
    {"*VOLT_CAL:",VoltCalibration},
    
};

const ST_FAULT_STRING FaultStr[]={
    {"NO FAULT"},
    {"LOW VOLT"},
    {"HIGH VOLT"},
    {"NO PHASE"},
    {"SPP"},
    {"PHASE REVERSE"},
    {"POWER OFF"},
    {"DRY RUN"},
    {"OVER LOAD"},
    {"UNKNOWN"},
    {"UNKNOWN"},
};


void user_sms_event_callback(uint8_t nSim, int event_id, void *ctx)
{ 
    ql_event_t sms_event = {0};
   
    switch(event_id)
	{
        case QL_SMS_INIT_OK_IND:
		{
			QL_SMS_LOG("QL_SMS_INIT_OK_IND");
			ql_rtos_semaphore_release(sms_init_sem);
			break;
		}
        case QL_SMS_NEW_MSG_IND:
		{
			
			ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
			QL_SMS_LOG("QL_SMS_NEW_MSG_IND");
			QL_SMS_LOG("sim=%d, index=%d, storage memory=%d", nSim, msg->index, msg->mem);
			sms_event.param1 = msg->index;
            break;
        }
        case QL_SMS_MEM_FULL_IND:
		{
			ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
			QL_SMS_LOG("QL_SMS_MEM_FULL_IND sim=%d, memory=%d",nSim,msg->mem);
			break;
		}
		default :
			break;

    }
    sms_event.id = event_id;
    //sms_event.param1 = msg->index;
    ql_rtos_event_send(sms_task, &sms_event);
}

void sms_demo_task(void * param)
{
 //   char addr[20] = {0};
    
    ql_event_t sms_event = {0};
    QL_SMS_LOG("sms_demo_task");
    ql_sms_callback_register(user_sms_event_callback);
    //wait sms ok
	if(ql_rtos_semaphore_wait(sms_init_sem, QL_WAIT_FOREVER)){
		QL_SMS_LOG("Waiting for SMS init timeout");
	}
    ql_sms_set_code_mode(QL_CS_GSM);
	ql_sms_delete_msg_ex(ACTIVE_SIM, 0, QL_SMS_DEL_ALL);
    ql_sms_set_storage(ACTIVE_SIM,SM,SM,SM);

    QL_SMS_LOG("SOFTWARE_VERSION %s",SOFT_VER);
    while (1)
    {
      ql_event_wait(&sms_event, QL_WAIT_FOREVER);
	  QL_SMS_LOG("QL_WAIT_FOREVER Break");
	  ql_sms_recv_s *sms_recv = (ql_sms_recv_s *)calloc(1,sizeof(ql_sms_recv_s));
	  QL_SMS_LOG("sms_event.id=%d",sms_event.id);
	  switch (sms_event.id)
	  {
	  case QL_SMS_NEW_MSG_IND:
		
		if(QL_SMS_SUCCESS == ql_sms_read_msg_ex(ACTIVE_SIM,sms_event.param1, TEXT,sms_recv))
		{
			QL_SMS_LOG("SMS:index=%d,os=%s,data=%s",sms_recv->index,sms_recv->oa,sms_recv->data);
			memset(SmsRevMob, '\0', sizeof(SmsRevMob));
       		memcpy(SmsRevMob, sms_recv->oa, strlen(sms_recv->oa));
			StrToUpper((char *)sms_recv->data);
			QL_SMS_LOG("SMS:UPPER: %s",sms_recv->data);
			QL_SMS_LOG("SMS:SmsRevMob %s",SmsRevMob);
            RevMobileIndex= CheckMobNo(SmsRevMob);
            if((RevMobileIndex!=0) || strstr((char *)sms_recv->data,DEV_ACT))
            {
             memset(LastMob, '\0', sizeof(SmsRevMob));
             memcpy(LastMob, sms_recv->oa, strlen(sms_recv->oa));
             QL_SMS_LOG("SMS:LastMob %s",LastMob);
             MsgHandler((const char *)sms_recv->data,(int *)SMS_MODE);
             
            }
            else 
            {
             QL_SMS_LOG("SMS:wrong mobile number\r\n");
            }
            if((Flags.BalSmsReply==1))
            {
                StrToUpper((char *)SmsRevMob);
                if(strstr(SmsRevMob,"JIO"))
                {
                    SendSmsBuff = malloc(200*sizeof(char));
	                memset(SendSmsBuff,'\0',200);
                    char *Ret = strchr((char *)sms_recv->data,':');
                    Ret++;
                    strcpy((char *)SendSmsBuff,(char *)Ret);
                    strcat((char *)SendSmsBuff,"\r\n");
                    ReplyToUser(LastMob, SendSmsBuff,NULL);
                    Flags.BalSmsReply=0;
                }
            }
			ql_sms_delete_msg_ex(ACTIVE_SIM, 0, QL_SMS_DEL_ALL);
		}
			if(sms_recv)free(sms_recv);
		break;
	  
	  default:
		break;
	  }

    }
    
}

QlOSStatus AutoSwitch_sms_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;

	err = ql_rtos_task_create(&sms_task, 4096, APP_PRIORITY_NORMAL, "QsmsApp", sms_demo_task, NULL, 2);
	if(err != QL_OSI_SUCCESS)
	{
		QL_SMS_LOG("sms_task created failed, ret = 0x%x", err);
	}
	
	err = ql_rtos_semaphore_create(&sms_init_sem, 0);
	if(err != QL_OSI_SUCCESS)
	{
		QL_SMS_LOG("sms_init_sem created failed, ret = 0x%x", err);
	}

	err = ql_rtos_semaphore_create(&sms_list_sem, 0);
	if(err != QL_OSI_SUCCESS)
	{
		QL_SMS_LOG("sms_init_sem created failed, ret = 0x%x", err);
	}

	return err;
}

//-----string to upper case------------//
char* StrToUpper(char* str)
{
    char* pCh = str;
    if (!str)
    {
        return NULL;
    }
    for ( ; *pCh != '\0'; pCh++)
    {
        if (((*pCh) >= 'a') && ((*pCh) <= 'z'))
        {
            *pCh = toupper((int )*pCh);
        }
    }
    return str;
}

void MsgHandler(const char *MsgText, void *reserved)
{
    uint16_t i;
  
    QL_SMS_LOG("\r\nSmsHandler:%s\r\n", MsgText);
    if (NULL == MsgText)
    {
        return;
    }

    for (i = 0; i < NUM_ELEMS(m_SmsHdlEntery); i++)
    {
        if (strstr(MsgText, m_SmsHdlEntery[i].keyword))
        {
            m_SmsHdlEntery[i].handler(MsgText, reserved);
            return;
        }
    }
}

//--------------- Phone number List-----------------//
void PhoneNumberList(const char *MsgText, void *reserved)
{
    uint8_t i = 0;
    char TempChar[20];
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    strcat(SendSmsBuff, "Dear Admin,\r\nPhone List:\r\n");
	QL_SMS_LOG("SMS:reserved %d",reserved);
    for (i = 1; i <= MAX_PH_SAVE; i++)
    {
		//QL_SMS_LOG("PH save %d",i);
        QL_SMS_LOG("SMS:PH%d=%s,Len=%d",i,SavePhNum.SaveNumbers[i - 1].PhoneNumb,strlen(SavePhNum.SaveNumbers[i-1].PhoneNumb));
		
        if (strlen(SavePhNum.SaveNumbers[i-1].PhoneNumb) >= 10)
        {
            memset(TempChar, '\0', 20);
            sprintf(TempChar, "PH%d-%s\r\n", i, SavePhNum.SaveNumbers[i-1].PhoneNumb);
            strcat(SendSmsBuff, TempChar);
        }
    }
    QL_SMS_LOG("SMS:PhoneListMsg:%s", SendSmsBuff);
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
    
	
}

//--------------------------SMS phone Number save----------------------------------------//
void PhoneNumberSave(const char *MsgText, void *reserved)
{
    char *p1 = NULL;
    char *p2 = NULL;
    uint8_t Index = 0;
    uint8_t Len = 0;
    QL_SMS_LOG("SMS:reserved %d",reserved);
    if (strstr(MsgText, PHONE))
    {
        p1 = strstr(MsgText, PHONE);
    again:
        p1 += strlen(PHONE);
        Index = p1[0] & 0x0f;
        p1++;
        p2 = strstr(p1, "#");
        Len = strlen(p1) - strlen(p2);
        if ((Len == 10) && (Index>0 && Index<=MAX_PH_SAVE))
        {
            if((Index==1) && ((RevMobileIndex ==1) || (RevMobileIndex == COMPANY_NUMBER_INDEX)))
            {
            strncpy(SavePhNum.SaveNumbers[Index - 1].PhoneNumb, p1, Len);
            SavePhNum.SaveNumbers[Index - 1].PhoneNumb[Len] = '\0';
            QL_SMS_LOG("SMS:PhoneNumber[%d]=%s", Index, SavePhNum.SaveNumbers[Index - 1].PhoneNumb);
            }
            else if (Index!=1)
            {
                strncpy(SavePhNum.SaveNumbers[Index - 1].PhoneNumb, p1, Len);
                SavePhNum.SaveNumbers[Index - 1].PhoneNumb[Len] = '\0';
                QL_SMS_LOG("SMS:PhoneNumber[%d]=%s", Index, SavePhNum.SaveNumbers[Index - 1].PhoneNumb);
            }

        }
        if (strstr(p2, PHONE))
        {
            p1 = strstr(p2, PHONE);
            goto again;
        }
        embed_flash_event.id =  ASW_FLASH_WRITE;
        ql_rtos_event_send(embed_flash_task, &embed_flash_event);
        ql_rtos_task_sleep_s(1);
        embed_flash_event.id =  ASW_FLASH_READ;
        ql_rtos_event_send(embed_flash_task, &embed_flash_event);  
        PhoneNumberList(NULL,reserved);
         
    }
}

void PhoneNumberDelete(const char *MsgText, void *reserved)
{
    char *p1 = NULL;
    uint8_t Index = 0;
    
    if (strstr(MsgText, PHONE_DEL))
    {
        p1 = strstr(MsgText, PHONE_DEL);
        p1 += strlen(PHONE_DEL);
        Index = p1[0] & 0x0f;
        if((Index!=1) || (RevMobileIndex==COMPANY_NUMBER_INDEX))
        {
       // memset(SavePhNum.SaveNumbers[Index-1].PhoneNumb,'0',MAX_PH_SAVE-1);
       // SavePhNum.SaveNumbers[Index-1].PhoneNumb[MAX_PH_SAVE-1]='\0';

        memset(SavePhNum.SaveNumbers[Index-1].PhoneNumb,0xFF,10);
        SavePhNum.SaveNumbers[Index-1].PhoneNumb[9]='\0';

        QL_SMS_LOG("SMS:PhoneNumber[%d]=%s\r\n", Index, SavePhNum.SaveNumbers[Index - 1].PhoneNumb);
        PhoneNumberList(NULL,NULL);

        embed_flash_event.id =  ASW_FLASH_WRITE;
        ql_rtos_event_send(embed_flash_task, &embed_flash_event);
        ql_rtos_task_sleep_s(1);
        }
        else
        {
            SendSmsBuff = malloc(150*sizeof(char));
	        memset(SendSmsBuff,'\0',150);
            sprintf(SendSmsBuff, "Dear Admin,\r\nAccess Denied..!");
            ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
        }

    }
}

//------------------FUNCTION:POD VALUE-------------------------//

void SetPodValue(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal(( char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:POD VALUE SET:%d\r\n", TempVal);
    if ((TempVal >= MIN_POD) && (TempVal <= MAX_POD))
    {
        Settings.PodSetVal = TempVal;
        sprintf(SendSmsBuff, "Dear Admin,\r\nDelay1 Set to %d sec\r\n", Settings.PodSetVal);
        MCU_UART_WRITE((char *)"%s%d\r\n", POD_SET, Settings.PodSetVal); ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s%d\r\n", POD_SET, Settings.PodSetVal); 
    }
    else
    {
        sprintf(SendSmsBuff, "Dear Admin,\r\nDelay1 setting is out of range.\r\nRange is %d to %d sec", MIN_POD, MAX_POD);
    }
     ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}


//------------------FUNCTION:SDT VALUE-------------------------//
void SetSdtValue(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal(( char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:SDT VALUE SET:%d\r\n", TempVal);
    if ((TempVal >= MIN_SDT) && (TempVal <= MAX_SDT))
    {
        Settings.SdtSetVal = TempVal;
        sprintf(SendSmsBuff, "Dear Admin,\r\nDelay2 Set to %d sec\r\n", Settings.SdtSetVal);
        MCU_UART_WRITE("%s%d\r\n", SDT_SET, Settings.SdtSetVal);ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s%d\r\n", SDT_SET, Settings.SdtSetVal);
    }
    else
    {
        sprintf(SendSmsBuff, "Dear Admin,\r\nDelay2 setting is out of range.\r\nRange is %d to %d sec", MIN_SDT, MAX_SDT);
    }
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:HIGH VOLTAGE VALUE-------------------------//
void SetHighVoltageValue(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal(( char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:HIGH VOLT VALUE SET:%d\r\n", TempVal);
    if ((TempVal >= MIN_HV) && (TempVal <= MAX_HV))
    {
        Settings.OvSetVal = TempVal;
        sprintf(SendSmsBuff, "Dear Admin,\r\nHIGH VOLT Set to %d Volt\r\n", Settings.OvSetVal);
        MCU_UART_WRITE("%s%d\r\n", HV_SET, Settings.OvSetVal);ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s%d\r\n", HV_SET, Settings.OvSetVal);
    }
    else
    {
        sprintf(SendSmsBuff, "Dear Admin,\r\nHIGH VOLT setting is out of range.\r\nRange is %d to %d Volt", MIN_HV, MAX_HV);
    }
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:LOW VOLTAGE VALUE-------------------------//
void SetLowVoltageValue(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal((char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:LOW VOLT VALUE SET:%d\r\n", TempVal);
    if ((TempVal >= MIN_LV) && (TempVal <= MAX_LV))
    {
        Settings.UvSetVal = TempVal;
        sprintf(SendSmsBuff, "Dear Admin,\r\nLOW VOLT Set to %d Volt\r\n", Settings.UvSetVal);
        MCU_UART_WRITE("%s%d\r\n", LV_SET, Settings.UvSetVal);ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s%d\r\n", LV_SET, Settings.UvSetVal);
    }
    else
    {
        sprintf(SendSmsBuff, "Dear Admin,\r\nLOW VOLT setting is out of range.\r\nRange is %d to %d Volt", MIN_LV, MAX_LV);
    }
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:OVER CURRENT VALUE-------------------------//
void SetOverCurrentValue(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal((char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:OVER CURRENT VALUE SET:%d\r\n", TempVal);
    if (((TempVal >= MIN_OC) && (TempVal <= MAX_OC)) || (TempVal==0))
    {
        Settings.OlSetVal = TempVal;
        sprintf(SendSmsBuff, "Dear Admin,\r\nOver Load Current Set to %d \r\n", Settings.OlSetVal);
        MCU_UART_WRITE("%s%d\r\n", OL_SET, Settings.OlSetVal);ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s%d\r\n", OL_SET, Settings.OlSetVal);
    }
    else
    {
        sprintf(SendSmsBuff, "Dear Admin,\r\nOver Current setting is out of range.\r\nRange is %d to %d ", MIN_OC, MAX_OC);
    }
     ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:DRY CURRENT VALUE-------------------------//
void SetDryCurrentValue(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal((char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:DRY CURRENT VALUE SET:%d\r\n", TempVal);
    if (((TempVal >= MIN_DC) && (TempVal <= MAX_DC)) || (TempVal==0))
    {
        Settings.DrSetVal = TempVal;
        sprintf(SendSmsBuff, "Dear Admin,\r\nDry Run Current Set to %d \r\n", Settings.DrSetVal);
        MCU_UART_WRITE("%s%d\r\n", DRY_SET, Settings.DrSetVal);ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s%d\r\n", DRY_SET, Settings.DrSetVal);
    }
    else
    {
        sprintf(SendSmsBuff, "Dear Admin,\r\nDry Run Current setting is out of range.\r\nRange is %d to %d ", MIN_DC, MAX_DC);
    }
     ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:OVER LOAD RESTART VALUE-------------------------//
void SetOverLoadRestart(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal((char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:Overload Restart Time SET:%d\r\n", TempVal);
    if ((TempVal >= MIN_OLR) && (TempVal <= MAX_OLR))
    {
        Settings.OlRstTime = TempVal;
        sprintf(SendSmsBuff, "Dear Admin,\r\nOverload Restart Time Set to %d min \r\n", Settings.OlRstTime);
        MCU_UART_WRITE("%s%d\r\n", OLR_SET, Settings.OlRstTime);ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s%d\r\n", OLR_SET, Settings.OlRstTime);
    }
    else
    {
        sprintf(SendSmsBuff, "Dear Admin,\r\nOverload Restart Time setting is out of range.\r\nRange is %d to %d min", MIN_OLR, MAX_OLR);
    }
     ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}
//------------------FUNCTION:DRY RUN RESTART VALUE-------------------------//
void SetDryRunRestart(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal((char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:DryRun Restart SET:%d\r\n", TempVal);
    if ((TempVal >= MIN_DRR) && (TempVal <= MAX_DRR))
    {
        Settings.DrRstTime = TempVal;
        sprintf(SendSmsBuff, "Dear Admin,\r\nDryRun Restart Time Set to %d min \r\n", Settings.DrRstTime);
        MCU_UART_WRITE("%s%d\r\n", DRR_SET, Settings.DrRstTime);ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s%d\r\n", DRR_SET, Settings.DrRstTime);
    }
    else
    {
        sprintf(SendSmsBuff, "Dear Admin,\r\nDryRun Restart Time setting is out of range.\r\nRange is %d to %d min", MIN_DRR, MAX_DRR);
    }
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:GET SETTING-------------------------//
void GetSettings(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(200*sizeof(char));
	memset(SendSmsBuff,'\0',200);
    MCU_UART_WRITE("%s", GET_SET);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s", GET_SET);
    ql_rtos_task_sleep_s(1);
    sprintf(SendSmsBuff, "Dear Admin,\r\nSET:\r\nHV=%d\r\nLV=%d\r\nDR=%d\r\nOLCur=%d\r\nDryRst=%d\r\nOLRst=%d\r\nDel1=%d\r\nDel2=%d\r\n",
               Settings.OvSetVal, Settings.UvSetVal, Settings.DrSetVal, Settings.OlSetVal, Settings.DrRstTime, Settings.OlRstTime, Settings.PodSetVal, Settings.SdtSetVal);
    if (Settings.Mode == MANUAL_MODE)
    {
        strcat(SendSmsBuff, "MODE=MAN\r\n");//Ql_Sleep(1000);
    }
    if (Settings.Mode == AUTO_MODE)
    {
    	strcat(SendSmsBuff, "MODE=AUTO\r\n");//Ql_Sleep(1000);
    }
    if (Settings.ReplyMode == CALL_REPLY)
    {
    	strcat(SendSmsBuff, "RMODE=CALL\r\n");//Ql_Sleep(1000);
    }
    if (Settings.ReplyMode == SMS_REPLY)
    {
    	strcat(SendSmsBuff, "RMODE=SMS\r\n");//Ql_Sleep(1000);
    }
    if (Settings.ReplyMode == NO_REPLY)
    {
    	strcat(SendSmsBuff, "RMODE=NO\r\n");//Ql_Sleep(1000);
    }

    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}
//------------------FUNCTION:GET STATUS-------------------------//
void GetStatus(const char *SmsText, void *reserved)
{
    char TempArray[100];
    SendSmsBuff = malloc(200*sizeof(char));
	memset(SendSmsBuff,'\0',200);
    MCU_UART_WRITE("%s", GET_STATUS);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s", GET_STATUS);
    ql_rtos_task_sleep_s(1);
    if((Flags.CFault==NO_FAULT) && (Flags.VFault==NO_FAULT) && (Flags.Power==TRUE))
    {
        if(Flags.MotorOn==TRUE)
         strcpy(SendSmsBuff,"Dear Admin\r\nPOWER IS ON MOTOR IS ON\r\n");
        else
         {
            strcpy(SendSmsBuff,"Dear Admin\r\nPOWER IS ON MOTOR IS OFF\r\n");
			Current = 0;
         }
    }
    else
    {
        //strcpy(SendSmsBuff,"Dear Admin\r\nDue To Fault MOTOR IS OFF\r\n");
		strcpy(SendSmsBuff,"Dear Admin\r\nDue To ");//
        if(Flags.VFault!=NO_FAULT)
         strcat(SendSmsBuff, FaultStr[Flags.VFault].FaultString);
        if (Flags.CFault!=NO_FAULT)
             strcat(SendSmsBuff, FaultStr[Flags.CFault].FaultString);
        strcat(SendSmsBuff, " Fault MOTOR IS OFF\r\n");
        
        Current = 0; 
    }
     sprintf(TempArray,"RY-VOLT=%03d\r\nYB-VOLT=%03d\r\nBR-VOLT=%03d\r\nCURRENT=%d.%d\r\n",RY,YB,BR,(Current/10),(Current%10));
     strcat(SendSmsBuff,TempArray);
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:SetAutoMode-------------------------//
void SetAutoMode(const char *SmsText, void *reserved)
{
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    QL_SMS_LOG("SMS:SET_AUTO_MODE\r\n", TempVal);
    APP_DEBUG("AUTO MODE SET\r\n");
    sprintf(SendSmsBuff, "Dear Admin,\r\nAUTO MODE SET\r\n");
    Settings.Mode = AUTO_MODE;
    MCU_UART_WRITE("%s", MODE_AUTO);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s", MODE_AUTO);
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:Set MANUAL MODE-------------------------//
void SetManualMode(const char *SmsText, void *reserved)
{
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    QL_SMS_LOG("SMS:SET_MANUAL_MODE\r\n", TempVal);
    APP_DEBUG("MANUAL MODE SET\r\n");
    sprintf(SendSmsBuff, "Dear Admin,\r\nMANUAL MODE SET\r\n");
    Settings.Mode = AUTO_MODE;
    MCU_UART_WRITE("%s", MODE_MAN);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s", MODE_MAN);
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:Set AUTO CURRENT-------------------------//
void SetAutoCurrent (const char *SmsText, void *reserved)
{
   SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    QL_SMS_LOG("SMS: AUTO CURRENT SET\r\n");
    MCU_UART_WRITE("%s", AUTO_CURR);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s", AUTO_CURR);ql_rtos_task_sleep_s(1);
    ql_rtos_task_sleep_s(2);
    sprintf(SendSmsBuff, "Dear Admin,\r\nCURRENT SETTING:DryRunCu=%d\r\nOLCur=%d\r\n",Settings.DrSetVal, Settings.OlSetVal);
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}
//------------------FUNCTION:TURN ON THE MOTOR-------------------------//
void MotorOn(const char *SmsText, void *reserved)
{
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    MCU_UART_WRITE("%s", GET_STATUS);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s", GET_STATUS);ql_rtos_task_sleep_s(1);
   // if(Settings.Mode==MANUAL_MODE)
   if(Flags.MotorOn==FALSE)
    {
        if( (Flags.VFault==NO_FAULT))  //(Flags.CFault==NO_FAULT) &&
        {
            MCU_UART_WRITE("%s", MOTOR_ON);ql_rtos_task_sleep_s(1);
            MCU_UART_WRITE("%s", MOTOR_ON);
            strcpy(SendSmsBuff,"Dear Admin,\r\nMOTOR TURNED ON\r\n");
            
        }
        else
        {
            strcpy(SendSmsBuff,"Dear Admin\r\nDue To ");//
            if(Flags.VFault!=NO_FAULT)
              strcat(SendSmsBuff, FaultStr[Flags.VFault].FaultString);
            if (Flags.CFault!=NO_FAULT)
              strcat(SendSmsBuff, FaultStr[Flags.CFault].FaultString);
            strcat(SendSmsBuff, " Fault MOTOR IS OFF\r\n");
        }
    }
    else 
    {
        if((Flags.MotorOn==TRUE) && (Settings.Mode == AUTO_MODE)) 
           strcpy(SendSmsBuff,"Dear Admin,\r\nMODE = AUTO MODE\r\n MOTOR IS ALREADY ON\r\n");
        else  if((Flags.MotorOn==TRUE) && (Settings.Mode == MANUAL_MODE)) 
           strcpy(SendSmsBuff,"Dear Admin,\r\nMODE = MANUAL MODE\r\n MOTOR IS ALREADY ON\r\n");
    }
    //ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
    //ReplyToUser(SavePhNum.SaveNumbers[0].PhoneNumb, SendSmsBuff,(int *)reserved);
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
    //if(SendSmsBuff)free(SendSmsBuff);
}

//------------------FUNCTION:TURN OFF THE MOTOR-------------------------//
void MotorOff(const char *SmsText, void *reserved)
{
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    MCU_UART_WRITE("%s", GET_STATUS);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s", GET_STATUS);ql_rtos_task_sleep_s(1);
    if(Flags.MotorOn==TRUE)
    {
        MCU_UART_WRITE("%s", MOTOR_OFF);ql_rtos_task_sleep_s(1);
        MCU_UART_WRITE("%s", MOTOR_OFF);
        if(Settings.Mode == AUTO_MODE)
           strcpy(SendSmsBuff,"Dear Admin,\r\nMODE = AUTO MODE\r\n MOTOR TURNED OFF\r\n");
        else  
           strcpy(SendSmsBuff,"Dear Admin,\r\nMODE = MANUAL MODE\r\n MOTOR TURNED OFF\r\n");
    }
    else 
    {
        if(Settings.Mode == AUTO_MODE)
           strcpy(SendSmsBuff,"Dear Admin,\r\nMODE = AUTO MODE\r\n MOTOR IS ALREADY OFF\r\n");
        else  
           strcpy(SendSmsBuff,"Dear Admin,\r\nMODE = MANUAL MODE\r\n MOTOR IS ALREADY OFF\r\n");
    }
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
    
    //ReplyToUser(SavePhNum.SaveNumbers[0].PhoneNumb, SendSmsBuff,(int *)reserved);
    //if(SendSmsBuff)free(SendSmsBuff);
}

//------------------FUNCTION:DeviceActivation-------------------------//
void DeviceActivation(const char *SmsText, void *reserved)
{
    char *p1 = NULL;
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    APP_DEBUG("\r\n Device Activation\r\n");
    
    if (strstr(SmsText, DEV_ACT) && ((SavePhNum.ICCID_Array[2]==0xFF) || (RevMobileIndex==1) ||(RevMobileIndex=COMPANY_NUMBER_INDEX)))
    {
        ql_sim_get_iccid(ACTIVE_SIM, SavePhNum.ICCID_Array, 20);
        p1 = strstr(SmsText, DEV_ACT);
        p1 += strlen(DEV_ACT);
        p1[strlen(p1)-1]='\0';
        strcpy(SavePhNum.SaveNumbers[0].PhoneNumb,p1);
        Settings.ReplyMode = SMS_REPLY;
        embed_flash_event.id =  ASW_FLASH_WRITE;
        ql_rtos_event_send(embed_flash_task,&embed_flash_event);

    }
    sprintf(SendSmsBuff, "Dear Buyer,\r\nThanks for choosing SAMRUDHI GSM AUTOSWITCH\r\nWelcome to Smart Farming\r\n");
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------FUNCTION:SET REPLY MODE-------------------------//

void SetReplyMode(const char *SmsText, void *reserved)
{
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
   if (strstr(SmsText, SET_REPLYMOD))
    {
       
        if(strstr(SmsText, "SMS"))
        {
          strcpy(SendSmsBuff,"Dear Admin,\r\nREPLY MODE SET = SMS\r\n");     
          Settings.ReplyMode = SMS_REPLY;
        }
        if(strstr(SmsText, "CALL"))
        {
          strcpy(SendSmsBuff,"Dear Admin,\r\nREPLY MODE SET = CALL\r\n");     
          Settings.ReplyMode = CALL_REPLY;
        }
        if(strstr(SmsText, "OFF"))
        {
         strcpy(SendSmsBuff,"Dear Admin,\r\nREPLY MODE SET = OFF\r\n");  
         Settings.ReplyMode = NO_REPLY;
        }
        //WriteNumberInMemory(REPLY_INDEX,Settings.ReplyMode);
        embed_flash_event.id =  ASW_FLASH_WRITE;
        ql_rtos_event_send(embed_flash_task, &embed_flash_event);
        ql_rtos_task_sleep_s(1);
        embed_flash_event.id =  ASW_FLASH_READ;
        ql_rtos_event_send(embed_flash_task, &embed_flash_event);  
        ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
    }
}
//------------------FUNCTION:RESET-------------------------//
void ResetSetting(const char *SmsText, void *reserved)
{
    if((RevMobileIndex==1) || (RevMobileIndex==COMPANY_NUMBER_INDEX))
    {
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    MCU_UART_WRITE("%s",RESET);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s",RESET);
    strcpy(SendSmsBuff,"Dear Admin,\r\nAll Settings are set to Default\r\n");
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
    }
}

//------------------FUNCTION:RESET-------------------------//
void FactoryReset(const char *SmsText, void *reserved)
{
    u8 Index=0;
    if(RevMobileIndex==COMPANY_NUMBER_INDEX)
    {
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
   
    for(Index=1;Index<=MAX_PH_SAVE;Index++)
    {
       // memset(SavePhNum.SaveNumbers[Index].PhoneNumb,'0',MAX_PH_SAVE-1);
       // SavePhNum.SaveNumbers[Index].PhoneNumb[MAX_PH_SAVE]='\0';

        memset(SavePhNum.SaveNumbers[Index-1].PhoneNumb,0xFF,10);
        SavePhNum.SaveNumbers[Index-1].PhoneNumb[9]='\0';

        ///WriteDataInMemory(Index+1,SavePhNum.SaveNumbers[Index].PhoneNumb);
        ql_rtos_task_sleep_ms(500);
    }
    Settings.ReplyMode = SMS_REPLY;
   // WriteNumberInMemory(REPLY_INDEX, Settings.ReplyMode);
   QL_SMS_LOG("before write SMS REC FACTORY %s",SmsRevMob);
    embed_flash_event.id =  ASW_FLASH_WRITE;
    ql_rtos_event_send(embed_flash_task, &embed_flash_event);  
    strcpy(SendSmsBuff,"Dear Admin,\r\nDevice Reset To Factory\r\n");
    QL_SMS_LOG("SMS REC FACTORY %s",SmsRevMob);
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
    MCU_UART_WRITE("%s",RESET);ql_rtos_task_sleep_s(1);
    MCU_UART_WRITE("%s",RESET);
    }
}
//------------------FUNCTION: check bal-------------------------//
void CheckBal(const char *SmsText, void *reserved)
{ 
//   char TempArray[50];
//   char *p=NULL;
//   SendSmsBuff = Ql_MEM_Alloc(150);
//   RIL_NW_GetOperator(TempArray);
//   APP_DEBUG("Operator Name:%s\r\n",TempArray);
   
  
//   p=Ql_strstr(SmsText,GET_BAL);
//   p+=Ql_strlen(GET_BAL);
//   APP_DEBUG("Bal Code:%s\r\n",p);
//   Ql_memset(TempArray,'\0',50);
//   Ql_sprintf(TempArray,"AT+CUSD=1,\"%s\"\r\n",p);
//   Ql_RIL_SendATCmd(TempArray,Ql_strlen(TempArray), ATResponseCUSD_Handler,SendSmsBuff, 10000);
//   APP_DEBUG("Bal Code: %s",SendSmsBuff);
//   SMS_TextMode_Send(SmsRevMob, SendSmsBuff);

    if(CheckBalance()==1)
    {
        if(strlen((char *)ql_ussd_buffer)>200)
        {
            char OutputStr[200]={0};
            uint16 Len=0,Cnt=0;
            Len = strlen((char *)ql_ussd_buffer);
            QL_SMS_LOG("SMS:strlen %d",Len);

            for(Cnt=0;Cnt<Len/2;Cnt++)
            {
                ql_ussd_buffer[Cnt] = ql_ussd_buffer[3+(Cnt%2)+((Cnt/2)*2)];
            }
            ql_ussd_buffer[Len/2]='\0';
            Len = strlen((char *)ql_ussd_buffer);
            QL_SMS_LOG("SMS:Newstrlen%d  %s",Len,ql_ussd_buffer);

            int32_t ret=core_str2hex((char *)ql_ussd_buffer,strlen((char *)ql_ussd_buffer),(uint8_t *)OutputStr);
            QL_SMS_LOG("SMS:Output=%d   %s",ret,OutputStr);
            memset(ql_ussd_buffer,0x00,QUEC_SS_USSD_UCS2_SIZE_MAX);
            strcpy((char *)ql_ussd_buffer,(char *)OutputStr);

        }
        
        if(strlen((char *)ql_ussd_buffer)<200)
        {
        SendSmsBuff = malloc(200);
        memset(SendSmsBuff,'\0',200);
        ql_rtos_task_sleep_s(2);
       
        //strcpy(SendSmsBuff,(char *)ql_ussd_buffer);
        //else
        strcpy(SendSmsBuff,(char *)ql_ussd_buffer);
        strcat(SendSmsBuff,"\r\n");
        QL_SMS_LOG("SMS:BLA: %s",SendSmsBuff);
       // ql_rtos_task_sleep_s(2);
        ReplyToUser(SmsRevMob,SendSmsBuff,(int *)reserved);
        }
    }


}

//------------------FUNCTION: SEND FAULT SMS-------------------------//
void SendFaultSms(u8 VFault,u8 Cfault)
{
    char TempArray[100];
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    strcpy(SendSmsBuff,"Dear Admin\r\nMotor is OFF\r\n");
   
    switch (Cfault)
    {
    case DR_FAULT:
        strcat(SendSmsBuff,"Fault:Dry Run\r\n");
        Audio.PlayList = DRYRUN_FAULT_AUDIO;
        break;
    case OL_FAULT:
        strcat(SendSmsBuff,"Fault:OverLoad\r\n");
        Audio.PlayList = OL_FAULT_AUDIO;
        break;
       case UN_FAULT:
        strcat(SendSmsBuff,"Fault:UNKNOWN\r\n");
        Audio.PlayList = UN_M_OFF_AUDIO;
        break; 
    default:
        break;
    }

    switch (VFault)
    {
    case LV_FAULT:
        strcat(SendSmsBuff,"Fault:Low Volt\r\n");
        Audio.PlayList = SPP_FAULT_AUDIO;
        break;
    case HV_FAULT:
        strcat(SendSmsBuff,"Fault:High Volt\r\n");
        Audio.PlayList = SPP_FAULT_AUDIO;
        break;
    case PSL_FAULT:
        strcat(SendSmsBuff,"Fault:NO Phase\r\n");
        Audio.PlayList = SPP_FAULT_AUDIO;
        break;
    case SPP_FAULT:
        strcat(SendSmsBuff,"Fault:SPP\r\n");
        Audio.PlayList = SPP_FAULT_AUDIO;
        break;
    case PHASE_REV_FAULT:
        strcat(SendSmsBuff,"Fault:PHASE REVERSE\r\n");
        Audio.PlayList = RP_FAULT_AUDIO;
        break;
    case PWR_OFF_FAULT:
        strcat(SendSmsBuff,"Fault:POWER OFF\r\n");
        Audio.PlayList = POWER_OFF_MOTOR_OFF_AUDIO;
        break;    
    default:
        break;
    }

    
    sprintf(TempArray,"RY=%d\r\nYB=%d\r\nBR=%d\r\nCurr=%d.%d\r\nMode=",RY,YB,BR,(Current/10),(Current%10));
    if(Settings.Mode==MANUAL_MODE)
    strcat(TempArray,"MANUAL\r\n");
    else
    strcat(TempArray,"AUTO\r\n");
    strcat(SendSmsBuff,TempArray);
    if(Settings.ReplyMode==SMS_REPLY)
    {
        ReplyToUser(SavePhNum.SaveNumbers[0].PhoneNumb, SendSmsBuff,(int *)NULL);
    }
    else if(Settings.ReplyMode==CALL_REPLY)
    {
       // DialCall(SavePhNum.SaveNumbers[PH1_INDEX-1].PhoneNumb);
       if((strlen((char *)SavePhNum.SaveNumbers[0].PhoneNumb)>=10) && (SavePhNum.SaveNumbers[0].PhoneNumb[3]!=0xFF))
       { 
        
        uint32_t Ret =  ql_voice_call_start(ACTIVE_SIM,SavePhNum.SaveNumbers[0].PhoneNumb);
        Flags.CallState = OUTGOING;
        QL_SMS_LOG("SMS:DIAL=%d , Mob=%s\r\n", Ret,SavePhNum.SaveNumbers[0].PhoneNumb);
        }
        else
        QL_SMS_LOG("SMS:DIAL wrong Mob=%s\r\n",SavePhNum.SaveNumbers[0].PhoneNumb);

        if(SendSmsBuff)free(SendSmsBuff);
    }
    else
    {
        if(SendSmsBuff)free(SendSmsBuff);
    }

     //if(SendSmsBuff)free(SendSmsBuff);
    Flags.PonStsSend = TRUE;
  
}

//------------------FUNCTION: SEND POWER ON SMS-------------------------//
void SendPowerOnSms(void)
{
     if(Flags.CFault==NO_FAULT && Flags.VFault==NO_FAULT)
     {
          SendSmsBuff = malloc(150);
          strcpy(SendSmsBuff,"Dear Admin,\r\nPOWER=");
        if(Flags.Power==TRUE)
        {
            if(Flags.MotorOn==TRUE)
            {
            strcat(SendSmsBuff,"ON\r\nMOTOR:ON\r\n");
            Audio.PlayList = POWER_ON_MOTOR_ON_AUDIO;
            }
            else
            {
            strcat(SendSmsBuff,"ON\r\nMOTOR:OFF\r\n");
            Audio.PlayList = POWER_ON_MOTOR_OFF_AUDIO;
            }
        }
        else
        {
          strcat(SendSmsBuff,"OFF\r\nMOTOR:OFF\r\n");
          Audio.PlayList = POWER_OFF_MOTOR_OFF_AUDIO;
        }

        if(Settings.Mode==MANUAL_MODE)
        strcat(SendSmsBuff,"MODE:MANUAL\r\n");
        else
        strcat(SendSmsBuff,"MODE:AUTO\r\n");
        if(Settings.ReplyMode==SMS_REPLY)
        {
            //SMS_TextMode_Send(SavePhNum.SaveNumbers[PH1_INDEX-1].PhoneNumb, SendSmsBuff);
            ReplyToUser(SavePhNum.SaveNumbers[ADMIN1_INDEX-1].PhoneNumb, SendSmsBuff,(int *)NULL);
        }
        else if(Settings.ReplyMode==CALL_REPLY)
        {

            //DialCall(SavePhNum.SaveNumbers[PH1_INDEX-1].PhoneNumb);
            if(Flags.CallState == NO_CALL)
            {
                if((strlen((char *)SavePhNum.SaveNumbers[ADMIN1_INDEX-1].PhoneNumb)>=10) && (SavePhNum.SaveNumbers[ADMIN1_INDEX-1].PhoneNumb[3]!=0xFF))
                {
                     uint32_t Ret = ql_voice_call_start(ACTIVE_SIM,SavePhNum.SaveNumbers[ADMIN1_INDEX-1].PhoneNumb);
                     Flags.CallState = OUTGOING;
                     QL_SMS_LOG("SMS:DIAL=%d , Mob=%s\r\n", Ret,SavePhNum.SaveNumbers[ADMIN1_INDEX-1].PhoneNumb);
                }else
                {
                    QL_SMS_LOG("SMS:DIAL wrong Mob=%s\r\n",SavePhNum.SaveNumbers[ADMIN1_INDEX-1].PhoneNumb);
                }
            }
           if(SendSmsBuff)free(SendSmsBuff);
        }
        else
        {
            if(SendSmsBuff)free(SendSmsBuff);
        }

     }
     else if ((Flags.PreVFault!=Flags.VFault) || (Flags.PreCFault!=Flags.CFault))
     {
        SendFaultSms(Flags.VFault,Flags.CFault);
        Flags.PreVFault=Flags.VFault;
        Flags.PreCFault=Flags.CFault;
     }
     Flags.PonStsSend = TRUE;
}

//------------------FUNCTION: UPDATE THE SOFTWARE-------------------------//
void FotaUpdate(const char *SmsText, void *reserved)
{
    ASW_fota_ftp_app_init();
}

void GetVersion(const char *SmsText, void *reserved)
{
    SendSmsBuff = malloc(150);
    memset(SendSmsBuff,'\0',150);
    strcpy(SendSmsBuff,"Dear Admin\r\nGSM SOFT VER:\r\n");
    strcat(SendSmsBuff,SOFT_VER);
    strcat(SendSmsBuff,"\r\n");
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

void GetIMEI(const char *SmsText, void *reserved)
{
    if((RevMobileIndex == COMPANY_NUMBER_INDEX))
    {
    SendSmsBuff = malloc(150);    
    char IMEI_sms[64];
    ql_dev_get_imei( IMEI_sms, 64, 0);    
    
    memset(SendSmsBuff,'\0',150);
    strcpy(SendSmsBuff,"Dear Admin\r\nIMEI:");
    strcat(SendSmsBuff,IMEI_sms);
    strcat(SendSmsBuff,"\r\n");
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
    }
}

void PsrSetting(const char *SmsText, void *reserved)
{
    SendSmsBuff = malloc(150); 
    memset(SendSmsBuff,'\0',150);
    strcpy(SendSmsBuff,"Dear Admin\r\n");
    if(strstr(SmsText,PSR_ON))
    {
          strcat(SendSmsBuff,"Phase Rev Enabled\r\n");
          MCU_UART_WRITE("%s", PSR_EN); 
    }
    else if(strstr(SmsText,PSR_OFF))
    {
          strcat(SendSmsBuff,"Phase Rev Disabled\r\n");
          MCU_UART_WRITE("%s", PSR_DIS);

    }
    ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//----------------------current calibration------------------------//

void CurCalibration(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal((char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:Current Calibration:%d\r\n", TempVal);
   
        sprintf(SendSmsBuff, "Current Callibration to %d\r\n", TempVal);
        MCU_UART_WRITE("%s%d\r\n", CAL_CUR, TempVal);ql_rtos_task_sleep_s(1);
        //MCU_UART_WRITE("%s%d\r\n", CAL_CUR, TempVal);
   
    // else
    // {
    //     sprintf(SendSmsBuff, "Dear Admin,\r\nOver Restart Time setting is out of range.\r\nRange is %d to %d min", MIN_OLR, MAX_OLR);
    // }
     ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//----------------------Voltage calibration------------------------//

void VoltCalibration(const char *MsgText, void *reserved)
{
    SendSmsBuff = malloc(150*sizeof(char));
	memset(SendSmsBuff,'\0',150);
    TempVal = GetVal((char *)MsgText, SEPRATOR);
    QL_SMS_LOG("SMS:Current Calibration:%d\r\n", TempVal);
    
        sprintf(SendSmsBuff, "Voltage Callibration to %d\r\n", TempVal);
        MCU_UART_WRITE("%s%d\r\n", CAL_VOLT, TempVal);ql_rtos_task_sleep_s(1);
      //  MCU_UART_WRITE("%s%d\r\n", CAL_VOLT, TempVal);

     ReplyToUser(SmsRevMob, SendSmsBuff,(int *)reserved);
}

//------------------------------REPLY to user --------------------//
void ReplyToUser(char *MobNumber, char *Text,void *mode)
{
   ql_sms_errcode_e SmsErrorCode=0;
   ql_rtc_time_t tm;
   char TimeArray[15];
   ql_rtc_set_timezone(22);    //UTC+32
   ql_rtc_get_localtime(&tm);
   ql_rtc_print_time(tm);

	if(mode == SMS_MODE)
    {
        if(strlen(Text)<125)
        {
        sprintf(TimeArray,"#%02d/%02d %02d:%02d",tm.tm_mday,tm.tm_mon,tm.tm_hour,tm.tm_min); 
        strcat(Text, TimeArray);
        strcat(Text, "\r\nRegards\r\nSamrudhi");
        }
        if((strlen((char *)MobNumber)>=10) && MobNumber[3]!=0Xff)
        {
	        SmsErrorCode = ql_sms_send_msg(ACTIVE_SIM,MobNumber,Text, GSM);
            QL_SMS_LOG("SMS:ERROR CODE %d  Mob:%s MSg=%s",SmsErrorCode,MobNumber,Text);
        }
        else
        {
            QL_SMS_LOG("SMS:ERROR CODE INCORRECT  Mob:%s MSg=%s",MobNumber,Text);
        }
    }

	if(SendSmsBuff)free(SendSmsBuff);
}

void ReplyToUser2(char *MobNumber, char *Text)
{
    u16 SmsErrorCode=0;
   ql_rtc_time_t tm;
   char TimeArray[15];
   ql_rtc_set_timezone(22);    //UTC+32
   ql_rtc_get_localtime(&tm);
   ql_rtc_print_time(tm);

	//if(mode == SMS_MODE)
    {
        sprintf(TimeArray,"#%02d/%02d %02d:%02d",tm.tm_mday,tm.tm_mon,tm.tm_hour,tm.tm_min); 
       strcat(Text, TimeArray);
        strcat(Text, "\r\nRegards\r\nSamrudhi");
	    SmsErrorCode = ql_sms_send_msg(ACTIVE_SIM,MobNumber,Text, GSM);
        QL_SMS_LOG("SMS:ERROR CODE %d",SmsErrorCode);
    }

	if(SendSmsBuff)free(SendSmsBuff);
}

//--------------------------------------------------------------------------------------------//
//--------GET Numeric value from string------------------------------------------------------//
uint16_t GetVal(char *Str, char ch)
{
    char *Ret = NULL;
    Ret = strchr(Str, ch);
    Ret++;
    Ret[strlen(Ret) - 1] = '\0';
    return (atoi(Ret));
}
//----------------------------------------------------------------------------------------------//

//----------------check Mob Number----------------//
uint8_t CheckMobNo(char* MobNo)
{
    
    uint8_t Index=0,RetValue=0;
     
   APP_DEBUG("\r\n----------Phone List-------------\r\n");
    for ( Index = 1; Index <= MAX_PH_SAVE; Index++)
    {
     APP_DEBUG("PH%d=%s\r\n",Index,SavePhNum.SaveNumbers[Index - 1].PhoneNumb);
    }
    #ifdef PHONE_NU_CHECK
	 for ( Index = 1; Index <= MAX_PH_SAVE; Index++)
    {
     if(strstr(MobNo,SavePhNum.SaveNumbers[Index - 1].PhoneNumb))
     {
        RetValue = Index;
        break;
     }
      
    }
    if(strstr(MobNo,COMPANY_NUMBER))
    {
         RetValue=COMPANY_NUMBER_INDEX;
    }
    return RetValue;
    #endif
    return COMPANY_NUMBER_INDEX;

}