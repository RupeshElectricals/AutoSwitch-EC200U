#ifndef __UART_H__
#define __UART_H__

#define QL_UART_RX_BUFF_SIZE                1024
#define QL_UART_TX_BUFF_SIZE                1024

//--------------------DEBUG MESSAGE DEFINATION-------------//
#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DBG_BUF_LEN   QL_UART_TX_BUFF_SIZE

#define APP_DEBUG(FORMAT,...) {\
    memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    ql_uart_write(DEBUG_UART, (u8*)(DBG_BUFFER), strlen((const char *)(DBG_BUFFER)));\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif
//----------------------------------------------------------------//

//--------------------MCU MESSAGE DEFINATION-------------//
#define MCU_ENABLE 1
#if MCU_ENABLE > 0
#define MCU_BUF_LEN   QL_UART_TX_BUFF_SIZE

#define MCU_UART_WRITE(FORMAT,...) {\
    memset(MCU_BUFFER, 0, MCU_BUF_LEN);\
    sprintf(MCU_BUFFER,FORMAT,##__VA_ARGS__); \
    ql_uart_write(MCU_UART, (u8*)(MCU_BUFFER), strlen((const char *)(MCU_BUFFER)));\
}
#else
#define MCU_UART_WRITE(FORMAT,...) 
#endif
//----------------------------------------------------------------//

void AutoSwitch_Uart_app_init(void);
void McuUartHandle( char *Data);

#endif//__UART_H__
