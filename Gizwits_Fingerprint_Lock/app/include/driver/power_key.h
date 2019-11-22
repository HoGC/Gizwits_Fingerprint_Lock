/*
 * power_key driver
 * Author: HoGC 
 */
#ifndef _POWER_KEY_H_
#define _POWER_KEY_H_
#include "ets_sys.h"

#define POWER_KEY_FLASH  251
#define POWER_MAX_CONUT  3

typedef void (*power_key_cb_t)(void);
void power_flag_check(void);
void power_key_init(power_key_cb_t user_power_key_cb);


#endif 
