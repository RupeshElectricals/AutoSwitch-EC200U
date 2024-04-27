/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "usb_demo.h"
#include "ql_power.h"

#define QL_USB_LOG_LEVEL	            QL_LOG_LEVEL_INFO
#define QL_USB_LOG(msg, ...)			QL_LOG(QL_USB_LOG_LEVEL, "ql_usb", msg, ##__VA_ARGS__)
#define QL_USB_LOG_PUSH(msg, ...)		QL_LOG_PUSH("ql_usb", msg, ##__VA_ARGS__)

#define QL_USB_MASS_STORAGE	1 //验证mass storage功能打开此宏

static void ql_usb_demo_thread(void *param)
{
	QL_USB_LOG("enter usb demo");

#if QL_USB_MASS_STORAGE
	ql_usb_msc_cfg_t msc_cfg = {0};
/*
	配置映射模块的内置flash/sd卡/6线flash等存储器到PC端, 模块作为虚拟U盘
	注意：
		  1. 在target.config中,打开CONFIG_QUEC_PROJECT_FEATURE_USB_MASS_STORAGE宏
		  2. protocol参数中支持MTP和MSG两种协议,差异详见ql_usb_protocol_e定义处
		  3. 烧录代码后第一次开机进app可能会比较慢,如果sd卡/外置flash在app挂载,则烧录后第一次开机可能虚拟不出sd卡/外置flash,后续开机不会无法映射
*/
	msc_cfg.msc_device = QL_USB_MSC_UFS; 		//如需映射2个存储器,可用"|"连接,如同时映射外置6线flash和sd卡: QL_USB_MSC_EFS | QL_USB_MSC_SDCARD
	msc_cfg.protocol = QL_USB_PROTOCOL_MTP;			//QL_USB_PROTOCOL_MSG传输速率更快,但支持的功能少一点,详见ql_usb_protocol_e定义处的备注

/*
	1. PC上显示的磁盘名称,用户可以自己定义,长度不超过15字节,仅MTP协议下支持; 默认显示"ANDROID"
	2. 注意: 已经加载过一次虚拟U盘的PC,如果要修改msc_cfg.dev_name,需要卸载重装一次QUECTEL的USB驱动,因为windows系统会记住第一次识别到的设备名称,只有
	         卸载重装一次USB驱动才会去刷新虚拟U盘设备名称;
*/
	strcpy(msc_cfg.dev_name, "ANDROID");
	if(ql_usb_get_enum_mode()!=QL_USB_ENUM_MASS_STORAGE)	
	{		
		ql_usb_set_enum_mode(QL_USB_ENUM_MASS_STORAGE); //重启生效
		ql_usb_msc_config_set(&msc_cfg); //重启生效
		ql_rtos_task_sleep_s(2);
		ql_power_reset(RESET_NORMAL);
	}
#endif

/*
	使用默认的端口组合,配置枚举哪些USB端口,也可以直接在quec_usb_serial_create中配置,该函数开源在app_start.c中
	ql_usb_set_enum_mode(QL_USB_ENUM_USBNET_COM);
*/
	ql_rtos_task_delete(NULL);
}

void ql_usb_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	ql_task_t ql_usb_task = NULL;
	
    QL_USB_LOG("usb demo enter");
	
    err = ql_rtos_task_create(&ql_usb_task, QL_USB_TASK_STACK, APP_PRIORITY_NORMAL, "ql_usb_demo", ql_usb_demo_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_USB_LOG("usb task create failed");
	}
}


