#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_sim.h"
#include "Defination.h"
#include "phonebook.h"
#include "sms.h"
#include "embedded_flash.h"

#define QL_PBK_LOG_LEVEL           QL_LOG_LEVEL_INFO
#define QL_PBK_LOG(msg, ...)       QL_LOG(QL_PBK_LOG_LEVEL, "ql_pbk_demo", msg, ##__VA_ARGS__)

ql_task_t pbk_task = NULL;
ql_sem_t pbk_init_sem = NULL;
ql_event_t pbk_event = {0};

extern struct SavePhone SavePhNum;
char ICCIDCode[25];

extern ql_task_t embed_flash_task;
extern ql_event_t embed_flash_event;


void user_pbk_event_callback(uint8_t nSim, int event_id, void *ctx)
{
    switch (event_id)
    {
    case QL_PBK_INIT_OK_IND: {
        QL_PBK_LOG("PH:QL_PBK_INIT_OK_IND sim:%d", nSim);
        if (0 == nSim)
        {
            ql_rtos_semaphore_release(pbk_init_sem);
        }
        break;
    }
    default:
        break;
    }
}

static void phonebook_task(void *param)
{
   // QlOSStatus err = QL_OSI_SUCCESS;
    ql_sim_errcode_e ret = QL_SIM_SUCCESS;
    uint8_t sim_id = ACTIVE_SIM;
   // int start_index = 1;
   // int end_index = 1;
   // uint8_t *username = NULL;
   // uint8_t username_len = 0;
    // char siminfo[64] = {0};
    ql_sim_pbk_item_info_s item = {0};
    ql_sim_pbk_itemset_info_s *itemset = NULL;

    itemset = (ql_sim_pbk_itemset_info_s *)malloc(sizeof(ql_sim_pbk_itemset_info_s));
    if (itemset == NULL)
    {
        QL_PBK_LOG("PH:malloc itemset fail");
      //  goto exit;
    }

    ql_pbk_callback_register(user_pbk_event_callback);
    ql_sim_set_pbk_encoding(QL_SIM_PBK_GSM);
    
    //wait pbk ok
    if (ql_rtos_semaphore_wait(pbk_init_sem, QL_WAIT_FOREVER))
    {
        QL_PBK_LOG("PH:Waiting for PBK init timeout");
    }


    // ret = ql_sim_get_imsi(sim_id, siminfo, 64);
	// QL_PBK_LOG("ret:0x%x, IMSI: %s", ret, siminfo);

  

    //ret = ql_sim_get_phonenumber(sim_id, siminfo, 64);
    //QL_PBK_LOG("ret:0x%x, phonenumber: %s", ret, siminfo);

    // item.index = 1;
    // strcpy((char *)item.username, "Admin");
    // strcpy((char *)item.phonenum, "8600687322");
    // item.username_len = strlen((const char *)item.username);

    // ret = ql_sim_write_pbk_item(sim_id, QL_SIM_PBK_STORAGE_SM, &item);
    // QL_PBK_LOG("add item ret:0x%x", ret);

    //read
    // start_index = 1;
    // //end_index = QL_PBK_ITEM_COUNT_MAX * 2; //You can actually read QL_PBK_ITEM_COUNT_MAX items
    // end_index = MAX_PH_SAVE;
    // memset((void *)itemset, 0, sizeof(ql_sim_pbk_itemset_info_s));

    // ret = ql_sim_read_pbk_item(sim_id, QL_SIM_PBK_STORAGE_ME, start_index, end_index, username, username_len, itemset);
    // QL_PBK_LOG("PH:read item ret:0x%x", ret);
    // if (QL_SIM_SUCCESS == ret)
    // {
    //     QL_PBK_LOG("PH:read item count:%d", itemset->item_count);
    //     for (size_t i = 0; i < itemset->item_count; i++)
    //     {
    //         strcpy((char *)SavePhNum.SaveNumbers[i].PhoneNumb,(const char *)itemset->item[i].phonenum);
    //         // QL_PBK_LOG("PH:read item index:%-3d phonenum:%-24.24s username:%-32.32s namelen:%d",
    //         //            itemset->item[i].index,
    //         //            itemset->item[i].phonenum,
    //         //            itemset->item[i].username,
    //         //            itemset->item[i].username_len);
    //         QL_PBK_LOG("PH:PhoneBook %d-->%s",i,SavePhNum.SaveNumbers[i].PhoneNumb);     
    //     }
    // }

    ret = ql_sim_get_iccid(sim_id, ICCIDCode, 20);
    QL_PBK_LOG("ret:0x%x, ICCID: %s", ret, ICCIDCode);
    ICCIDCode[strlen((const char*)ICCIDCode)-1]='\0';
    ql_rtos_task_sleep_s(5);
   

    // start_index = ICCID;
    // end_index = ICCID+2;
    // memset((void *)itemset, 0, sizeof(ql_sim_pbk_itemset_info_s));
    // ret = ql_sim_read_pbk_item(sim_id, QL_SIM_PBK_STORAGE_SM, start_index, end_index, username, username_len, itemset);
    // QL_PBK_LOG("PH:read ICCID ret:0x%x", ret);
    // if (QL_SIM_SUCCESS == ret)
    // {
    //     for (size_t i = 0; i < itemset->item_count; i++)
    //     {
    //     QL_PBK_LOG("PH:read item index:%-3d phonenum:%-24.24s username:%-32.32s namelen:%d",
    //                    itemset->item[i].index,
    //                    itemset->item[i].phonenum,
    //                    itemset->item[i].username,
    //                    itemset->item[i].username_len);
    //     }
    //     QL_PBK_LOG("PH: ICCID Len =%d ",strlen((const char *)itemset->item[0].phonenum));

    //     if(strlen((const char *)itemset->item[0].phonenum)<10)
    //     {
    //         item.index = ICCID;
    //         strcpy((char *)item.username, "ICCID");
    //         strcpy((char *)item.phonenum, (char *)ICCIDCode);
    //         item.username_len = strlen((const char *)item.username);
    //          QL_PBK_LOG("%d %s  %s  %d  %s", item.index,item.username,item.phonenum,item.username_len,ICCIDCode);
    //         ret = ql_sim_write_pbk_item(sim_id, QL_SIM_PBK_STORAGE_SM, &item);
    //         QL_PBK_LOG("add item ret:0x%x", ret);
    //     }
    //      if(strstr(ICCIDCode,(const char *)itemset->item[itemset->item_count].phonenum))
    //     {
    //            QL_PBK_LOG("PH:SIM CARD CORRECT ");
    //     }
    //     else
    //     {
    //         QL_PBK_LOG("PH:SIM CARD IS CHANGED");
    //     }
    
    //}

    while (1)
    {
        ql_event_wait(&pbk_event, QL_WAIT_FOREVER);
         QL_PBK_LOG("PH:PBK EVENT");
        switch (pbk_event.id)
        {
        case ASW_PBK_SAVE:
            item.index = pbk_event.param1;
            strcpy((char *)item.username, "Admin");
            strcpy((char *)item.phonenum, (char *)SavePhNum.SaveNumbers[pbk_event.param1-1].PhoneNumb);
            item.username_len = strlen((const char *)item.username);
            ret = ql_sim_write_pbk_item(sim_id, QL_SIM_PBK_STORAGE_ME, &item);
            QL_PBK_LOG("add item ret:0x%x", ret);
            break;


        case ASW_PBK_READ:
        
            break;    
        
        default:
            break;
        }
    }
    //while(1);
  //ql_rtos_task_delete(NULL);

}

void AutoSwitch_pbk_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_semaphore_create(&pbk_init_sem, 0);
    if (err != QL_OSI_SUCCESS)
    {
        QL_PBK_LOG("PH:pbk_init_sem created failed, ret = 0x%x", err);
    }

    err = ql_rtos_task_create(&pbk_task, 1*1024, APP_PRIORITY_NORMAL, "AUTO_PHONEBOOK", phonebook_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        QL_PBK_LOG("PH:task created failed");
    }
}
