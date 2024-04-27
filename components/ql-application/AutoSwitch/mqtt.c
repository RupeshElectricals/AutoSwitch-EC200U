#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_api_nw.h"
#include "ql_api_dev.h"
#include "ql_log.h"
#include "ql_api_datacall.h"
#include "ql_mqttclient.h"
#include "Defination.h"
#include "network.h"

#include "ql_ssl.h"

#define QL_MQTT_LOG_LEVEL	            QL_LOG_LEVEL_INFO
#define QL_MQTT_LOG(msg, ...)			QL_LOG(QL_MQTT_LOG_LEVEL, "ql_MQTT", msg, ##__VA_ARGS__)
#define QL_MQTT_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_MQTT", msg, ##__VA_ARGS__)
ql_task_t mqtt_task = NULL;
ql_event_t mqtt_event;
static ql_sem_t  mqtt_semp;


#define MQTT_CLIENT_IDENTITY        "quectel_01"
#define MQTT_CLIENT_USER            ""
#define MQTT_CLIENT_PASS            ""

#define MQTT_CLIENT_QUECTEL_URL    "mqtt://220.180.239.212:8306"

static int  mqtt_connected = 0;
char IMEI_buff[64] = {0};
char PubData[100];

static void mqtt_state_exception_cb(mqtt_client_t *client)
{
	QL_MQTT_LOG("mqtt session abnormal disconnect");
	mqtt_connected = 0;
}

static void mqtt_connect_result_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_e status)
{
	QL_MQTT_LOG("status: %d", status);
	if(status == 0){
		mqtt_connected = 1;
	}
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_inpub_data_cb(mqtt_client_t *client, void *arg, int pkt_id, const char *topic, const unsigned char *payload, unsigned short payload_len)
{
	QL_MQTT_LOG("topic: %s", topic);
	QL_MQTT_LOG("payload: %s", payload);
    mqtt_event.id = ASW_MQTT_PUB;
    ql_rtos_event_send(mqtt_task,&mqtt_event);
}

static void mqtt_requst_result_cb(mqtt_client_t *client, void *arg,int err)
{
	QL_MQTT_LOG("err: %d", err);
	
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_app_thread(void * arg)
{
   int ret = 0; 
   char SubTopic[64];
   char PubTopic[64];
   mqtt_client_t  mqtt_cli;
   uint16_t sim_cid;
   struct mqtt_connect_client_info_t  client_info = {0};
   ql_rtos_semaphore_create(&mqtt_semp, 0);
   ql_rtos_task_sleep_s(2);
   ql_dev_get_imei(IMEI_buff, 64, 0);
   QL_MQTT_LOG("IMEI: %s", IMEI_buff);
   sprintf(SubTopic,"Topic/%s/Sub",IMEI_buff);
   sprintf(PubTopic,"Topic/%s/Pub",IMEI_buff);
   QL_MQTT_LOG("SubTopic: %s", SubTopic);
   QL_MQTT_LOG("PubTopic: %s", PubTopic);
   
   while(1)
   {
        ql_event_wait(&mqtt_event,QL_WAIT_FOREVER);

        switch (mqtt_event.id)
        {
            case ASW_MQTT_OPEN:
            {
                if(QL_DATACALL_SUCCESS != ql_bind_sim_and_profile(ACTIVE_SIM, PROFILE_IDX, &sim_cid))
                {
                    QL_MQTT_LOG("nSim or profile_idx is invalid!!!!");
                    break;
                }
                
                if(ql_mqtt_client_init(&mqtt_cli, sim_cid) != MQTTCLIENT_SUCCESS){
                    QL_MQTT_LOG("mqtt client init failed!!!!");
                    break;
                }
                
                

                QL_MQTT_LOG("mqtt_cli:%d", mqtt_cli);
                client_info.keep_alive = 3600;
                client_info.pkt_timeout = 5;
                client_info.retry_times = 3;
                client_info.clean_session = 1;
                client_info.will_qos = 0;
                client_info.will_retain = 0;
                client_info.will_topic = NULL;
                client_info.will_msg = NULL;
                client_info.client_id = IMEI_buff;
                client_info.client_user = MQTT_CLIENT_USER;
                client_info.client_pass = MQTT_CLIENT_PASS;
                ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_QUECTEL_URL , mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);

                if(ret  == MQTTCLIENT_WOUNDBLOCK)
                {
                    QL_MQTT_LOG("====wait connect result");
                    ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
                    if(mqtt_connected == 0)
                    {
                        ql_mqtt_client_deinit(&mqtt_cli);
                        break;
			        }
		        }else
                {
			        QL_MQTT_LOG("===mqtt connect failed ,ret = %d",ret);
			        break;
		        }

        		ql_mqtt_set_inpub_callback(&mqtt_cli, mqtt_inpub_data_cb, NULL);
                
                if(mqtt_connected == 1)
                {
                    if(ql_mqtt_sub_unsub(&mqtt_cli, SubTopic, 1, mqtt_requst_result_cb,NULL, 1) == MQTTCLIENT_WOUNDBLOCK)
                    {
    				    QL_MQTT_LOG("======wait subscrible result");
    				    ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
                        QL_MQTT_LOG("subscrible complete");
    			    }
                    //  mqtt_event.id = ASW_MQTT_PUB;
                    //ql_rtos_event_send(mqtt_task,&mqtt_event);
                }
              break;  
            }
            case ASW_MQTT_PUB:
            {
             strcpy(PubData,"This is test data\0");
             if(ql_mqtt_publish(&mqtt_cli, PubTopic, PubData, strlen(PubData), 0, 0, mqtt_requst_result_cb,NULL) == MQTTCLIENT_WOUNDBLOCK){
    				QL_MQTT_LOG("======wait publish result");
    				ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
                    QL_MQTT_LOG("publish complete");
    			}
             break;
            }
        }
   }

}


int AutoSwitch_mqtt_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	
    err = ql_rtos_task_create(&mqtt_task, 4*1024, APP_PRIORITY_ABOVE_NORMAL, "QmqttApp", mqtt_app_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_MQTT_LOG("mqtt_app init failed");
	}

	return err;
}