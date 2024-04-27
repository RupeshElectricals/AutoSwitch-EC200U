#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"

#include "ql_osi_def.h"
#include "ql_audio.h"
#include "ql_fs.h"
#include "ql_i2c.h"
#include "quec_pin_index.h"
#include "ql_gpio.h"
#include "ql_api_tts.h"
#include "osi_pipe.h"
#include "audio_resample.h"
#include "ql_api_voice_call.h"
#include "ql_api_ss.h"
#include "ussd_demo.h"
#include "Defination.h"
#include "audio.h"
#include "sms.h"
#include "uart.h"
#include "ussd_demo.h"

#define QL_AUDIO_LOG_LEVEL	            QL_LOG_LEVEL_INFO
#define QL_AUDIO_LOG(msg, ...)			QL_LOG(QL_AUDIO_LOG_LEVEL, "ql_audio", msg, ##__VA_ARGS__)
#define QL_AUDIO_LOG_PUSH(msg, ...)		QL_LOG_PUSH("ql_AUDIO", msg, ##__VA_ARGS__)
#define QUEC_16K_FRAME_LEN 640 
#define QUEC_8K_FRAME_LEN  320
#if !defined(require_action)
	#define require_action(x, action, str)																		\
			do                                                                                                  \
			{                                                                                                   \
				if(x != 0)                                                                        				\
				{                                                                                               \
					QL_AUDIO_LOG(str);																			\
					{action;}																					\
				}                                                                                               \
			} while( 1==0 )
#endif
#define OSI_MIN(type, a, b) ({ type _a = (type)(a); type _b = (type)(b); _a < _b? _a : _b; })
typedef struct osi_buff
{
    unsigned size;
    unsigned rd;
    unsigned wr;
    char data[];
}osi_buff_t;

typedef struct
{
	char  	   	   	*input_buffer;
	char  	   	   	*output_buffer;
	osi_buff_t  	   	*pipe;	
	ql_resampler      	*filter;
	uint8_t 		tts_type;		//0 for tts, 1 for wtts
	PCM_HANDLE_T 	pcm;
}tts_loacl_info_t;

ql_task_t audio_task = NULL;
ql_event_t audio_event = {0};
PCM_HANDLE_T tts_player = NULL;
struct Audi Audio;
extern struct Setting Settings;
extern struct Flag Flags;

//-----CALL EVENT------//
extern ql_task_t call_task;
extern ql_event_t vc_event; 

UINT8 AudioLanguage=MARATHI_AUDIO;


extern UINT8 OpNameBuff[];

const AudioFileLoc  AudioFile[] ={
{MARATHI_LOC,""},
{MARATHI_LOC,WELCOME_FILE},
{HINDI_LOC,AUTO_MODE_M_ON_FILE},

{ENGLISH_LOC,TURN_M_OFF_FILE},
{ENGLISH_LOC,TURN_M_ON_FILE},
{ENGLISH_LOC,SET_MAN_FILE},
{ENGLISH_LOC,SET_AUTO_FILE},
{ENGLISH_LOC,AUTO_CURR_FILE},

{ENGLISH_LOC,SEL_SET_FILE},
{ENGLISH_LOC,NO_SEL_SET_FILE},

{ENGLISH_LOC,SPP_FILE},
{ENGLISH_LOC,RP_M_OFF_FILE},
{ENGLISH_LOC,D_R_FILE},
{ENGLISH_LOC,OL_M_OFF_FILE},


{ENGLISH_LOC,P_ON_M_ON_FILE},
{ENGLISH_LOC,P_ON_M_OFF_FILE},
{ENGLISH_LOC,P_OFF_M_OFF_FILE},

{ENGLISH_LOC,AUTO_MODE_FILE},
{ENGLISH_LOC,MANUAL_MODE_FILE},

{ENGLISH_LOC,UN_M_OFF_FILE},

};

//--------------USSD------------------------//
uint8_t ql_ussd_buffer[QUEC_SS_USSD_UCS2_SIZE_MAX] = {0};
ql_sem_t ql_ussd_sem = NULL;

static tts_loacl_info_t tts_loacl_info = {0};

void TTS_Init(void);

osi_buff_t *osibuffCreate(unsigned size)
{
    if (size == 0)
        return NULL;

    osi_buff_t *pipe = (osi_buff_t *)calloc(1, sizeof(osi_buff_t) + size);
    if (pipe == NULL)
        return NULL;

    pipe->size = size;
	pipe->rd = 0;
	pipe->wr = 0;
    return pipe;
}

int osibuffWrite(osi_buff_t *pipe, const void *buf, unsigned size)
{
    if (size == 0)
        return 0;
    if (pipe == NULL || buf == NULL)
        return -1;

    uint32_t critical = osiEnterCritical();
    unsigned space = pipe->size - (pipe->wr - pipe->rd);
    unsigned len = OSI_MIN(unsigned, size, space);
    unsigned wr = pipe->wr;

    if (len == 0)
    {
        osiExitCritical(critical);
        return 0;
    }

    unsigned offset = wr % pipe->size;
    unsigned tail = pipe->size - offset;
    if (tail >= len)
    {
        memcpy(&pipe->data[offset], buf, len);
    }
    else
    {
        memcpy(&pipe->data[offset], buf, tail);
        memcpy(pipe->data, (const char *)buf + tail, len - tail);
    }

    pipe->wr += len;
    osiExitCritical(critical);

    return len;
}

int osibuffWriteAll(osi_buff_t *pipe, const void *buf, unsigned size, unsigned timeout)
{
    if (size == 0)
        return 0;
    if (pipe == NULL || buf == NULL)
        return -1;

    unsigned len = 0;
    for (;;)
    {
        int bytes = osibuffWrite(pipe, buf, size);
        if (bytes < 0)
            return -1;

        len += bytes;
        size -= bytes;
        buf = (const char *)buf + bytes;
        if (size == 0 )
            break;
    }
    return len;
}

int osibuffReadAvail(osi_buff_t *pipe)
{
    if (pipe == NULL)
        return -1;

    uint32_t critical = osiEnterCritical();
    unsigned bytes = pipe->wr - pipe->rd;
    osiExitCritical(critical);
    return bytes;
}

int osibuffRead(osi_buff_t *pipe, void *buf, unsigned size)
{
    if (size == 0)
        return 0;
    if (pipe == NULL || buf == NULL)
        return -1;

    uint32_t critical = osiEnterCritical();
    unsigned bytes = pipe->wr - pipe->rd;
    unsigned len = OSI_MIN(unsigned, size, bytes);
    unsigned rd = pipe->rd;

    if (len == 0)
    {
        osiExitCritical(critical);
        return 0;
    }

    unsigned offset = rd % pipe->size;
    unsigned tail = pipe->size - offset;
    if (tail >= len)
    {
        memcpy(buf, &pipe->data[offset], len);
    }
    else
    {
        memcpy(buf, &pipe->data[offset], tail);
        memcpy((char *)buf + tail, pipe->data, len - tail);
    }

    pipe->rd += len;
    osiExitCritical(critical);

    return len;
}

int osibuffReadAll(osi_buff_t *pipe, void *buf, unsigned size, unsigned timeout)
{
    if (size == 0)
        return 0;
    if (pipe == NULL || buf == NULL)
        return -1;

    unsigned len = 0;
    for (;;)
    {
        int bytes = osibuffRead(pipe, buf, size);
        if (bytes < 0)
            return -1;

        len += bytes;
        size -= bytes;
        buf = (char *)buf + bytes;
        if (size == 0 )
            break;
    }

    return len;
}

int TTSuserCallback(void *param, int param1, int param2, int param3, int data_len, const void *pcm_data)
{
	int err,cnt = 0;
	tts_loacl_info_t *tts = &tts_loacl_info;
	int a=0;
	if(a)
	{
		err = ql_pcm_write(tts_player, (void *)pcm_data, data_len);
		if(err <= 0)
		{
			QL_AUDIO_LOG("AUDIO:TTS write data to PCM player failed");
			return -1;
		}
	}
	else
	{
		err = osibuffWriteAll(tts->pipe, pcm_data, data_len, QL_WAIT_FOREVER); //é™é‡‡æ ·éœ€è¦æ¯æ¬¡è¾“å…¥ä¸€åŒ…å®Œæ•´çš„PCMå¸§640å­—èŠ‚,å› æ­¤å…ˆç¼“å­˜
		require_action((err<0), goto exit, "cache data fail");
		require_action(!((uint)tts->input_buffer & (uint)tts->output_buffer), goto exit, "invalid buffer");
		
        while(1)
        {
			cnt = osibuffReadAvail(tts->pipe);
			require_action((err<0), goto exit, "read avil fail");

			if(cnt >= QUEC_16K_FRAME_LEN)	//é™é‡‡æ ·éœ€è¦æ¯æ¬¡è¾“å…¥ä¸€åŒ…å®Œæ•´çš„PCMå¸§640å­—èŠ‚,å› æ­¤å…ˆç¼“å­˜,å†æ¯æ¬¡è¯»å–640å­—èŠ‚ 
			{
				cnt = osibuffReadAll(tts->pipe, (void *)tts->input_buffer, QUEC_16K_FRAME_LEN, QL_WAIT_FOREVER);
				require_action((cnt!=QUEC_16K_FRAME_LEN), goto exit, "pipe read fail");
				
				ql_aud_resampler_run(tts->filter, (short *)tts->input_buffer, (short *)tts->output_buffer); //å¼€å§‹é™é‡‡æ ·		
				err = ql_pcm_write(tts_player, (void *)tts->output_buffer, QUEC_8K_FRAME_LEN);
				require_action((err!=QUEC_8K_FRAME_LEN), goto exit, "pcm write fail");
			}
			else
			{
				break;
			}
        }	
	}

    return 0;
exit:
	QL_AUDIO_LOG("tts callback failed");
	return -1;
}
//----------------------AUDIO CALLBACK--------------------------------//
static int Audio_play_callback(char *p_data, int len, enum_aud_player_state state)
{
	if(state == AUD_PLAYER_START)
	{
		QL_AUDIO_LOG("AUDIO:player start run");
	}
	else if(state == AUD_PLAYER_FINISHED)
	{
		QL_AUDIO_LOG("AUDIO:player stop run");

       if(Flags.HangUpcall==TRUE)
		{
            vc_event.id = ASW_CALL_HANG;
			ql_rtos_event_send(call_task, &vc_event);

			//ql_voice_call_end(ACTIVE_SIM);
			
		}
        else
			CheckNextAudio();  
	}
	else
	{
		QL_AUDIO_LOG("AUDIO:type is %d", state);
	}

	return QL_AUDIO_SUCCESS;
}

//-----------USSD CALLBACK-----------------//
void ql_ss_ussd_ind_cb(uint8_t sim_id,unsigned int ind_type,void *ind_msg_buf)
{
	if(QUEC_SS_USSD_IND == ind_type)
    {
        ql_ss_ussd_str_s *ussd_str = (ql_ss_ussd_str_s *)ind_msg_buf;
        QL_AUDIO_LOG("ussd resp:%d",ussd_str->resp_type);
        switch(ussd_str->resp_type)
        {
            case QL_SS_USSD_RESP_SUCCESS:
            {
                QL_AUDIO_LOG("ussd dcs:%d,len:%d",ussd_str->dcs,ussd_str->len);
                if(0 < ussd_str->len)
                {
                    memcpy((void*)ql_ussd_buffer,(void*)ussd_str->str,ussd_str->len);
                    //Display
                    QL_AUDIO_LOG("ussd code:%s",ql_ussd_buffer);

                    bool state = false;
                    if(QL_SS_SUCCESS != ql_ss_ussd_get_session_state(sim_id,&state))
                    {
                        QL_AUDIO_LOG("ussd get state fail");
                    }
                    if(state)
                    {
                        QL_AUDIO_LOG("ussd session continue");
                        //User cancel current session.And str should be NULL when cancel session.
                    //     ql_ss_ussd_send_s ctx = {
                    //         .option = QL_SS_USSD_SESSION_CANCEL,
                    //         .str = NULL,
                    //     };
                    //    UINT8 ret = ql_ss_ussd_send(sim_id,&ctx);
                    //     QL_USSD_DEMO_LOG("ussd session close =%d",ret);
                    }
                    else
                    {
                        QL_AUDIO_LOG("ussd session end");
                    }

					 ql_rtos_semaphore_release(ql_ussd_sem);
                }
                break;
            }
            case QL_SS_USSD_CANCEL_SUCCESS:
            {
                QL_AUDIO_LOG("user cancle success");
                break;
            }
            case QL_SS_USSD_OTHER_ERR:
            {
                QL_AUDIO_LOG("ussd error code:%ld",ussd_str->err_code);
                break;
            }
            case QL_SS_USSD_SAT_SUCCESS:
            case QL_SS_USSD_NO_STR_ERR:
            default:break;
        }
    }
    return;
}



void audio_play_task(void *param)
{
	
    //QL_PCM_CONFIG_T config = {1, 16000, 0};
	QL_AUDIO_LOG("AUDIO:enter audio demo");
	
	TTS_Init();
	ql_ss_register_cb(ql_ss_ussd_ind_cb); // USSD CALLBACK

    while (1)
    {
        
        ql_event_wait(&audio_event, QL_WAIT_FOREVER);
        switch (audio_event.id)
		{
		case ASW_AUDIO_WELCOME:
		     if(Flags.CallState == OUTGOING)
                	PlayAudio(AudioLanguage,Audio.PlayList);
			else		
				PlayAudio(AudioLanguage,WELCOME_AUDIO);
			break;

		case ASW_AUDIO_TTS_VOLT:
		    //StopTTS();
			PlayTTS("RY VOLT 440 YB VOLT 410 BR VOLT 420 CURRENT 10.5");
			//if(Audio.SeqCnt!=0)
			//	Audio.SeqCnt--;
			CheckNextAudio();  
			break;	

		case ASW_AUDIO_TTS_BAL:
		  
		//    if( ql_rtos_semaphore_create(&ql_ussd_sem, 0) == QL_OSI_SUCCESS)
		//    {
		//     ql_ss_ussd_send_s ctx = {
        // 		.option = QL_SS_USSD_SESSION_INITIATE,
        // 		.str = "*123*1#",
   		// 	 };
   		// 	ql_ss_ussd_send(ACTIVE_SIM,&ctx);
		// 	ql_rtos_semaphore_wait(ql_ussd_sem, 0x1388);
		// 	PlayTTS((char *) ql_ussd_buffer);
		// 	//if(Audio.SeqCnt!=0)
		// 	//	Audio.SeqCnt--;
		// 	CheckNextAudio(); 
		//    } 

		if(CheckBalance()==1)
		{
			PlayTTS((char *) ql_ussd_buffer);
			//if(Audio.SeqCnt!=0)
			//	Audio.SeqCnt--;
			CheckNextAudio(); 
			QL_AUDIO_LOG("bal:%s",ql_ussd_buffer);
		}
		else
		{
			QL_AUDIO_LOG("bal:ERROR");
		}
		
		break;	
			
		
		default:
			break;
		}
        
		// AudioError = ql_aud_player_stop();
        // QL_AUDIO_LOG("AUDIO:play stop %d",AudioError);
        // ql_rtos_task_sleep_ms(1000);


    //    AudioError = ql_file_exist("UFS:MarathiAudio/Welcome.amr");
    //    QL_AUDIO_LOG("AUDIO:FILE EXIST %d",AudioError);
    //    AudioError = ql_aud_play_file_start("UFS:MarathiAudio/Welcome.amr", QL_AUDIO_PLAY_TYPE_VOICE, Audio_play_callback);
	//    ql_aud_wait_play_finish(QL_WAIT_FOREVER);
		
		// ql_rtos_task_sleep_ms(2000);
		// ql_voice_call_end(ACTIVE_SIM);
		// ql_rtos_task_sleep_ms(2000);	

     //  PlayAudio(AudioLanguage,WELCOME_AUDIO);
	//    ql_aud_wait_play_finish(QL_WAIT_FOREVER);
	//    PlayTTS("RY VOLT 440 YB VOLT 410 BR VOLT 420 CURRENT 10.5");
	//    PlayAudio(AudioLanguage,SELECTION_SET);
    //    ql_aud_wait_play_finish(QL_WAIT_FOREVER);
	//    PlayTTS("to select english press 9 for marathi 2 for hindi 3");
/*
       ql_event_wait(&audio_event, QL_WAIT_FOREVER);
        ql_rtos_task_sleep_ms(1000);
		TTS_Init();
		tts_player = ql_aud_pcm_open(&config, QL_AUDIO_FORMAT_PCM, QL_PCM_BLOCK_FLAG|QL_PCM_WRITE_FLAG, QL_PCM_VOICE_CALL);
       // ql_tts_set_config_param(QL_TTS_CONFIG_ENCODING,QL_TTS_UTF8);
		//ql_tts_set_config_param(QL_TTS_CONFIG_DGAIN, 0);
		QL_AUDIO_LOG("AUDIO:TTS CONFIG:%d",tts_player);
		char *str_eng = "RY VOLT 440 YB VOLT 410 BR VOLT 420 CURRENT 10.5";
		AudioError = ql_tts_start((const char *)str_eng, strlen(str_eng));
        QL_AUDIO_LOG("AUDIO:TTS PLAY %d",AudioError);
        ql_aud_data_done();
		ql_aud_wait_play_finish(QL_WAIT_FOREVER);
		ql_tts_end();  
         
        if(tts_player){
			ql_pcm_close(tts_player);
			tts_player = NULL;}

        AudioError = ql_aud_player_stop();
        QL_AUDIO_LOG("AUDIO:play stop %d",AudioError);
        ql_rtos_task_sleep_ms(1000);
        AudioError = ql_file_exist("UFS:MarathiAudio/Welcome.amr");
        QL_AUDIO_LOG("AUDIO:FILE EXIST %d",AudioError);
        AudioError = ql_aud_play_file_start("UFS:MarathiAudio/Welcome.amr", QL_AUDIO_PLAY_TYPE_VOICE, Audio_play_callback);
	    QL_AUDIO_LOG("AUDIO:play code%d",AudioError);
	    ql_aud_wait_play_finish(QL_WAIT_FOREVER);
        AudioError = ql_aud_player_stop();
        QL_AUDIO_LOG("AUDIO:play stop %d",AudioError);
		ql_rtos_task_sleep_ms(3000);


   	ql_rtos_task_sleep_ms(2000);
		ql_voice_call_end(ACTIVE_SIM);
		ql_rtos_task_sleep_ms(2000);	*/





    }
    
}


void CheckNextAudio(void)
{
	Audio.SeqCnt++;
    Repeat:
    QL_AUDIO_LOG("\r\nFlags.CallState=%d,Settings.Mode=%d,Audio.SeqCnt=%d,Audio.Repeat=%d\r\n",Flags.CallState,Settings.Mode,Audio.SeqCnt,Audio.Repeat);
	//if(Flags.CallState==INCOMMING)
/*	{
		switch (Audio.SeqCnt)
		{
		case 1:
		if(Flags.CFault==NO_FAULT && Flags.VFault==NO_FAULT)
		{
			if(Settings.Mode==MANUAL_MODE)
			{
				if(Flags.MotorOn==TRUE)
				 Audio.PlayList=TURN_OFF_MOTOR;
				else
				  Audio.PlayList=TURN_ON_MOTOR;
			}
			else
			{
			  Audio.PlayList=AUTO_MODE_MOTOR_ON;

			}
		}
		else{
			if(Flags.CFault==DR_FAULT)
			 Audio.PlayList=DRYRUN_FAULT_AUDIO;
			else if (Flags.CFault==OL_FAULT)
			 Audio.PlayList=OL_FAULT_AUDIO;
			else{
			  Audio.PlayList = SPP_FAULT_AUDIO;}
			  
			  
		}
		Audio.Play = TRUE;
			break;
		case 2:

			if((Audio.Repeat>2) && (Flags.MotorOn==FALSE))
				 Flags.HangUpcall = TRUE;

			if((Flags.CFault!=NO_FAULT) && (Settings.Mode==MANUAL_MODE) && (Audio.PlayList!=TURN_ON_MOTOR))
			{
				Audio.PlayList=TURN_ON_MOTOR;
				Audio.SeqCnt--;
				Flags.HangUpcall = FALSE;
			}

			else if(Settings.Mode==AUTO_MODE)
				Audio.PlayList = SEL_MANUAL_MODE;
			else
				Audio.PlayList = SEL_AUTO_MODE;

		

				 Audio.Play = TRUE;

			break;
		case 3:
			if(Flags.MotorOn==TRUE)
			{
				Audio.PlayList = SET_AUTO_CURR;
				Audio.SeqCnt=0;
				Audio.Play = TRUE;
				Audio.Repeat++;
				if(Audio.Repeat>2)
				 Flags.HangUpcall = TRUE;
			}
			else
			{
				Audio.Repeat++;
				Audio.SeqCnt=1;
			
				goto Repeat;
			}
			break;
		
		default:
			break;
		}
	}*/

    {
		switch (Audio.SeqCnt)
		{

		case 1:
			if(Settings.Mode==MANUAL_MODE)
			{
				 Audio.PlayList=MANUAL_MODE_AUDIO;
			}
			else
			{
				 Audio.PlayList=AUTO_MODE_AUDIO;
			}

		break;	
		case 2:
		if(Flags.CFault==NO_FAULT && Flags.VFault==NO_FAULT)
		{

				if(Flags.MotorOn==TRUE)
				 Audio.PlayList=TURN_OFF_MOTOR;
				else
				  Audio.PlayList=TURN_ON_MOTOR;
		}
		else{
			if(Flags.CFault==DR_FAULT)
			 Audio.PlayList=DRYRUN_FAULT_AUDIO;
			else if (Flags.CFault==OL_FAULT)
			 Audio.PlayList=OL_FAULT_AUDIO;
			else if (Flags.CFault==UN_FAULT) 
			 Audio.PlayList=UN_M_OFF_AUDIO;
			else{
			   switch (Flags.VFault)
				{
				case LV_FAULT:
					Audio.PlayList = SPP_FAULT_AUDIO;
					break;
				case HV_FAULT:
					Audio.PlayList = SPP_FAULT_AUDIO;
					break;
				case PSL_FAULT:
					Audio.PlayList = SPP_FAULT_AUDIO;
					break;
				case SPP_FAULT:
					Audio.PlayList = SPP_FAULT_AUDIO;
					break;
				case PHASE_REV_FAULT:
					Audio.PlayList = RP_FAULT_AUDIO;
					break;
				case PWR_OFF_FAULT:
					Audio.PlayList = POWER_OFF_MOTOR_OFF_AUDIO;
					break; 
				case PRE_PWR_OFF_FAULT:
					Audio.PlayList = UN_M_OFF_AUDIO;
					break; 	  
					 
				}
			}
		}
		Audio.Play = TRUE;
			break;

		case 3:

			if((Audio.Repeat>2) && (Flags.MotorOn==FALSE))
				 Flags.HangUpcall = TRUE;

			if((Flags.CFault!=NO_FAULT) && (Settings.Mode==MANUAL_MODE) && (Audio.PlayList!=TURN_ON_MOTOR))
			{
				Audio.PlayList=TURN_ON_MOTOR;
				Audio.SeqCnt--;
				Flags.HangUpcall = FALSE;
			}

			else if(Settings.Mode==AUTO_MODE)
				Audio.PlayList = SEL_MANUAL_MODE;
			else
				Audio.PlayList = SEL_AUTO_MODE;

		

				 Audio.Play = TRUE;

			break;
		case 4:
			if(Flags.MotorOn==TRUE)
			{
				Audio.PlayList = SET_AUTO_CURR;
				Audio.SeqCnt=0;
				Audio.Play = TRUE;
				Audio.Repeat++;
				if(Audio.Repeat>2)
				 Flags.HangUpcall = TRUE;
			}
			else
			{
				Audio.Repeat++;
				Audio.SeqCnt=1;
			
				goto Repeat;
			}
			break;
		
		default:
			break;
		}
	}


    PlayAudio(AudioLanguage,Audio.PlayList);
	if(Audio.SeqCnt>=4)
	Audio.SeqCnt=0;
}



//-----------Audio play list------------------//
void PlayAudio(UINT8 Loc, UINT8 AudioF)
{
	ql_audio_errcode_e AudioError=0;
	char FileNameStr[50];
	PlayAgain:
	StopTTS();
	AudioError = ql_aud_player_stop();
    QL_AUDIO_LOG("AUDIO:play stop %d",AudioError);
	ql_rtos_task_sleep_ms(200);
    
	memset(FileNameStr,0x00,50);
	strcpy(FileNameStr,AudioFile[Loc].FileLoc);
	strcat(FileNameStr,AudioFile[AudioF].FileName);
	QL_AUDIO_LOG("AUDIO:FileNameStr %s",FileNameStr);
   
    
	AudioError = ql_aud_play_file_start(FileNameStr, QL_AUDIO_PLAY_TYPE_VOICE, Audio_play_callback);
	QL_AUDIO_LOG("AUDIO:play code%x",AudioError);
	if(AudioError!=0)
	{
		goto PlayAgain;
	}
	  
}

void TTS_Init(void)
{
	int err = 0;
	tts_loacl_info_t *tts = &tts_loacl_info;
	tts_param_t tts_param = {0};

	tts->filter = (ql_resampler *)calloc(1, sizeof(ql_resampler));
	if(!(tts->filter))
	{
		QL_AUDIO_LOG("no mem");
		goto exit;	
	}	
	ql_aud_resampler_create(16000, 8000, 16000*20/1000, tts->filter);

	tts->tts_type = 1;
	tts->pipe = osibuffCreate(4*1024);
	QL_AUDIO_LOG("osibuffCreate tts-:%x",tts);
	QL_AUDIO_LOG("osibuffCreate tts->pipe:%x",tts->pipe);
	require_action(!tts->pipe, goto exit, "no mem");

	tts->input_buffer = (char *)calloc(1, 640);
	require_action(!tts->input_buffer, goto exit, "no mem");
	
	tts->output_buffer = (char *)calloc(1, 640);
	require_action(!tts->output_buffer, goto exit, "no mem");

    tts_param.resource = TTS_RESOURCE_16K_EN;
	//tts_param.resource = TTS_RESOURCE_16K_CN;
	tts_param.position = POSIT_INTERNAL_FS; 
	
    err = ql_tts_engine_init_ex(TTSuserCallback, &tts_param);
	QL_AUDIO_LOG("AUDIO:TTS INIT:%d",err);
	return;

exit:
	if(tts->filter)
	{
		ql_aud_resampler_destroy(tts->filter);
		free(tts->filter);
		tts->filter = NULL;
	}
	if(tts->pipe)
	{
		free(tts->pipe);
		tts->pipe = NULL;
	}
	if(tts->input_buffer)
	{
		free(tts->input_buffer);
		tts->input_buffer = NULL;
	}
	if(tts->output_buffer){
		free(tts->output_buffer);
		tts->output_buffer = NULL;
	}
}


void AutoSwitch_audio_app_init(void)
{

	QlOSStatus err = QL_OSI_SUCCESS;
	
	
    QL_AUDIO_LOG("audio demo enter");
	
	err = ql_rtos_task_create(&audio_task, 4096, APP_PRIORITY_NORMAL, "audio_task", audio_play_task, NULL, 2);
	if(err != QL_OSI_SUCCESS)
    {
		QL_AUDIO_LOG("audio task create failed");
	}
	QL_AUDIO_LOG("this is git test");
}

void PlayTTS(char *str_eng)
{
	    int AudioError=0;
	    QL_PCM_CONFIG_T config = {1, 8000, 0};
		QL_AUDIO_LOG("AUDIO:play stop %d",ql_aud_player_stop());
	    TTS_Init();
	    ql_rtos_task_sleep_ms(200);
		
		tts_player = ql_aud_pcm_open(&config, QL_AUDIO_FORMAT_PCM, QL_PCM_BLOCK_FLAG|QL_PCM_WRITE_FLAG, QL_PCM_VOICE_CALL);
        //tts_player = ql_pcm_open(&config, QL_PCM_BLOCK_FLAG|QL_PCM_WRITE_FLAG);
      
        ql_tts_set_config_param(QL_TTS_CONFIG_ENCODING,QL_TTS_UTF8);
		ql_tts_set_config_param(QL_TTS_CONFIG_SPEED, 1);
		//char *str_eng = "RY VOLT 440 YB VOLT 410 BR VOLT 420 CURRENT 10.5";
		//char *str_eng = "CURRENT VOLTAGE OF RY 440 VOLT";
		AudioError = ql_tts_start((const char *)str_eng, strlen(str_eng));
        QL_AUDIO_LOG("AUDIO:TTS PLAY %d",AudioError);
		if (AudioError!=0)
		{
			AudioError = ql_tts_start((const char *)str_eng, strlen(str_eng));
            QL_AUDIO_LOG("AUDIO:TTS next PLAY %d",AudioError);
		}
		
        ql_aud_data_done();
		ql_aud_wait_play_finish(QL_WAIT_FOREVER);
		ql_tts_end();  
         
        if(tts_player){
			AudioError=ql_pcm_close(tts_player);
			 QL_AUDIO_LOG("AUDIO:ql_pcm_close= %d",AudioError);
			tts_player = NULL;
        }
}

void StopTTS(void)
{
	if(tts_player){
			ql_tts_end();
			 QL_AUDIO_LOG("AUDIO:ql_pcm_close= %d",ql_pcm_close(tts_player));
			tts_player = NULL;
        }
}

UINT8 CheckBalance(void)
{
	if( ql_rtos_semaphore_create(&ql_ussd_sem, 0) == QL_OSI_SUCCESS)
	{
	ql_ss_ussd_send_s ctx = {
		.option = QL_SS_USSD_SESSION_INITIATE,
		.str = "*123*1#",
		};

	if(strstr((char *)OpNameBuff,"AIRTEL"))
	ctx.str = "*123*1#";

	if(strstr((char *)OpNameBuff,"VI") || strstr((char *)OpNameBuff,"VODA") || strstr((char *)OpNameBuff,"IDEA"))
	ctx.str = "*199*2*1#";

	if(strstr((char *)OpNameBuff,"JIO"))
	{
		ql_voice_call_start(ACTIVE_SIM,"1299");
		Flags.BalSmsReply=1;
		return 0;
	}

    
    
	QL_AUDIO_LOG("checking for bal %s",ctx.str);
	ql_ss_ussd_send(ACTIVE_SIM,&ctx);
	if(ql_rtos_semaphore_wait(ql_ussd_sem, 0x1388)==QL_OSI_SUCCESS)
		return 1;
	else
		return 0;
	}
	return 0;
}