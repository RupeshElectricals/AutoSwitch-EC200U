#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_pin_cfg.h"
#include "ql_power.h"
#include "Defination.h"
#include "sms.h"
#include "uart.h"

#define QL_UART_DEMO_LOG_LEVEL			QL_LOG_LEVEL_INFO
#define QL_UART_DEMO_LOG(msg, ...)		QL_LOG(QL_UART_DEMO_LOG_LEVEL, "ql_uart_demo", msg, ##__VA_ARGS__)

#define MIN(a,b) ((a) < (b) ? (a) : (b))

char DBG_BUFFER[DBG_BUF_LEN];
char MCU_BUFFER[MCU_BUF_LEN];



ql_queue_t Uart_queue_01 = NULL;

#define QUEUE_01_LEN	QL_UART_RX_BUFF_SIZE
unsigned char recv_buff_queue[ QUEUE_01_LEN+1]={0};
//ql_task_t  Uart_task=NULL;


//-----------MCU----->EC200U CODES-----------------------//
const char SETTINGS[]="SETTING:";
const char STATUS[]="STATUS:";
const char PON_SMS[]="PON_SMS";
const char RESET_GSM[]="RESET_GSM";
extern const char GET_STATUS[];

//-----CALL EVENT------//
extern ql_task_t call_task;
extern ql_event_t vc_event; 
extern ql_task_t ASW_power_task;

extern int RY,YB,BR,Current;
extern struct Flag Flags;
extern struct Setting Settings;
void ql_uart_notify_cb(unsigned int ind_type, ql_uart_port_number_e port, unsigned int size)
{
 //   ql_event_t Uart_event = {0};
    unsigned char *recv_buff = calloc(1, QL_UART_RX_BUFF_SIZE+1);
    unsigned int real_size = 0;
    int read_len = 0;
    QL_UART_DEMO_LOG("UART: port %d receive ind type:0x%x, receive data size:%d", port, ind_type, size);
    switch(ind_type)
    {
        case QUEC_UART_RX_OVERFLOW_IND:  //rx buffer overflow
        case QUEC_UART_RX_RECV_DATA_IND:
        {
            while(size > 0)
            {
                memset(recv_buff, 0, QL_UART_RX_BUFF_SIZE+1);
                real_size= MIN(size, QL_UART_RX_BUFF_SIZE);
                
                read_len = ql_uart_read(port, recv_buff, real_size);
                QL_UART_DEMO_LOG("read_len=%d, recv_data=%s", read_len, recv_buff);
                ql_rtos_queue_release(Uart_queue_01, strlen((char *)recv_buff), recv_buff, QL_NO_WAIT);
                if((read_len > 0) && (size >= read_len))
                {
                    size -= read_len;
                }
                else
                {
                    break;
                }

            }
            break;
        }
        case QUEC_UART_TX_FIFO_COMPLETE_IND: 
        {
            QL_UART_DEMO_LOG("tx fifo complete");
            break;
        }
    }
    free(recv_buff);
    recv_buff = NULL;
}

static void uart_demo_task(void *param)
{
    int ret = 0;
	QlOSStatus err = 0;
    ql_uart_config_s uart_cfg = {0};
   // int write_len = 0;
    //ql_uart_tx_status_e tx_status;

    //---------UART CONFIGURATION---------------//
    uart_cfg.baudrate = QL_UART_BAUD_115200;
    uart_cfg.flow_ctrl = QL_FC_NONE;
    uart_cfg.data_bit = QL_UART_DATABIT_8;
    uart_cfg.stop_bit = QL_UART_STOP_1;
    uart_cfg.parity_bit = QL_UART_PARITY_NONE;

    ret = ql_uart_set_dcbconfig(MCU_UART, &uart_cfg);
    QL_UART_DEMO_LOG("UART:MCU UART CONFIG ret: 0x%x", ret);
    ret = ql_uart_set_dcbconfig(DEBUG_UART, &uart_cfg);
    QL_UART_DEMO_LOG("UART:DEBUG UART CONFIG ret: 0x%x", ret);
	
    ret = ql_pin_set_func(QL_UART2_TX_PIN, QL_UART2_TX_FUNC);
    QL_UART_DEMO_LOG("UART:DEBUG UART TX PIN CONFIG ret: 0x%x", ret);
	ret = ql_pin_set_func(QL_UART2_RX_PIN, QL_UART2_RX_FUNC);
    QL_UART_DEMO_LOG("UART:DEBUG UART RX PIN CONFIG ret: 0x%x", ret);

    ret = ql_uart_open(MCU_UART);
    QL_UART_DEMO_LOG("UART:MCU UART OPEN ret: 0x%x", ret);

    if(QL_UART_SUCCESS == ret)
	{
     ret = ql_uart_register_cb(MCU_UART, ql_uart_notify_cb);
    }

    ret = ql_uart_open(DEBUG_UART);
    QL_UART_DEMO_LOG("UART:DEBUG UART OPEN ret: 0x%x", ret);
    if(QL_UART_SUCCESS == ret)
	{
     ret = ql_uart_register_cb(DEBUG_UART, ql_uart_notify_cb);
    }
    APP_DEBUG("DEBUG UART WRITE");
   
    //MCU_UART_WRITE("%s", GET_STATUS);ql_rtos_task_sleep_s(1);
    //MCU_UART_WRITE("%s", GET_STATUS);

    while (1)
    {
      
      // ql_rtos_task_sleep_s(5);
      err = ql_rtos_queue_wait(Uart_queue_01, recv_buff_queue, sizeof(recv_buff_queue), QL_WAIT_FOREVER);
      if (err == QL_OSI_SUCCESS)
	    {
            
			QL_UART_DEMO_LOG("UART:Queue_01 Data Received: %s", recv_buff_queue);
          //  MCU_UART_WRITE("UART REC:%s", recv_buff_queue);
            McuUartHandle((char *)recv_buff_queue);
		}
     // if(recv_buff_queue)free(recv_buff_queue);
    }
    
	

}

void AutoSwitch_Uart_app_init(void)
{
    QlOSStatus err = 0;
	ql_task_t uart_task = NULL;

    err = ql_rtos_queue_create(&Uart_queue_01, QUEUE_01_LEN, sizeof(int));
    if (err != QL_OSI_SUCCESS)
	{
		QL_UART_DEMO_LOG("UART:Queue failed");
        return;
	}
    err = ql_rtos_task_create(&uart_task, 4096, APP_PRIORITY_NORMAL, "QUARTDEMO", uart_demo_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
	{
		QL_UART_DEMO_LOG("UART:demo task created failed");
        return;
	}
}

//-----------------------MCU---->EC200U CODES HANDLERE--------------//
void McuUartHandle( char *Data)
{
    APP_DEBUG("%s\r\n",Data);
     //-----------------------SETTING-----------------------------------//
     APP_DEBUG("PREV_Set=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",Settings.PodSetVal,Settings.SdtSetVal,Settings.OlSetVal,Settings.DrSetVal,Settings.OvSetVal,Settings.UvSetVal,Settings.OlRstTime,Settings.DrRstTime,Settings.Mode,Settings.ReplyMode);
     APP_DEBUG("PREV_FLAG=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",Flags.MotorOn,Flags.Power,Flags.VFault,Flags.CFault,Flags.PreVFault,Flags.PreCFault,Flags.StausSend,Flags.CallState,Flags.HangUpcall,Flags.PonStsSend,Flags.MemoryRead);

	 if(strstr(Data,SETTINGS))
	 {
        sscanf(Data,"%*[^:]:%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",(int *)&Settings.PodSetVal,(int *)&Settings.SdtSetVal,(int *)&Settings.OlSetVal,(int *)&Settings.DrSetVal,(int *)&Settings.OvSetVal,(int *)&Settings.UvSetVal,(int *)&Settings.OlRstTime,(int *)&Settings.DrRstTime,(int *)&Settings.Mode);
        APP_DEBUG("POD           = %d\r\n",Settings.PodSetVal);
        APP_DEBUG("SDT           = %d\r\n",Settings.SdtSetVal);
        APP_DEBUG("OverLoad Curr = %d\r\n",Settings.OlSetVal);
        APP_DEBUG("Dryrun Curr   = %d\r\n",Settings.DrSetVal);
        APP_DEBUG("High Voltage  = %d\r\n",Settings.OvSetVal);
        APP_DEBUG("Under Voltage = %d\r\n",Settings.UvSetVal);
        APP_DEBUG("OverLoad Restart  = %d\r\n",Settings.OlRstTime);
        APP_DEBUG("Dry Run Restart  = %d\r\n",Settings.DrRstTime);
        Settings.Mode= Settings.Mode&0x01;
        APP_DEBUG("Mode          = %d\r\n",Settings.Mode);
        QL_UART_DEMO_LOG("UART:POD:%d,SDT:%d,OL:%d,DR:%d,HV:%d,LO:%d,OLT:%d,DRT:%d,MOD:%d\r\n",Settings.PodSetVal,Settings.SdtSetVal,Settings.OlSetVal,Settings.DrSetVal,Settings.OvSetVal,Settings.UvSetVal,Settings.OlRstTime,Settings.DrRstTime,Settings.Mode);
       //return; 
     }
     //-----------------------STATUS-----------------------------------//
     if(strstr(Data,STATUS))
	 {
        sscanf(Data,"%*[^:]:%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",&RY,(int *)&YB,(int *)&BR,(int *)&Current,(int *)&Flags.MotorOn,(int *)&Flags.Power,(int *)&Flags.VFault,(int *)&Flags.CFault,(int *)&Settings.Mode);
        Settings.Mode= Settings.Mode&0x01;
        APP_DEBUG("RY_VOLT      = %d\r\n",RY);
		APP_DEBUG("YB_VOLT      = %d\r\n",YB);
		APP_DEBUG("BR_VOLT      = %d\r\n",BR);
		APP_DEBUG("CURRENT      = %d\r\n",Current);
		APP_DEBUG("MOTOR        = %d\r\n",Flags.MotorOn);
		APP_DEBUG("POWER        = %d\r\n",Flags.Power);
		APP_DEBUG("VFAULT        = %d\r\n",Flags.VFault);
        APP_DEBUG("CFAULT        = %d\r\n",Flags.CFault);
        APP_DEBUG("MODE         = %d\r\n",Settings.Mode);
        
        APP_DEBUG("Flags.PreVFault=%d,Flags.PreCFault=%d\r\n",Flags.PreVFault,Flags.PreCFault);
        if(((Flags.PreVFault!=Flags.VFault) || (Flags.PreCFault!=Flags.CFault)) && (Flags.MemoryRead ==TRUE))// && (SavePhNum.SaveNumbers[PH1_INDEX-1].PhoneNumb, SendSmsBuff!=NULL))
        {

            APP_DEBUG("UART:FAULT CHANGE\r\n");
            if(((Flags.VFault!=NO_FAULT) && (Flags.VFault!=PRE_PWR_OFF_FAULT))|| (Flags.CFault!=NO_FAULT))
            SendFaultSms(Flags.VFault,Flags.CFault);
            #ifdef POWER_SAVING_EN
            if(Flags.VFault==PWR_OFF_FAULT)
            {
                ql_event_t event;
                event.id = QUEC_SLEEP_ENETR_AUTO_SLEPP;
	            ql_rtos_event_send(ASW_power_task, &event);
            }
            else if ((Flags.PreVFault==PWR_OFF_FAULT) && (Flags.VFault!=PWR_OFF_FAULT))
            {
                ql_event_t event;
                event.id = QUEC_SLEEP_EXIT_AUTO_SLEPP;
	            ql_rtos_event_send(ASW_power_task, &event);
            }
            #endif

            Flags.PreVFault=Flags.VFault;
            Flags.PreCFault=Flags.CFault;
        }

        QL_UART_DEMO_LOG("UART:RY:%d,YB:%d,BR:%d,CU:%d,MOTOR:%d,POWER:%d,VFAULT:%d,CFAULT:%d,MODE:%d\r\n",RY,YB,BR,Current,Flags.MotorOn,Flags.Power,Flags.VFault,Flags.CFault,Settings.Mode);
        //return;
     }
     //--------------------------send power On sms----------------------------//
     if((strstr(Data,PON_SMS)) && (Flags.MemoryRead ==TRUE))
     {
        SendPowerOnSms();
     }

     //--------------------------RESET GSM----------------------------//
     if(strstr(Data,RESET_GSM))
     {
        ql_power_reset(RESET_NORMAL);
     }


    //   if(strstr(Data,"ATH"))
    //  {
    //     SendPowerOnSms();
    //     vc_event.id = ASW_CALL_HANG;
	// 	ql_rtos_event_send(call_task, &vc_event);
    //  }

     APP_DEBUG("AFTER_Set=%d,%d,%d,%d,%d,%d,%d,%d,%d/%d\r\n",Settings.PodSetVal,Settings.SdtSetVal,Settings.OlSetVal,Settings.DrSetVal,Settings.OvSetVal,Settings.UvSetVal,Settings.OlRstTime,Settings.DrRstTime,Settings.Mode,Settings.ReplyMode);
     APP_DEBUG("AFTER_FLAG=%d,%d,%d,%d/%d,%d,%d,%d,%d,%d,%d\r\n",Flags.MotorOn,Flags.Power,Flags.VFault,Flags.CFault,Flags.PreVFault,Flags.PreCFault,Flags.StausSend,Flags.CallState,Flags.HangUpcall,Flags.PonStsSend,Flags.MemoryRead);

}

