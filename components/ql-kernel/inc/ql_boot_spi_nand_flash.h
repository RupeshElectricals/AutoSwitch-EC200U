/**  @file
  ql_boot_spi_nand_flash.h

  @brief
  This file is used to define boot spi nand flash api for different Quectel Project.

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


#ifndef QL_BOOT_SPI_NAND_FLASH_H
#define QL_BOOT_SPI_NAND_FLASH_H
#include "ql_api_common.h"
#include "quec_boot_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                        spi nand flash api接口
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*========================================================================
 *  Variable Definition
 *========================================================================*/
 
#define QL_BOOT_SPI_NAND_PAGE_SIZE                       2048
#define QL_BOOT_SPI_NAND_SPARE_SIZE                      64
#define QL_BOOT_SPI_NAND_PAGE_SPARE_SIZE                 (QL_BOOT_SPI_NAND_PAGE_SIZE+QL_BOOT_SPI_NAND_SPARE_SIZE)

typedef ql_boot_spi_flash_config_s ql_boot_spi_nand_config_s ;

typedef enum
{
    QL_BOOT_NAND_FLASH_STATUS_1 = 0,                     //Status Register-1
    QL_BOOT_NAND_FLASH_STATUS_2,                         //Status Register-2
    QL_BOOT_NAND_FLASH_STATUS_3,                         //Status Register-3
    QL_BOOT_NAND_FLASH_STATUS_4,                         //Status Register-4
}ql_boot_spi_nand_status_reg_e;

typedef enum
{
    QL_BOOT_NAND_FLASH_ID_16BIT = 0,                    //Manufacturer ID(8bit) + device ID(8bit)
    QL_BOOT_NAND_FLASH_ID_24BIT,                        //Manufacturer ID(8bit) + device ID(16bit)
}ql_boot_spi_nand_id_type_e;

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
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_init(ql_boot_spi_port_e port, ql_boot_spi_clk_e spiclk);

/*****************************************************************
* Function: ql_spi_nand_init_ext
*
* Description:
*   扩展初始化NAND FLASH，与ql_spi_nand_init有更多的设置选项
* 
* Parameters:
*   nand_config   [in]  SPI配置参数
*
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_init_ext(ql_boot_spi_nand_config_s nand_config);

/*****************************************************************
* Function: ql_boot_spi_nand_read_devid_ex
*
* Description:
*   read flash devie id
* 
* Parameters:
*   port     [in]  SPI bus select
*   mid      [out] manufacturer id
*   mid_type [out] manufacturer id type
*
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_read_devid_ex(ql_boot_spi_port_e port, unsigned  char *mid,ql_boot_spi_nand_id_type_e mid_type);

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
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
//ql_boot_errcode_spi_e ql_boot_spi_nand_read_devid(ql_boot_spi_port_e port, unsigned  char *mid);
#define ql_boot_spi_nand_read_devid(port, mid)  ql_boot_spi_nand_read_devid_ex(port, mid, QL_BOOT_NAND_FLASH_ID_24BIT)

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
*   len             [out] data read length
*
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_read_page_spare(ql_boot_spi_port_e port, unsigned int page_addr, unsigned short column_addr, unsigned char *data, int len);

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
*   data            [out] data read buffer
*   len             [out] data read length
*
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_write_page_spare(ql_boot_spi_port_e port, unsigned int page_addr, unsigned short column_addr, unsigned char *data, int len);

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
*   len             [out] data read length
*
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_write_spare(ql_boot_spi_port_e port, unsigned int page_addr, unsigned char *data, int len);

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
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_read_status(ql_boot_spi_port_e port, ql_boot_spi_nand_status_reg_e reg, unsigned char *status);

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
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_write_status(ql_boot_spi_port_e port, ql_boot_spi_nand_status_reg_e reg, unsigned char status);

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
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_erase_block(ql_boot_spi_port_e port, unsigned int page_addr);

/*****************************************************************
* Function: ql_spi_nand_reset
*
* Description:
*   reset flash device:FFH
* 
* Parameters:
*   port            [in]  SPI bus selelct
*
* Return:ql_boot_errcode_spi_e
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_spi_nand_reset(ql_boot_spi_port_e port);



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                          spi nand flash demo 实列
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*===========================================================================
 * Macro Definition
 *===========================================================================*/


typedef enum
{
    QL_BOOT_ECC_OK,
    QL_BOOT_ECC_1BIT,
    QL_BOOT_ECC_2BIT
}ql_boot_nand_ecc_status_e;
    
    
typedef struct
{
    unsigned int data_blocknum;
    unsigned int free_blocknum;
    unsigned int nand_id;
    unsigned int *logical_map_table;
    unsigned char *pbuffer;
}ql_boot_nand_config_s;


typedef struct
{
    unsigned int acture_block_num;
    unsigned short acture_page_num;
    unsigned int cache_block_num;
    unsigned short cache_page_num;
}ql_boot_nand_cache_page_s;

typedef struct
{
	unsigned int page_mainsize;
	unsigned int page_sparesize;
	unsigned int block_pagenum;
	unsigned int plane_blocknum;	//not use
	unsigned int block_totalnum;
	unsigned int cache_blocknum;
	unsigned int cache_total_pagenum;

	unsigned short page_spare_shift;
	unsigned short block_postion_shift;
	unsigned short block_type_shift;
	unsigned short logic_addr_shift; //offset in page
    unsigned short logic_addr_addr;  //addr in page,4byte
    unsigned short page_used_shift;  //offset in page
    unsigned short page_used_addr;   //addr in page,1byte
    unsigned short page_garbage_shift;
    unsigned short page_num_shift;
} ql_boot_nand_spec_s;

/*****************************************************************
* Function: ql_nand_init
*
* Description: nand flash init
*
* Parameters:
*   NULL
*
* Return:
*   QL_BOOT_SPI_SUCCESS     success
*	other 	                error code
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_nand_init();

/*****************************************************************
* Function: ql_nand_uninit
*
* Description: nand flash uninit
*
* Parameters:
*   NULL
*
* Return:
*   QL_BOOT_SPI_SUCCESS     success
*	other 	                error code
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_nand_uninit();

/*****************************************************************
* Function: ql_nand_read_page
*
* Description: read page
*
* Parameters:
*   block_num      [in]     physical block number 
*   page_num       [in]     page number 
*   column_addr    [in]     column address
*   pbuffer        [out]    data buffer
*   len            [in]     data len
*
* Return:
*   QL_BOOT_SPI_SUCCESS     success
*	other 	                error code
*
*****************************************************************/
ql_boot_errcode_spi_e ql_boot_nand_read_page(unsigned      int block_num, unsigned short page_num, unsigned short column_addr, unsigned char *pbuffer, int len);


#ifdef __cplusplus
} /*"C" */
#endif

#endif /* QL_API_SPI_NAND_FLASH_H */



