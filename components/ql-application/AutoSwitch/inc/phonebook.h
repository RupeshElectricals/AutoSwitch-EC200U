#ifndef PHONEBOOK_H
#define PHONEBOOK_H


#ifdef __cplusplus
extern "C" {
#endif


/*========================================================================
 *  Variable Definition
 *========================================================================*/
enum
{
    Admin1=0,
    Admin2,
    Admin3,
    Admin4,
    Admin5,
    ICCID,
};


/*========================================================================
 *  function Definition
 *========================================================================*/
void AutoSwitch_pbk_app_init(void);


#ifdef __cplusplus
} /*"C" */
#endif

#endif /* PHONEBOOK_H */