#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_common.h"
#include "ql_api_osi.h"
#include "ql_api_voice_call.h"
#include "ql_log.h"
#include "ql_audio.h"
#include "ql_type.h"
#include "ql_api_ss.h"
#include "cfw.h"
#include "Defination.h"
#include "call.h"
#include "audio.h"
#include "uart.h"
#include "ussd_demo.h"
#include "sms.h"
ql_task_t call_task = NULL;
ql_event_t vc_event = {0};

extern ql_task_t audio_task;
extern ql_event_t audio_event;
extern struct Flag Flags;
extern struct Setting Settings;
extern struct Audi Audio;
extern  UINT8 AudioLanguage;

//----------------EC200U--->MCU CODES------------------------//
extern const char MOTOR_ON[];
extern const char MOTOR_OFF[];
extern const char MODE_AUTO[];
extern const char MODE_MAN[];
extern const char AUTO_CURR[];
extern const char CALL_CONN[];
extern const char CALL_CUT[];

extern char DBG_BUFFER[];
extern char MCU_BUFFER[];

extern const char GET_STATUS[];

//char CallRecMob[15];

//--------call URC call back----------------------//
void user_voice_call_event_callback(uint8_t sim, int event_id, void *ctx)
{
	
	switch(event_id)
	{
		case QL_VC_RING_IND:
			//ctx will be destroyed when exit.
			QL_VC_LOG("nSim=%d, RING TEL = %s",sim, (char *)ctx);
			
			//strcpy(CallRecMob,(char *)ctx);
			break;
		case QL_VC_CCWA_IND:
			//ctx will be destroyed when exit.
			QL_VC_LOG("nSim=%d, QL_VC_CCWA_IND : %s",sim, (char *)ctx);
			break;
		default:
			break;
	}
	vc_event.id = event_id;
	vc_event.param1 = sim;
	if(NULL != ctx)//The value of the ctx->msg may be NULL
	{
		vc_event.param2 = *(uint32 *)ctx;
	}
	
	ql_rtos_event_send(call_task, &vc_event);
	
}




//----------DTMF CALL BACK--------//
 void dtmf_callback(char tone)
{
	QL_VC_LOG("CALL:DTMF:enter%c",tone);
	
	DTMFKeyAction(tone);
}


void DTMFKeyAction(char Key)
{
    switch (Key)
    {
    case MOTOR_ONOFF_KEY:
     //   if((Settings.Mode==MANUAL_MODE)  && (Flags.VFault==NO_FAULT))//&& (Flags.CFault==NO_FAULT)
	   if((Flags.VFault==NO_FAULT))
        {
            Audio.Play = FALSE;
            if(Flags.MotorOn==FALSE)
            {
                MCU_UART_WRITE("%s", MOTOR_ON);
             //   APP_DEBUG("<<Audio STOP KEY PRESSED %d\r\n",RIL_AUD_StopPlayMemBg());
                PlayAudio(AudioLanguage,SELECTION_SET);
                MCU_UART_WRITE("%s", MOTOR_ON);
                Flags.HangUpcall = TRUE;
            }
            else
            {
                MCU_UART_WRITE("%s", MOTOR_OFF);
       
	           // APP_DEBUG("<<Audio STOP KEY PRESSED %d\r\n",RIL_AUD_StopPlayMemBg());
                PlayAudio(AudioLanguage,SELECTION_SET);
                MCU_UART_WRITE("%s", MOTOR_OFF);
                Flags.HangUpcall = TRUE;
            }
          //  Flags.PonStsSend = FALSE;
        }
        break;
    case MODE_SEL_KEY:
            Audio.Play = FALSE;
            
            if(Settings.Mode==AUTO_MODE)
 		    {
 			   MCU_UART_WRITE("%s", MODE_MAN);
             //  APP_DEBUG("<<Audio STOP KEY PRESSED %d\r\n",RIL_AUD_StopPlayMemBg());
              PlayAudio(AudioLanguage,SELECTION_SET); 
               MCU_UART_WRITE("%s", MODE_MAN);
 			   Settings.Mode = MANUAL_MODE;
               Flags.HangUpcall = TRUE;
 		    }
 			else
 			{
 			   MCU_UART_WRITE("%s", MODE_AUTO);
              // APP_DEBUG("<<Audio STOP KEY PRESSED %d\r\n",RIL_AUD_StopPlayMemBg());
               PlayAudio(AudioLanguage,SELECTION_SET);
               MCU_UART_WRITE("%s", MODE_AUTO);
 			   Settings.Mode = MANUAL_MODE;
               Flags.HangUpcall = TRUE;
 			}

        break;
    case AUTO_CURR_KEY:
            Audio.Play = FALSE;
            if(Flags.MotorOn==TRUE)
            {   MCU_UART_WRITE("%s", AUTO_CURR);
                PlayAudio(AudioLanguage,SELECTION_SET);
                MCU_UART_WRITE("%s", AUTO_CURR);
                Flags.HangUpcall = TRUE;
            }
            else
            {
                PlayAudio(AudioLanguage,SELECTION_NOT_SET);
                Flags.HangUpcall = TRUE;
            }

        break;    
		#ifdef TTS_EN
		  case '9':
            Audio.Play = FALSE;
            audio_event.id = ASW_AUDIO_TTS_VOLT;
			ql_rtos_event_send(audio_task, &audio_event);

        break; 

		  case '0':
            Audio.Play = FALSE;
            audio_event.id = ASW_AUDIO_TTS_BAL;
			ql_rtos_event_send(audio_task, &audio_event);

        break; 


    	#endif
    
    default:
        break;
    }
}


void voice_call_task(void * param)
{
	//ql_vc_errcode_e err = QL_VC_SUCCESS;
	ql_event_t vc_event = {0};
	uint8_t nSim = ACTIVE_SIM;
	ql_voice_call_callback_register(user_voice_call_event_callback);
	ql_aud_tone_detect(1);         //DTMF DETECT ENABLE
	ql_aud_tone_detect_set_cb(dtmf_callback);  //DTMF REC CALLBACK
	
    Flags.CallState = NO_CALL;
    QL_VC_LOG("Voice Call  Task");
    
    while(1)
    {
		ql_vc_errcode_e err = QL_VC_SUCCESS;
		uint8_t total;
	    ql_vc_info_s vc_info[QL_VC_MAX_NUM];
		ql_event_wait(&vc_event, QL_WAIT_FOREVER);
		switch(vc_event.id)
		{
			case QL_VC_RING_IND:
			    ql_rtos_task_sleep_s(2);
				ql_voice_call_clcc(nSim,&total, vc_info);
				//QL_VC_LOG(" MOB NUMBER= %s",CallRecMob);
			err = ql_voice_call_clcc(nSim,&total, vc_info);	
			if(err != QL_VC_SUCCESS){
				QL_VC_LOG("ql_voice_call_clcc FAIL");
			}else {
				QL_VC_LOG("total number = %d",total);
				for(uint8_t i = 0; i<total; i++){
					QL_VC_LOG("index:%d direction:%d status:%d mpty:%d number:%s",
							vc_info[i].idx,vc_info[i].direction,vc_info[i].status,vc_info[i].multiparty,vc_info[i].number);
				}
			}
				
				
				//if(Flags.CallState == NO_CALL)
				{
					QL_VC_LOG("Mobile Index %d",CheckMobNo(vc_info[total-1].number));
					//
					
					if(CheckMobNo(vc_info[total-1].number)!=0)
					{
						Flags.CallState = INCOMMING;
						err = ql_voice_call_answer(nSim);
						if(err != QL_VC_SUCCESS){
							QL_VC_LOG("ql_voice_call_answer FAIL");
						}else{
							QL_VC_LOG("ql_voice_call_answer OK");
						}
					}
					else
					{
						err= ql_voice_call_end(ACTIVE_SIM);
						if(err != QL_VC_SUCCESS)
						{
							QL_VC_LOG("ql_voice_call_end FAIL");
						}
					}
					
				}
				break;
			case QL_VC_CONNECT_IND:
				QL_VC_LOG("CONNECT");
                ql_voice_call_clcc(nSim,&total, vc_info);
				QL_VC_LOG("Total=%d Direction = %d MOB NUMBER; %s",total,vc_info[total-1].direction, vc_info[total-1].number);
				
				MCU_UART_WRITE("%s", GET_STATUS);ql_rtos_task_sleep_s(1);
    			MCU_UART_WRITE("%s", GET_STATUS);
				//ql_rtos_task_sleep_s(10);
				//test_wav();
				// err = ql_voice_call_end(nSim);
				// if(err != QL_VC_SUCCESS){
				// 	QL_VC_LOG("ql_voice_call_end FAIL");
				// }else{
				// 	QL_VC_LOG("ql_voice_call_end OK");}

				//	ql_voice_call_start(nSim,(char *)"8600687322");
				//	QL_VC_LOG("Dial Call OK");
				//}
               
				ql_rtos_task_sleep_s(1);
				audio_event.id = ASW_AUDIO_WELCOME;
				ql_rtos_event_send(audio_task, &audio_event);
				MCU_UART_WRITE("%s", CALL_CONN);

				break;
			case QL_VC_NOCARRIER_IND:
			    Audio.Play=FALSE;
        		Flags.HangUpcall=FALSE;
        		Audio.SeqCnt=0;
        		Audio.Repeat=0;
				Flags.CallState = NO_CALL;
				QL_VC_LOG("NOCARRIER");
				// MCU_UART_WRITE("%s", CALL_CUT);
				ql_rtos_task_sleep_s(1);
       			QL_VC_LOG("AUDIO:play stop %d",ql_aud_player_stop());
				MCU_UART_WRITE("%s", CALL_CUT);
				break;


			case ASW_CALL_HANG:
				
				QL_VC_LOG("AUDIO:play stop %d",ql_aud_player_stop());
				ql_rtos_task_sleep_s(2);
				
				err= ql_voice_call_end(ACTIVE_SIM);
				 if(err != QL_VC_SUCCESS)
				 {
				 	QL_VC_LOG("ql_voice_call_end FAIL");
				 }
				 else{
				 	QL_VC_LOG("ql_voice_call_end OK");
        		Audio.Play=FALSE;
        		Flags.HangUpcall=FALSE;
        		Audio.SeqCnt=0;
        		Audio.Repeat=0;
				Flags.CallState = NO_CALL;
        		QL_VC_LOG("Hang Up the call\r\n");}
				break;	
			default:
				QL_VC_LOG("event id = 0x%x",vc_event.id);
				break;	
		}

        
    }
}

QlOSStatus AutoSwitch_call_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	//CFW_SIM_PBK_ENTRY_INFO PbkEntery;
	err = ql_rtos_task_create(&call_task, 4096, APP_PRIORITY_NORMAL, "call_task", voice_call_task, NULL, 10);
	if(err != QL_OSI_SUCCESS)
	{
		QL_VC_LOG("voice_call_demo_task created failed, ret = 0x%x", err);
	}
	// //----------setting phonebook memory-------------//
	// err = CFW_CfgSetPbkStorage(CFW_PBK_ME,ACTIVE_SIM);
	// QL_VC_LOG("PH:Set Storage, ret = 0x%x", err);
    // //-----------store phonebook enetry---------------//
	// PbkEntery.pNumber =(uint8_t *)"8600687322";
	// PbkEntery.phoneIndex = 1; 
	// PbkEntery.pFullName =(uint8_t *)"Admin";
	// PbkEntery.nNumberSize=strlen((char *)PbkEntery.pNumber);
	// PbkEntery.iFullNameSize=strlen((char *)PbkEntery.pFullName);
	// PbkEntery.nType = 129;
	// err=CFW_SimAddPbkEntry(CFW_PBK_AUTO,&PbkEntery,1,ACTIVE_SIM);  
	// QL_VC_LOG("PH:Entery Write, ret = 0x%x", err);


	return err;
}