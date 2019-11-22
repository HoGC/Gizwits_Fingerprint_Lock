/*
 * lock driver
 * Author: HoGC 
 */
#include "ets_sys.h"
#include "osapi.h"
#include "driver/lock.h"

#define LOCK_DEBUG_ON

#if defined(LOCK_DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

os_timer_t Timer_lock_close;

lock_hender_t user_lock_hender = NULL;

#if PWM_LOCK

uint32 angle_max=10;
uint32 angle_min=0;
uint32 open_time=5;

os_timer_t Timer_pwm_stop;

void ICACHE_FLASH_ATTR seve_param(void){
    spi_flash_erase_sector(LOCK_FLASH);
    spi_flash_write(LOCK_FLASH * 4096, (uint32 *) &angle_max, 4);
    spi_flash_write(LOCK_FLASH * 4096+4, (uint32 *) &angle_min, 4);
    spi_flash_write(LOCK_FLASH * 4096+8, (uint32 *) &open_time, 4);
}

void ICACHE_FLASH_ATTR read_param(void){

    spi_flash_read(LOCK_FLASH*4096, (uint32 *) &angle_max, 4);
    if(angle_max>10||angle_max<0){
    	angle_max=10;
    }
    spi_flash_read(LOCK_FLASH*4096+4, (uint32 *) &angle_min, 4);
    if(angle_min>10||angle_min<0){
    	angle_min=0;
    }
    spi_flash_read(LOCK_FLASH*4096+8, (uint32 *) &open_time, 4);
    if(open_time>20||open_time<0){
    	open_time=5;
    }

    INFO("angle_max=%d\n",angle_max);
    INFO("angle_min=%d\n",angle_min);
    INFO("open_time=%d\n",open_time);
}
void ICACHE_FLASH_ATTR set_angle_max(u8 angle){
    if(0<=angle_max&&angle_max<=10){
    	angle_max=angle;
    	seve_param();
    }
}

void ICACHE_FLASH_ATTR set_angle_min(u8 angle){
    if(0<=angle_min&&angle_min<=10){
    	angle_min=angle;
    	seve_param();
    }
}

void ICACHE_FLASH_ATTR set_open_time(u8 time){
    if(0<time&&time<20){
    	open_time=time;
    	seve_param();
    }
}

void ICACHE_FLASH_ATTR pwm_stop(void){
    pwm_set_duty(0,0);
    pwm_start();
	gpio_wirte(LOCK_GPIO,1);
}

void ICACHE_FLASH_ATTR lock_pwm_init(void){

    uint32 gpio_name;
    uint8 gpio_fun;
    gpio_out_init(LOCK_GPIO,0);
    gpio_switch(LOCK_GPIO,&gpio_name,&gpio_fun);
    uint32 io_info[][3]={gpio_name,gpio_fun,LOCK_GPIO};
    pwm_init(100000,0,1,io_info);
    pwm_start();
    gpio_wirte(LOCK_GPIO,1);
}
#endif

void ICACHE_FLASH_ATTR lock_close(void){
    
    INFO("lock_close\r\n");

#if PWM_LOCK
    os_timer_disarm(&Timer_lock_close);
	currentDataPoint.valuelock=0;
	gizwitsHandle(&currentDataPoint);

    u32 duty;
    duty=2500+1000*angle_min;
    pwm_set_duty(duty,0);
    pwm_start();

	os_timer_disarm(&Timer_pwm_stop);
	os_timer_setfn(&Timer_pwm_stop, (os_timer_func_t *) pwm_stop, NULL);
	os_timer_arm(&Timer_pwm_stop, 1000, 0);
#else
    gpio_wirte(LOCK_GPIO, 0);
#endif

    if(user_lock_hender != NULL){
        user_lock_hender(0);
    }
}

void ICACHE_FLASH_ATTR lock_open(void){

    INFO("lock_open\r\n");

#if PWM_LOCK
    os_timer_disarm(&Timer_pwm_stop);

    u32 duty;
    duty=2500+1000*angle_max;
    pwm_set_duty(duty,0);
    pwm_start();

	os_timer_disarm(&Timer_lock_close);
	os_timer_setfn(&Timer_lock_close, (os_timer_func_t *) lock_close, NULL);
	os_timer_arm(&Timer_lock_close, open_time*1000, 0);
#else
    gpio_wirte(LOCK_GPIO, 1);
    os_timer_disarm(&Timer_lock_close);
	os_timer_setfn(&Timer_lock_close, (os_timer_func_t *) lock_close, NULL);
	os_timer_arm(&Timer_lock_close, 100, 0);
#endif

    if(user_lock_hender != NULL){
        user_lock_hender(1);
    }
}

void ICACHE_FLASH_ATTR lock_init(lock_hender_t lock_hender){

    INFO("lock_init\r\n");
    user_lock_hender = lock_hender;

#if PWM_LOCK
    //pwm初始化
    lock_pwm_init();
    //读取参数
    read_param();
#else
    gpio_out_init(LOCK_GPIO,0);
#endif
}


