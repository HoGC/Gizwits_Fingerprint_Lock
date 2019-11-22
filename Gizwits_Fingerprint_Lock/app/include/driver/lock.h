/*
 * lock driver
 * Author: HoGC 
 */
#ifndef _LOCK_H_
#define _LOCK_H_

#include "driver/gpio_key.h"
#include "user_interface.h"
#include "gizwits_product.h"

// 1: 使用舵机实现拉动门锁   0：使用脉冲式触发电磁锁
#define PWM_LOCK    1

#define LOCK_GPIO 	D3

typedef enum {
    LOCK_CLOSE = 0x00,
    LOCK_OPEN
} lock_status;

typedef void (*lock_hender_t)(lock_status status);

void ICACHE_FLASH_ATTR lock_init(lock_hender_t lock_hender);
void ICACHE_FLASH_ATTR lock_open(void);
void ICACHE_FLASH_ATTR lock_close(void);


#if PWM_LOCK

#define LOCK_FLASH  250

void ICACHE_FLASH_ATTR set_angle_max(u8 angle);
void ICACHE_FLASH_ATTR set_angle_min(u8 angle);
void ICACHE_FLASH_ATTR set_open_time(u8 time);

#endif

#endif /* _LOCK_H_ */