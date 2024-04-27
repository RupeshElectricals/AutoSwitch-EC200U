#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_app_feature_config.h"
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_nw.h"
#include "ql_api_datacall.h"
#include "ql_api_voice_call.h"
#include "ql_type.h"
#include "Defination.h"
#include "network.h"
#include "ql_uart.h"
#include "mqtt.h"
#include "Defination.h"
#include "sms.h"
#include "uart.h"



#define QL_NW_LOG_LEVEL	                    QL_LOG_LEVEL_INFO
#define QL_NW_DEMO_LOG(msg, ...)			QL_LOG(QL_NW_LOG_LEVEL, "asw_nw_demo", msg, ##__VA_ARGS__)

ql_task_t nw_task = NULL;
ql_event_t nw_event ={0};


extern char DBG_BUFFER[];
extern char MCU_BUFFER[];

extern const char GET_STATUS[];

#ifdef MQTT_ENABLE
extern ql_task_t mqtt_task;
extern ql_event_t mqtt_event;
#endif 

UINT8 OpNameBuff[33];

void ql_nw_notify_cb(uint8_t sim_id, unsigned int ind_type, void *ind_msg_buf)
{    
    switch(ind_type)
    {
        case QUEC_NW_VOICE_REG_STATUS_IND:
        {
           ql_nw_common_reg_status_info_s  *voice_reg_status=(ql_nw_common_reg_status_info_s  *)ind_msg_buf;
           QL_NW_DEMO_LOG("NW: Sim%d voice: state:%d",sim_id,voice_reg_status->state);
            nw_event.id = QUEC_NW_VOICE_REG_STATUS_IND;
            nw_event.param1 = voice_reg_status->state;
            ql_rtos_event_send(nw_task,&nw_event);
        }
         break;

        case QUEC_NW_DATA_REG_STATUS_IND:
        {
           ql_nw_common_reg_status_info_s  *data_reg_status=(ql_nw_common_reg_status_info_s  *)ind_msg_buf;
           QL_NW_DEMO_LOG("NW: Sim%d data: state:%d",sim_id,data_reg_status->state);
            nw_event.id = QUEC_NW_DATA_REG_STATUS_IND;
            nw_event.param1 = data_reg_status->state;
            ql_rtos_event_send(nw_task,&nw_event);
        }
         break; 
        case QUEC_NW_NITZ_TIME_UPDATE_IND:
        {
             ql_nw_nitz_time_info_s  *nitz_info=(ql_nw_nitz_time_info_s  *)ind_msg_buf;
             QL_NW_DEMO_LOG("nitz update: nitz_time:%s, abs_time:%ld", nitz_info->nitz_time, nitz_info->abs_time);
             nw_event.id = QUEC_NW_DATA_REG_STATUS_IND;
             ql_rtos_event_send(nw_task,&nw_event);
             break;
        }
    }
}


static void nw_thread(void * arg)
{
    int ret = 0;
	uint8_t sim_id = ACTIVE_SIM;
    ql_nw_reg_status_info_s nw_info = {0};
    //ql_data_call_default_pdn_info_s apn_info= {0};
    #ifdef MQTT_ENABLE
    ql_data_call_info_s info;
    char ip4_addr_str[16] = {0};
    char ip6_addr_str[50] = {0};
    #endif


    if(ql_nw_register_cb(ql_nw_notify_cb) != QL_NW_SUCCESS)
    {
		QL_NW_DEMO_LOG("register network callback fail");
	}


    ql_data_call_default_pdn_info_s *apn_info = (ql_data_call_default_pdn_info_s *)calloc(1, sizeof(ql_data_call_default_pdn_info_s));
    ret = ql_datacall_get_default_pdn_info(sim_id,apn_info);  
    QL_NW_DEMO_LOG("NW: ret=0x%x, ip_version:%d apn_name:%s", 
                ret, apn_info->ip_version,apn_info->apn_name);    

    ql_data_call_default_pdn_cfg_s *defualt_apn_info = (ql_data_call_default_pdn_cfg_s *)calloc(1, sizeof(ql_data_call_default_pdn_cfg_s));
    ret = ql_datacall_get_default_pdn_cfg(sim_id,defualt_apn_info);  
    QL_NW_DEMO_LOG("NW: DEFAULT ret=0x%x, ip_version:%d apn_name:%s", 
                ret, defualt_apn_info->ip_version,defualt_apn_info->apn_name);

    // ret = ql_datacall_set_default_pdn_cfg(sim_id,defualt_apn_info);  

    // QL_NW_DEMO_LOG("NW: DEFAULT SET ret=0x%x, ip_version:%d apn_name:%s", 
    //             ret, defualt_apn_info->ip_version,defualt_apn_info->apn_name);


    while (1)
    {
        ql_event_wait(&nw_event,QL_WAIT_FOREVER);
        switch(nw_event.id)
        {
        case QUEC_NW_VOICE_REG_STATUS_IND:
            ret = ql_nw_get_reg_status(sim_id, &nw_info);
            if(nw_info.voice_reg.state == QL_NW_REG_STATE_HOME_NETWORK || nw_info.voice_reg.state == QL_NW_REG_STATE_ROAMING) 
            {
                 QL_NW_DEMO_LOG("NW: VOICE CREG=%d",nw_info.voice_reg.state);
                 ql_nw_operator_info_s *oper_info = (ql_nw_operator_info_s *)calloc(1, sizeof(ql_nw_operator_info_s));
                 ret = ql_nw_get_operator_name(sim_id, oper_info);
                 QL_NW_DEMO_LOG("NW: ret=0x%x, long_oper_name:%s, short_oper_name:%s, mcc:%s, mnc:%s", 
                                ret, oper_info->long_oper_name, oper_info->short_oper_name, oper_info->mcc, oper_info->mnc);
                 memcpy(OpNameBuff,oper_info->long_oper_name,strlen(oper_info->long_oper_name));
                 StrToUpper((char *)OpNameBuff);
                // ql_voice_call_start(ACTIVE_SIM,(char *)"8600687322");
                MCU_UART_WRITE("%s", GET_STATUS);
            }
         break;

        case QUEC_NW_DATA_REG_STATUS_IND:
            ret = ql_nw_get_reg_status(sim_id, &nw_info);
            if(nw_info.data_reg.state == QL_NW_REG_STATE_HOME_NETWORK || nw_info.data_reg.state == QL_NW_REG_STATE_ROAMING) 
            {
                 QL_NW_DEMO_LOG("NW: DATA CGREG=%d",nw_info.voice_reg.state);
                 ql_nw_operator_info_s *oper_info = (ql_nw_operator_info_s *)calloc(1, sizeof(ql_nw_operator_info_s));
                 ret = ql_nw_get_operator_name(sim_id, oper_info);
                 QL_NW_DEMO_LOG("NW: ret=0x%x, long_oper_name:%s, short_oper_name:%s, mcc:%s, mnc:%s", 
                                ret, oper_info->long_oper_name, oper_info->short_oper_name, oper_info->mcc, oper_info->mnc);
                
                ql_data_call_default_pdn_info_s *apn_info = (ql_data_call_default_pdn_info_s *)calloc(1, sizeof(ql_data_call_default_pdn_info_s));
                 ret = ql_datacall_get_default_pdn_info(sim_id,apn_info);  
                 QL_NW_DEMO_LOG("NW: ret=0x%x, ip_version:%d apn_name:%s", 
                                ret, apn_info->ip_version,apn_info->apn_name);    

                 ql_data_call_default_pdn_cfg_s *defualt_apn_info = (ql_data_call_default_pdn_cfg_s *)calloc(1, sizeof(ql_data_call_default_pdn_cfg_s));
                 ret = ql_datacall_get_default_pdn_cfg(sim_id,defualt_apn_info);  
                 QL_NW_DEMO_LOG("NW: DEFAULT ret=0x%x, ip_version:%d apn_name:%s", 
                                ret, defualt_apn_info->ip_version,defualt_apn_info->apn_name);                 

               #ifdef MQTT_ENABLE
                if(ql_datacall_get_sim_profile_is_active(sim_id,PROFILE_IDX)==0)
                {

                    
                 ql_set_data_call_asyn_mode(sim_id, PROFILE_IDX, 0);
                 ql_rtos_task_sleep_s(2);
                 ret=ql_start_data_call(sim_id, PROFILE_IDX , apn_info->ip_version,apn_info->apn_name, NULL, NULL, 0);
                 QL_NW_DEMO_LOG("NW:===data call result:%d", ret);
	             if(ret != 0)
                  {
		            QL_NW_DEMO_LOG("NW:====data call failure!!!!=====");
                    break;
	              }

                 memset(&info, 0x00, sizeof(ql_data_call_info_s));
	             ret = ql_get_data_call_info(sim_id, PROFILE_IDX, &info);
                 
                 QL_NW_DEMO_LOG("NW:info->profile_idx: %d", info.profile_idx);
	             QL_NW_DEMO_LOG("NW:info->ip_version: %d", info.ip_version);
                        
                 QL_NW_DEMO_LOG("NW:info->v4.state: %d", info.v4.state); 
                 inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
                 QL_NW_DEMO_LOG("NW:info.v4.addr.ip: %s\r\n", ip4_addr_str);

                 QL_NW_DEMO_LOG("NW:info->v6.state: %d", info.v6.state); 
                 inet_ntop(AF_INET6, &info.v6.addr.ip, ip6_addr_str, sizeof(ip6_addr_str));
                 QL_NW_DEMO_LOG("NW:info.v6.addr.ip: %s\r\n", ip6_addr_str);


                 mqtt_event.id = ASW_MQTT_OPEN;
                 ql_rtos_event_send(mqtt_task,&mqtt_event);

                }

                 #endif              
                   
            }
         break; 
        case QUEC_NW_NITZ_TIME_UPDATE_IND:
             QL_NW_DEMO_LOG("nitz update: nitz_time");
         break;
        }
    }
    
}

void AutoSwitch_nw_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;
    
    err = ql_rtos_task_create(&nw_task, 1024*4, APP_PRIORITY_NORMAL, "QNWDEMO", nw_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
	{
		QL_NW_DEMO_LOG("created task failed");
	}
}
