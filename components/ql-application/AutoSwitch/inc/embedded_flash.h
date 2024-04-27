/**  @file
  embed_nor_flash_demo.h

  @brief
  This file is demo of embed flash.

*/

/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/

#ifndef _EMBEDDDED_FLASH_H_
#define _EMBEDDDED_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * include files
 ===========================================================================*/

/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define SECTOR_SIZE  4096
#define FLASH_ADDR   0X60498000//0x60558000 //注意： demo 使用的预留flash分区地址，根据自己需求自行修改，确保读写位置为预留地址 
#define REPLY_MODE_ADD FLASH_ADDR
#define ADMIN1_ADD     FLASH_ADDR+1
#define ADMIN2_ADD     ADMIN1_ADD+12
#define ADMIN3_ADD     ADMIN2_ADD+12
#define ADMIN4_ADD     ADMIN3_ADD+12
#define ADMIN5_ADD     ADMIN4_ADD+12
#define ICCID_ADD      ADMIN5_ADD+12
#define REPLY_ADD      ICCID_ADD+21 

enum MemoryIndex
{
    REPLY_INDEX=0,
	  ADMIN1_INDEX,
    ADMIN2_INDEX,
	  ADMIN3_INDEX,
	  ADMIN4_INDEX,
	  ADMIN5_INDEX,
    ICCID_INDEX,
	
};


/*===========================================================================
 * Struct
 ===========================================================================*/

/*===========================================================================
 * Enum
 ===========================================================================*/

/*===========================================================================
 * Variate
 ===========================================================================*/
 
/*===========================================================================
 * Functions
 ===========================================================================*/

void AutoSwitch_embed_flash_app_init(void);



#ifdef __cplusplus
    } /*"C" */
#endif
    
#endif 
