#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "gagent_soc.h"
#include "user_devicefind.h"
#include "user_webserver.h"
#include "gizwits_product.h"
#include "driver/lock.h"
#include "driver/gpio_key.h"
#include "driver/power_key.h"
#include "driver/fprint1016.h"
#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

#ifdef SERVER_SSL_ENABLE
#include "ssl/cert.h"
#include "ssl/private_key.h"
#else
#ifdef CLIENT_SSL_ENABLE
unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;
#endif
#endif


//#define MAIN_DEBUG_ON

#if defined(MAIN_DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

/**
 * 按键长按触发回调
 * 连续上电三次触发回调
*/
void ICACHE_FLASH_ATTR key_cb(void)
{   
    gizwitsSetMode(WIFI_AIRLINK_MODE);      //进入配网模式
    gpio_wirte(D4,0);                       //点亮esp8266LED
}

//按键初始化
void ICACHE_FLASH_ATTR keyInit(void)
{
	set_key_num(1);                         //设置按键数量
	key_add(D2, key_cb, NULL);              //长按、短按的按键回调
}

/**
 * Lock事件处理
 */
void ICACHE_FLASH_ATTR lock_hender(lock_status status){
    switch (status)
    {
        //已开锁
        case LOCK_OPEN:
            //更新数据点
            currentDataPoint.valuelock=1;
            gizwitsHandle(&currentDataPoint);
            break;
        //已关锁
        case LOCK_CLOSE:
            //更新数据点
            currentDataPoint.valuelock=0;
            gizwitsHandle(&currentDataPoint);
            break;
        
        default:
            break;
    }
}

/**
 * 指纹事件处理
 */
void ICACHE_FLASH_ATTR fprint_hender(FprintStatus fprintStatus){
    
    switch (fprintStatus.mode)
    {
        //验证指纹
        case VERIFY_MODE:
            if(fprintStatus.status){                            //验证成功
                INFO("verify_success id:%d",fprintStatus.id);   //指纹ID
                gizwitsPassthroughData(&fprintStatus.id,1);     //数据透传发送到APP
                lock_open();
            }
            else{                                               //验证失败
                INFO("verify_error");
            }
            break;
        //录入指纹
        case REGISTER_MODE:
            if(fprintStatus.status){                             //注册成功
                INFO("register_success id:%d",fprintStatus.id);  //指纹ID
                gizwitsPassthroughData(&fprintStatus.id,1);      //数据透传发送到APP
            }else{                                               //注册失败
                INFO("register_error");
            }
        //删除指纹
        case DELETE_MODE:
            if(fprintStatus.status){                             //注册成功
                INFO("register_success");  //指纹ID
            }else{                                               //注册失败
                INFO("register_error");
            }
        
        default:
            break;
    }
}



uint32_t ICACHE_FLASH_ATTR user_rf_cal_sector_set()
{
    return 636;
}

void ICACHE_FLASH_ATTR user_init(void)
{
    wifi_station_set_auto_connect(1);
    wifi_set_sleep_type(NONE_SLEEP_T);
    espconn_tcp_set_max_con(10);
	uart_init(115200, 115200);
	os_delay_us(60000);

    gpio_out_init(D4,1);                //初始化esp8266LED

    gizwitsInit();                      //机智云初始化
    
    keyInit();                          //按键初始化
    power_key_init(key_cb);             //设置连续上电三次触发回调
    
    lock_init(lock_hender);             //Lock初始化

    fprint1016_init(fprint_hender);     //指纹检测初始化
}