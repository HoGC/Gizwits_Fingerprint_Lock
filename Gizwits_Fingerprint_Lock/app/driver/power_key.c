/*
 * power_key driver
 * Author: HoGC 
 */
#include "osapi.h"
#include "ets_sys.h"
#include "user_interface.h"
#include "driver/power_key.h"

power_key_cb_t power_key_cb = NULL;
os_timer_t os_power_timer;

//#define POWER_KEY_DEBUG_ON

#if defined(POWER_KEY_DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

void ICACHE_FLASH_ATTR set_power_count(uint8 count) {
    uint32 power_count = (uint32)count;
    INFO("write_power_count:%d\n",power_count);
    spi_flash_erase_sector(POWER_KEY_FLASH);
    spi_flash_write(POWER_KEY_FLASH * 4096, (uint32 *) &power_count, 4);
}

LOCAL uint8 ICACHE_FLASH_ATTR get_power_count() {
    uint32 power_count;
    spi_flash_read(POWER_KEY_FLASH * 4096, (uint32 *) &power_count, 4);
    if (power_count < 0 || power_count > POWER_MAX_CONUT) {
        power_count = 1;
        spi_flash_erase_sector(POWER_KEY_FLASH);
        spi_flash_write(POWER_KEY_FLASH * 4096, (uint32 *) &power_count, 4);
    }
    return (uint8)power_count;
}

void power_flag_check(void) {

    uint8 power_count = 0;
    static u8 time = 0;
    time++;
    if (time == 1) {
        power_count = get_power_count();
        INFO("power_count:%d\n",power_count);
        if (power_count >= POWER_MAX_CONUT-1) {
            set_power_count(0);
            if(power_key_cb != NULL){
                power_key_cb();
            }
            os_timer_disarm(&os_power_timer);
        }else{
            set_power_count(++power_count);
        }
        
    } else if (time == 5) {
        set_power_count(0);
        os_timer_disarm(&os_power_timer);
    }
}

void power_key_init(power_key_cb_t user_power_key_cb){

    power_key_cb = user_power_key_cb;

    power_flag_check();

    os_timer_disarm(&os_power_timer);
    os_timer_setfn(&os_power_timer, (os_timer_func_t *) power_flag_check, NULL);
    os_timer_arm(&os_power_timer, 1000, true);
}
