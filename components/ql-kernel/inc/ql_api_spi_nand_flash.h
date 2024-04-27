/**  @file
  ql_api_spi_nand_flash.h

  @brief
  This file is used to define bt api for different Quectel Project.

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


#ifndef QL_API_SPI_NAND_FLASH_H
#define QL_API_SPI_NAND_FLASH_H
#include "ql_api_common.h"
#include "ql_api_spi.h"
#include "ql_api_spi_flash.h"

#ifdef __cplusplus
extern "C" {
#endif


/*========================================================================
 *  Variable Definition
 *========================================================================*/
 
#define QL_SPI_NAND_PAGE_SIZE                       2048
#define QL_SPI_NAND_SPARE_SIZE                      64
#define QL_SPI_NAND_PAGE_SPARE_SIZE                 (QL_SPI_NAND_PAGE_SIZE+QL_SPI_NAND_SPARE_SIZE)

/****************************  error code about ql spi flash ***************************/
typedef ql_errcode_spi_flash_e ql_errcode_spi_nand_e ;
typedef ql_spi_flash_config_s ql_spi_nand_config_s ;

typedef enum
{
    QL_NAND_FLASH_STATUS_1 = 0,                     //Status Register-1
    QL_NAND_FLASH_STATUS_2,                         //Status Register-2
    QL_NAND_FLASH_STATUS_3,                         //Status Register-3
    QL_NAND_FLASH_STATUS_4,                         //Status Register-4
}ql_spi_nand_status_reg_e;

typedef enum
{
    QL_NAND_FLASH_ID_16BIT = 0,               //Manufacturer ID(8bit) + device ID(8bit)
    QL_NAND_FLASH_ID_24BIT,                   //Manufacturer ID(8bit) + device ID(16bit)
}ql_spi_nand_id_type_e;
/*========================================================================
 *  function Definition
 *========================================================================*/
/*****************************************************************
* Function: ql_spi_nand_init
*
* Description:
*   初始化NAND FLASH
* 
* Parameters:
*   port     [in]  SPI总线选择
*   spiclk   [in]  SPI时钟选择
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_init(ql_spi_port_e port, ql_spi_clk_e spiclk);

/*****************************************************************
* Function: ql_spi_nand_init_ext
*
* Description:
*   扩展初始化NAND FLASH，与ql_spi_nand_init有更多的设置选项
* 
* Parameters:
*   nand_config   [in]  SPI配置参数
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_init_ext(ql_spi_nand_config_s nand_config);

/*****************************************************************
* Function: ql_spi_nand_read_devid_ex
*
* Description:
*   read flash devie id
* 
* Parameters:
*   port     [in]  SPI bus select
*   mid      [out] manufacturer id
*   mid_type [in]  manufacturer id type
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_read_devid_ex(ql_spi_port_e port, unsigned  char *mid,ql_spi_nand_id_type_e mid_type);

/*****************************************************************
* Function: ql_spi_nand_read_devid
*
* Description:
*   read flash devie id
* 
* Parameters:
*   port     [in]  SPI bus select
*   mid      [out] manufacturer id
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_read_devid(ql_spi_port_e port, unsigned  char *mid);

/*****************************************************************
* Function: ql_spi_nand_read_page_spare
*
* Description:
*   read page(2048 byte)+spare area(64 byte)
* 
* Parameters:
*   port            [in]  SPI bus select
*   page_addr       [in]  page address
*   column_addr     [in]  column address
*   data            [out] data read buffer
*   len             [in] data read length
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_read_page_spare(ql_spi_port_e port, unsigned int page_addr, unsigned short column_addr, unsigned char *data, int len);

/*****************************************************************
* Function: ql_spi_nand_write_page_spare
*
* Description:
*   write page(2048 byte)+spare area(64 byte)
* 
* Parameters:
*   port            [in]  SPI bus select
*   page_addr       [in]  page address
*   column_addr     [in]  column address
*   data            [in] data read buffer
*   len             [in] data read length
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_write_page_spare(ql_spi_port_e port, unsigned int page_addr, unsigned short column_addr, unsigned char *data, int len);

/*****************************************************************
* Function: ql_spi_nand_write_spare
*
* Description:
*   write spare area(64 byte)
* 
* Parameters:
*   port            [in]  SPI bus select
*   page_addr       [in]  page address
*   data            [out] data read buffer
*   len             [in] data read length
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_write_spare(ql_spi_port_e port, unsigned int page_addr, unsigned char *data, int len);

/*****************************************************************
* Function: ql_spi_nand_read_status
*
* Description:
*   read status register value
* 
* Parameters:
*   port            [in]  SPI bus select
*   reg             [in]  FLASH status register
*   status          [out] status register value
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_read_status(ql_spi_port_e port, ql_spi_nand_status_reg_e reg, unsigned char *status);

/*****************************************************************
* Function: ql_spi_nand_write_status
*
* Description:
*   read status register value
* 
* Parameters:
*   port            [in]  SPI bus select
*   reg             [in]  FLASH status register
*   status          [in]  status register value
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_write_status(ql_spi_port_e port, ql_spi_nand_status_reg_e reg, unsigned char status);

/*****************************************************************
* Function: ql_spi_nand_erase_block
*
* Description:
*   erase block(128k):D8H
* 
* Parameters:
*   port            [in]  SPI bus selelct
*   page_addr       [in]  page address
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_erase_block(ql_spi_port_e port, unsigned int page_addr);

/*****************************************************************
* Function: ql_spi_nand_reset
*
* Description:
*   reset flash device:FFH
* 
* Parameters:
*   port            [in]  SPI bus selelct
*
* Return:ql_errcode_spi_nand_e
*
*****************************************************************/
ql_errcode_spi_nand_e ql_spi_nand_reset(ql_spi_port_e port);

#ifdef __cplusplus
} /*"C" */
#endif

#endif /* QL_API_SPI_NAND_FLASH_H */



