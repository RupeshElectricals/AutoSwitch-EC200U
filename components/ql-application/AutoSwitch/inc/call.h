#ifndef __CALL_H__
#define __CALL_H__

//------------DTMF KEY------------------//
#define MOTOR_ONOFF_KEY   49  //1
#define MODE_SEL_KEY      50 //2
#define AUTO_CURR_KEY     51 //3

QlOSStatus AutoSwitch_call_app_init(void);
void DTMFKeyAction(char Key);

#endif//__CALL_H__