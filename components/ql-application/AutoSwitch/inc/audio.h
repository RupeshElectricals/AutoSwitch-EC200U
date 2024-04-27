#ifndef __AUDIO_H__
#define __AUDIO_H__

#define ID_RIFF 	0x46464952
#define ID_WAVE		0x45564157
#define ID_FMT  	0x20746d66
#define FORMAT_PCM 	1

#define CHECK_AUDIO_CORRECT     0
#define CHECK_AUDIO_INVALID_PARAMETER       1
#define CHECK_AUDIO_WAV_ERR     2
#define CHECK_AUDIO_MP3_ERR     3
#define CHECK_AUDIO_AMR_ERR     4


// #define WELCOME_AUDIO_FILE    "UFS:Welcome.amr"
// #define SPP_AUDIO_FILE        "UFS:SPP_MotorOff.amr"
// #define DRY_AUDIO_FILE        "UFS:DryRunMotorOff.amr"



#define DEFAULT_LOC   "UFS:"
#define MARATHI_LOC   "UFS:MarathiAudio/"
#define HINDI_LOC     "UFS:HindiAudio/"
#define ENGLISH_LOC   "UFS:EnglishAudio/"


#define WELCOME_FILE 		"Welcome.amr"

#define SPP_FILE        	"SPP_M_OFF.amr"
#define RP_M_OFF_FILE       "RP_M_OFF.amr"
#define D_R_FILE       		"DRY_RUN_M_OFF.amr"
#define OL_M_OFF_FILE       "OL_M_OFF.amr"

#define AUTO_MODE_M_ON_FILE "AUTO_MODE_M_ON.amr"
#define TURN_M_OFF_FILE     "TURN_M_OFF.amr"
#define TURN_M_ON_FILE      "TURN_M_ON.amr"

#define SET_MAN_FILE        "SET_MAN_MODE.amr"
#define SET_AUTO_FILE       "SET_AUTO_MODE.amr"

#define AUTO_CURR_FILE      "AUTO_CUR.amr"

#define SEL_SET_FILE        "SEL_SET.amr"
#define NO_SEL_SET_FILE     "NO_SEL_SET.amr"

#define P_ON_M_OFF_FILE     "POWER_ON_M_OFF.amr"
#define P_ON_M_ON_FILE      "POWER_ON_M_ON.amr"
#define P_OFF_M_OFF_FILE    "NO_POWER_M_OFF.amr"

#define AUTO_MODE_FILE      "AUTO_M.amr"
#define MANUAL_MODE_FILE    "MANUAL_M.amr"

#define UN_M_OFF_FILE       "UN_M_OFF.amr"

//#define TTS_EN				1


typedef enum
{
	DEF_LOC_AUDIO=0,
	MARATHI_AUDIO=1,
    HINDI_AUDIO=2,
	ENGLISH_ADUIO=3,
}AudioType;



typedef enum
{
	NO_AUDIO = 0,
    WELCOME_AUDIO = 1,
	AUTO_MODE_MOTOR_ON =2,

	TURN_OFF_MOTOR = 3,
	TURN_ON_MOTOR = 4,

    SEL_MANUAL_MODE=5,
	SEL_AUTO_MODE=6,
	SET_AUTO_CURR=7,

    SELECTION_SET=8,
	SELECTION_NOT_SET=9,

	SPP_FAULT_AUDIO=10,
    RP_FAULT_AUDIO=11,   
 
	DRYRUN_FAULT_AUDIO=12,
	OL_FAULT_AUDIO=13,

	POWER_ON_MOTOR_ON_AUDIO=14,
	POWER_ON_MOTOR_OFF_AUDIO=15,
	POWER_OFF_MOTOR_OFF_AUDIO=16,

	AUTO_MODE_AUDIO=17,
	MANUAL_MODE_AUDIO=18,

	UN_M_OFF_AUDIO=19,

}AUDIO;




struct Audi
{
    UINT8 Play;          //audio play enable
    UINT8 PlayList;      // audio number
    UINT8 Repeat;
    UINT8 SeqCnt;        // audio play seq cnt
};

typedef struct 
{
	char FileLoc[25];
	char FileName[50];
}AudioFileLoc;



void AutoSwitch_audio_app_init(void);
void audio_play_task(void *param);
void TTS_Init(void);
void PlayAudio(UINT8 Loc, UINT8 AudioF);
void CheckNextAudio(void);
void PlayTTS(char *str_eng);
void StopTTS(void);
UINT8 CheckBalance(void);
#endif //__AUDIO_H__