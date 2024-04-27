# Copyright (C) 2020 QUECTEL Technologies Limited and/or its affiliates("QUECTEL").
# All rights reserved.
#



set(target ql_app_autoswitch)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
include_directories(${SOURCE_TOP_DIR}/components/audio/include)
include_directories(${SOURCE_TOP_DIR}/components/ql-kernel/inc)
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PUBLIC inc)
target_link_libraries(${target} PRIVATE ql_ali_linkSDK)
#target_link_libraries(${target} PRIVATE ql_api_common)
#target_link_libraries(${target} PRIVATE cfw)
target_sources(${target} PRIVATE
	call.c
	sms.c
	Uart.c
	audio.c
	phonebook.c
	embedded_flash.c
	network.c
	mqtt.c
	fota_ftp.c
	power.c
	rtc.c
)

relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
