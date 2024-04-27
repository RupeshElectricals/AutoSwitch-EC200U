/**  @file
  quec_boot_uart.h

  @brief
  This file is used to define boot uart api for different Quectel Project.

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
19/06/2021        Neo         Init version
=================================================================*/

#ifndef QUEC_BOOT_NAND_H
#define QUEC_BOOT_NAND_H

#include "quec_boot_pin_cfg.h"


#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================
 * Macro Definition
 ===========================================================================*/


/*===========================================================================
 * Enum
 ===========================================================================*/


/*===========================================================================
 * Functions declaration
 ===========================================================================*/

int quec_boot_nand_init(ql_boot_spi_port_e port);



#ifdef __cplusplus
} /*"C" */
#endif

#endif /* QUEC_BOOT_NAND_H */


