/**  @file
  quec_virt_func_common.c

  @brief
  This file is used to run virt function.

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
22/06/2021        neo     Init version
=================================================================*/
#include "quec_cust_feature.h"
#include "stdbool.h"


#ifndef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
__attribute__((weak)) bool ql_get_spi6_ext_nor_status(void)
{
    return false;
}

__attribute__((weak)) void ql_set_spi6_ext_nor_flash_status(void)
{
    return ;
}

__attribute__((weak)) void quec_set_spi6_ext_nor_flash_capacity(void)
{
    return ;
}

__attribute__((weak)) int quec_spi6_ext_nor_flash_mount(bool format_on_fail)
{
    return 0;
}

__attribute__((weak)) int quec_spi6_ext_nor_flash_mkfs(void)
{
    return 0;
}

__attribute__((weak)) void quec_boot_spi6_ext_norflash_gpio_init(void)
{
    return ;
}

__attribute__((weak)) void quec_boot_get_spi6_ext_norflash_gpio_default_fuction(void)
{
    return ;
}

__attribute__((weak)) void quec_boot_spi6_ext_norflash_gpio_deinit(void)
{
    return ;
}

__attribute__((weak)) void quec_extflash_erase_count_update(unsigned int flash_addr, unsigned int size)
{
    return ;
}

#endif



