/*
 * ota driver
 * Author: HoGC 
 */
#ifndef _FPRINT1016_H_
#define _FPRINT1016_H_

#include "driver/gpio_key.h"

#define WAKEUP_GPIO     D1

#define CMD_PACKET_LEN 26
//通讯命令
#define CMD_TEST_CONNECTION      0x01  //测试连接
#define CMD_GET_IMAGE            0x20  //从传感器采集图像并保存于ImageBuffer中
#define CMD_FINGER_DETECT        0x21  //检测指纹输入状态
#define CMD_SLED_CTRL            0x24  //设置LED
#define CMD_STORE_CHAR           0x40  //将指定编号 Ram Buffer 中的 Template， 注册到指定编号的库中
#define CMD_LOAD_CHAR            0x41  //读取库中指定编号中的 Template 到指定编号的 Ram Buffer
#define CMD_DEL_CHAR             0x44  //删除指定编号范围内的 Template
#define CMD_GET_EMPTY_ID         0x45  //获取指定范围内可注册的（没有注册的） 第一个模板编号
#define CMD_GET_STATUS           0x46  //获取指定编号的模板注册状态
#define CMD_GET_BROKEN_ID        0x47  //检查指定编号范围内的所有指纹模板是否存在坏损的情况
#define CMD_GET_ENROLL_COUNT     0x48  //获取指定编号范围内已注册的模板个数
#define CMD_GENERATE             0x60  //将 ImageBuffer 中的指纹图像生成模板数据，并保存于指定编号的 Ram Buffer 中
#define CMD_MERGE                0x61  //将保存于 Ram Buffer 中的两或三个模板数据融合成一个模板数据
#define CMD_MATCH                0x62  //指定 Ram Buffer 中的两个指纹模板之间进行 1:1 比对
#define CMD_SEARCH               0x63  //指定 Ram Buffer 中的模板与指纹库中指定编号范围内的所有模板之间进行 1:N 比对
#define CMD_VERIFY               0x64  //指定 Ram Buffer 中的指纹模板与指纹库中指定编号的指纹模板之间进行 1:1 比对
#define CMD_SET_MODULE_SN        0x08  //在设备中设置模块序列号信息（Module SN）
#define CMD_GET_MODULE_SN        0x09  //获取本设备的模块序列号（Module SN）
#define CMD_GET_ENROLLED_ID_LIST 0x49  //获取已注册 User ID 列表
#define CMD_ENTER_STANDY_STATE   0x0C  //使模块进入休眠状态


//响应指令
#define ERR_SUCCESS             0x00  //指令处理成功
#define ERR_FAIL                0x01  //指令处理失败
#define ERR_VERIFY              0x10  //与指定编号中 Template 的 1:1 比对失败
#define ERR_IDENTIFY            0x11  //已进行 1:N 比对， 但相同 Template 不存在
#define ERR_TMPL_EMPTY          0x12  //在指定编号中不存在已注册的 Template
#define ERR_TMPL_NOT_EMPTY      0x13  //在指定编号中已存在 Template 
#define ERR_ALL_TMPL_EMPTY      0x14  //不存在已注册的 Template
#define ERR_EMPTY_ID_NOEXIST    0x15  //不存在可注册的 Template ID 
#define ERR_BROKEN_ID_NOEXIST   0x16  //不存在已损坏的 Template
#define ERR_INVALID_TMPL_DATA   0x17  //指定的 Template Data 无效
#define ERR_DUPLICATION_ID      0x18  //该指纹已注册
#define ERR_BAD_QUALITY         0x19  //指纹图像质量不好
#define ERR_MERGE_FAIL          0x1A  //Template 合成失败
#define ERR_INVALID_TMPL_NO     0x1D  //指定 Template 编号无效
#define ERR_INVALID_PARAM       0X22  //使用了不正确的参数
#define ERR_GEN_COUNT           0X25  //指纹合成个数无效
#define ERR_INVALID_BUFFER_ID   0X26  //Buffer ID 值不正确
#define ERR_FP_NOT_DETECTED     0X28  //采集器上没有指纹输入
#define ERR_FP_CANCEL           0x41  //指令被取消


typedef enum {
    LED_OFF     = 0x00,
    LED_ON      = 0x01,
    LED_BLN     = 0x02,
    LED_BLINK_S = 0x03,
    LED_BLINK_F = 0x04
} led_mode;

typedef enum {
    LED_COLOR_BLUE   = 0X81,
    LED_COLOR_RED    = 0X82,
    LED_COLOR_GREEN  = 0X84,
    LED_COLOR_CYAN   = 0X85,
    LED_COLOR_YELLOW = 0X86
} led_color;

typedef enum {
    VERIFY_MODE = 0x00,
    REGISTER_MODE,
    DELETE_MODE
} FprintMode;

typedef struct
{
    u8 CMD;
    u8 dataLen;
    u8 data[14];
}DataPacket;

typedef struct
{
    FprintMode mode;
    bool status;
    u8 id;
}FprintStatus;

typedef void (*fprint_hender_t)(FprintStatus fprintStatus);

void ICACHE_FLASH_ATTR fprint1016_init(fprint_hender_t fprint_hender);

//LED控制
void ICACHE_FLASH_ATTR fp_led_close();   //关闭所有LED
void ICACHE_FLASH_ATTR fp_led_set(led_color color, led_mode status);

//切换模式
void ICACHE_FLASH_ATTR fp_delete(u8 ID);  //删除指纹
void ICACHE_FLASH_ATTR fp_register();     //注册指纹
void ICACHE_FLASH_ATTR fp_verify();       //验证指纹

#endif