/*=================================================================

						EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN			  WHO		  WHAT, WHERE, WHY
------------	 -------	 -------------------------------------------------------------------------------

=================================================================*/


/*===========================================================================
 * include files
 ===========================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_pin_cfg.h"
#include "ql_uart.h"
#include "Defination.h"
#include "power.h"
#include "uart.h"

#ifdef QL_APP_FEATURE_USB
#include "ql_usb.h"
#endif

/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define QL_POWERDEMO_LOG_LEVEL             QL_LOG_LEVEL_INFO
#define QL_POWERDEMO_LOG(msg, ...)         QL_LOG(QL_POWERDEMO_LOG_LEVEL, "ASW_POWER", msg, ##__VA_ARGS__)
#define QL_POWERDEMO_LOG_PUSH(msg, ...)    QL_LOG_PUSH("ql_POWER", msg, ##__VA_ARGS__)

#if !defined(require_action)
	#define require_action(x, action, str)																		\
			do                                                                                                  \
			{                                                                                                   \
				if(x != 0)                                                                        				\
				{                                                                                               \
					QL_POWERDEMO_LOG(str);																		\
					{action;}																					\
				}                                                                                               \
			} while( 1==0 )
#endif

/*===========================================================================
 * Variate
 ===========================================================================*/
ql_task_t ASW_power_task = NULL;
//ql_timer_t power_timer = NULL;
int wake_lock_1, wake_lock_2;

//ql_task_t pwrkey_task = NULL;

extern char DBG_BUFFER[];
extern char MCU_BUFFER[];

/*===========================================================================
 * Functions
 ===========================================================================*/
//Sleep callback function is executed before sleep, custom can close some pins to reduce leakage or saving some information in here
//Caution:callback functions cannot run too much code 
void ASW_enter_sleep_cb(void* ctx)
{   
    //QL_POWERDEMO_LOG("enter sleep cb");

#ifdef QL_APP_FEATURE_GNSS
    ql_pin_set_func(QL_PIN_NUM_KEYOUT_5, QL_FUN_NUM_UART_2_CTS);  //keyout5 pin need be low level when enter sleep, adjust the function to uart2_rts can do it
    ql_gpio_set_level(GPIO_12, LVL_HIGH);                         //close mos linked to gnss, to avoid high current in sleep mode
    ql_gpio_set_level(GPIO_11, LVL_LOW);                          //gpio11 need be low level when enter sleep to reduce leakage current to gnss
#endif
}

//exit sleep callback function is executed after exiting sleep, custom can recover the information before sleep
//Caution:callback functions cannot run too much code 
void ASW_exit_sleep_cb(void* ctx)
{   
    //QL_POWERDEMO_LOG("exit sleep cb");  
    
#ifdef QL_APP_FEATURE_GNSS
    ql_pin_set_func(QL_PIN_NUM_KEYOUT_5, QL_FUN_NUM_UART_3_TXD);  //keyout5 pin used as gnss uart3_txd function, after exit sleep, set it to uart3_txd
#endif    
}

#ifdef QL_APP_FEATURE_USB
int ASW_usb_hotplug_cb(QL_USB_HOTPLUG_E state, void *ctx)
{
	if(state == QL_USB_HOTPLUG_OUT)
	{
		QL_POWERDEMO_LOG("USB plug out");
	}
	else
	{
		QL_POWERDEMO_LOG("USB inserted");
	}

	return 0;
}
#endif


static void Asw_power_demo_thread(void *param)
{
    //QL_POWERDEMO_LOG("power demo thread enter, param 0x%x", param);

	ql_event_t event;
	int err;

    //register sleep callback function
    ql_sleep_register_cb(ASW_enter_sleep_cb);
    
    //register wakeup callback function
    ql_wakeup_register_cb(ASW_exit_sleep_cb);

#ifdef QL_APP_FEATURE_USB	
	//register usb hotplug callback function
	ql_usb_bind_hotplug_cb(ASW_usb_hotplug_cb);
#endif
    // ql_rtos_task_sleep_s(20);
    // event.id = QUEC_SLEEP_ENETR_AUTO_SLEPP;
	// ql_rtos_event_send(power_task, &event);
	while(1)
	{
		if(ql_event_try_wait(&event) != 0)
		{
			continue;
		}	
		QL_POWERDEMO_LOG("receive event, id is %d", event.id);
		//MCU_UART_WRITE("UART:receive event, id is %d\r\n", event.id);
		switch(event.id)
		{
			case QUEC_SLEEP_ENETR_AUTO_SLEPP:
				ql_rtos_task_sleep_s(20);
				err = ql_autosleep_enable(QL_ALLOW_SLEEP);
                if( err != 0 )
                {
                    QL_POWERDEMO_LOG("failed to set auto sleep");
                    break;
                }

				err = ql_lpm_wakelock_unlock(wake_lock_1);
                if( err != 0 )
                {
                    QL_POWERDEMO_LOG("lock1 unlocked failed");
                    break;
                }

				err = ql_lpm_wakelock_unlock(wake_lock_2);
                if( err != 0 )
                {
                    QL_POWERDEMO_LOG("lock2 unlocked failed");
                    break;
                }
				
				QL_POWERDEMO_LOG("set auto sleep mode ok");
				// MCU_UART_WRITE("UART:set auto sleep mode ok\r\n");
			break;

			case QUEC_SLEEP_EXIT_AUTO_SLEPP:
				err = ql_autosleep_enable(QL_NOT_ALLOW_SLEEP);
                if( err != 0 )
                {
                    QL_POWERDEMO_LOG("failed to set auto sleep");
                     MCU_UART_WRITE("UART:failed to set auto sleep\r\n");
                    break;
                }

			break;

			case QUEC_SLEEP_QUICK_POWER_DOWM:
				ql_power_down(POWD_IMMDLY);
			break;

			case QUEC_SLEEP_NORMAL_POWER_DOWM:
				ql_power_down(POWD_NORMAL);
			break;

			case QUEC_SLEEP_QUICK_RESET:
				ql_power_reset(RESET_QUICK);
			break;

			case QUEC_SLEEP_NORMAL_RESET:
				ql_power_reset(RESET_NORMAL);
			break;

			default:
			break;
		}
	}

    ql_rtos_task_delete(NULL);
}

void ASW_power_timer_callback(void *ctx)
{
	// ql_event_t event = {0};

	// event.id = QUEC_SLEEP_ENETR_AUTO_SLEPP;
	//ql_rtos_event_send(power_task, &event);
}

void ASW_power_app_init(void)
{
    
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&ASW_power_task, 1024, APP_PRIORITY_NORMAL, "ql_powerdemo", Asw_power_demo_thread, NULL, 3);
	require_action(err, return, "power demo task created failed");

}

