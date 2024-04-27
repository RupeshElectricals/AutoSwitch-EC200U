#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "Defination.h"
#include "embedded_flash.h"
#include "ql_embed_nor_flash.h"
#include "sms.h"
#define QL_APP_EMBED_NOR_FLASH_LOG_LEVEL             QL_LOG_LEVEL_INFO
#define QL_EMBED_NOR_FLASH_LOG(msg, ...)             QL_LOG(QL_APP_EMBED_NOR_FLASH_LOG_LEVEL, "QL_embed_flash", msg, ##__VA_ARGS__)


ql_task_t embed_flash_task = NULL;
ql_event_t embed_flash_event= {0};

extern struct SavePhone SavePhNum;
extern struct Setting Settings;
extern struct Flag Flags;


static void Embedded_flash_task(void *param)
{
  ql_errcode_e ret=QL_SUCCESS;
  char FlashBuff[5];
  while (1)
  {
   QL_EMBED_NOR_FLASH_LOG("FLASH: EMBEDED FLASH TASK");
   ql_event_wait(&embed_flash_event, QL_WAIT_FOREVER);
   QL_EMBED_NOR_FLASH_LOG("FLASH: EMBEDED FLASH TASK RECEIVED %d",embed_flash_event.id);
   switch (embed_flash_event.id)
   {
   case ASW_FLASH_WRITE:
    ret=ql_embed_nor_flash_erase(FLASH_ADDR,SECTOR_SIZE);
    if(ret==QL_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("FLASH: Settings.ReplyMode =%d",Settings.ReplyMode);

        FlashBuff[0]= Settings.ReplyMode&0x0F;
       // ret=ql_embed_nor_flash_read(FLASH_ADDR,(void *)FlashBuff,1);
    
        ret=ql_embed_nor_flash_write(REPLY_MODE_ADD,(void *)&Settings.ReplyMode,sizeof(uint8_t));
        ret=ql_embed_nor_flash_write(ADMIN1_ADD,(void *)&SavePhNum.SaveNumbers[ADMIN1_INDEX-1].PhoneNumb,strlen((const char *)SavePhNum.SaveNumbers[ADMIN1_INDEX-1].PhoneNumb));
        ret=ql_embed_nor_flash_write(ADMIN2_ADD,(void *)&SavePhNum.SaveNumbers[ADMIN2_INDEX-1].PhoneNumb,strlen((const char *)SavePhNum.SaveNumbers[ADMIN2_INDEX-1].PhoneNumb));
        ret=ql_embed_nor_flash_write(ADMIN3_ADD,(void *)&SavePhNum.SaveNumbers[ADMIN3_INDEX-1].PhoneNumb,strlen((const char *)SavePhNum.SaveNumbers[ADMIN3_INDEX-1].PhoneNumb));
        ret=ql_embed_nor_flash_write(ADMIN4_ADD,(void *)&SavePhNum.SaveNumbers[ADMIN4_INDEX-1].PhoneNumb,strlen((const char *)SavePhNum.SaveNumbers[ADMIN4_INDEX-1].PhoneNumb));
        ret=ql_embed_nor_flash_write(ADMIN5_ADD,(void *)&SavePhNum.SaveNumbers[ADMIN5_INDEX-1].PhoneNumb,strlen((const char *)SavePhNum.SaveNumbers[ADMIN5_INDEX-1].PhoneNumb));
        ret=ql_embed_nor_flash_write(ICCID_ADD, (void *)&SavePhNum.ICCID_Array,strlen((const char *)SavePhNum.ICCID_Array));
         QL_EMBED_NOR_FLASH_LOG("FLASH: WRITE DONE");
    }
    else
    {
        QL_EMBED_NOR_FLASH_LOG("FLASH:embed nor flash erase faild,erase addr:0x%X",FLASH_ADDR);
    }
    break;
    case ASW_FLASH_READ:
    
        ret=ql_embed_nor_flash_read(REPLY_MODE_ADD,(void *)FlashBuff,1);
        Settings.ReplyMode = FlashBuff[0] &0x0F;
        ret=ql_embed_nor_flash_read(ADMIN1_ADD,(void *)SavePhNum.SaveNumbers[ADMIN1_INDEX - 1].PhoneNumb,10);
        ret=ql_embed_nor_flash_read(ADMIN2_ADD,(void *)SavePhNum.SaveNumbers[ADMIN2_INDEX - 1].PhoneNumb,10);
        ret=ql_embed_nor_flash_read(ADMIN3_ADD,(void *)SavePhNum.SaveNumbers[ADMIN3_INDEX - 1].PhoneNumb,10);
        ret=ql_embed_nor_flash_read(ADMIN4_ADD,(void *)SavePhNum.SaveNumbers[ADMIN4_INDEX - 1].PhoneNumb,10);
        ret=ql_embed_nor_flash_read(ADMIN5_ADD,(void *)SavePhNum.SaveNumbers[ADMIN5_INDEX - 1].PhoneNumb,10);
        ret=ql_embed_nor_flash_read(ICCID_ADD,(void *)SavePhNum.ICCID_Array,20);

        
        QL_EMBED_NOR_FLASH_LOG("FLASH:read REPLY_MODE addr 0x%X,content:%d",REPLY_MODE_ADD,Settings.ReplyMode);
        QL_EMBED_NOR_FLASH_LOG("FLASH:read ADMIN1 addr 0x%X,content:%s",ADMIN1_ADD,SavePhNum.SaveNumbers[ADMIN1_INDEX - 1].PhoneNumb);
        QL_EMBED_NOR_FLASH_LOG("FLASH:read ADMIN2 addr 0x%X,content:%s",ADMIN2_ADD,SavePhNum.SaveNumbers[ADMIN2_INDEX - 1].PhoneNumb);
        QL_EMBED_NOR_FLASH_LOG("FLASH:read ADMIN3 addr 0x%X,content:%s",ADMIN3_ADD,SavePhNum.SaveNumbers[ADMIN3_INDEX - 1].PhoneNumb);
        QL_EMBED_NOR_FLASH_LOG("FLASH:read ADMIN4 addr 0x%X,content:%s",ADMIN4_ADD,SavePhNum.SaveNumbers[ADMIN4_INDEX - 1].PhoneNumb);
        QL_EMBED_NOR_FLASH_LOG("FLASH:read ADMIN5 addr 0x%X,content:%s",ADMIN5_ADD,SavePhNum.SaveNumbers[ADMIN5_INDEX - 1].PhoneNumb);
        QL_EMBED_NOR_FLASH_LOG("FLASH:read ICCID  addr 0x%X,content:%s",ICCID_ADD,SavePhNum.ICCID_Array);
       
        //QL_EMBED_NOR_FLASH_LOG("FLASH:read addr 0x%X,content:%d",TEST_ADD,testMem.Test);
        Flags.MemoryRead = TRUE; 
       
    break;
   
   default:
    break;
   }
  } 
}

void AutoSwitch_embed_flash_app_init(void)
{
    int err = QL_OSI_SUCCESS;
        
    err = ql_rtos_task_create(&embed_flash_task, 15*1024,23,"AUTO_embed_flash", Embedded_flash_task, NULL,1);
	
    if (err != QL_OSI_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("embed nor flash demo task created failed");
    }

    ql_rtos_task_sleep_s(5);
    embed_flash_event.id =  ASW_FLASH_READ;
    ql_rtos_event_send(embed_flash_task, &embed_flash_event);  

}

