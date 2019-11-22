/*
 * gpio_key driver
 * Author: HoGC 
 */
#ifndef _GPIO_KEY_H_
#define _GPIO_KEY_H_

#include <stdio.h>
#include <c_types.h>
#include <gpio.h>
#include "os_type.h"
#include "osapi.h"

#define G_SET_BIT(a,b)                          (a |= (1 << b))
#define G_CLEAR_BIT(a,b)                        (a &= ~(1 << b))
#define G_IS_BIT_SET(a,b)                       (a & (1 << b))

#define KEY_TIMER_MS                            10                          ///< Defines the timing period required for the keypad module
#define KEY_MAX_NUMBER                          12                          ///< Maximum number of keys supported
#define DEBOUNCE_TIME                           30
#define PRESS_LONG_TIME                         3000

#define NO_KEY                                  0x0000
#define KEY_DOWN                                0x1000
#define KEY_UP     		                        0x2000
#define KEY_SHORT_UP                            0x4000
#define KEY_LONG                                0x8000

#define D0										16
#define D1                                      5
#define D2                                      4
#define D3                                      0
#define D4                                      2
#define D5                                      14
#define D6                                      12
#define D7                                      13
#define D8                                      15
#define D9                                      3
#define D10                                     1
#define RX                                      3
#define TX                                      1

#define M_D1                                    PERIPHS_IO_MUX_GPIO5_U
#define M_D2                                    PERIPHS_IO_MUX_GPIO4_U
#define M_D3                                    PERIPHS_IO_MUX_GPIO0_U
#define M_D4                                    PERIPHS_IO_MUX_GPIO2_U
#define M_D5                                    PERIPHS_IO_MUX_MTMS_U
#define M_D6                                    PERIPHS_IO_MUX_MTDI_U
#define M_D7                                    PERIPHS_IO_MUX_MTCK_U
#define M_D8                                    PERIPHS_IO_MUX_MTDO_U
#define M_D9                                    PERIPHS_IO_MUX_U0RXD_U
#define M_D10                                   PERIPHS_IO_MUX_U0TXD_U
#define M_RX                                    PERIPHS_IO_MUX_U0RXD_U
#define M_TX                                    PERIPHS_IO_MUX_U0TXD_U

#define F_D1                                    FUNC_GPIO5
#define F_D2                                    FUNC_GPIO4
#define F_D3                                    FUNC_GPIO0
#define F_D4                                    FUNC_GPIO2
#define F_D5                                    FUNC_GPIO14
#define F_D6                                    FUNC_GPIO12
#define F_D7                                    FUNC_GPIO13
#define F_D8                                    FUNC_GPIO15
#define F_D9                                    FUNC_GPIO3
#define F_D10                                   FUNC_GPIO1
#define F_RX                                    FUNC_GPIO3
#define F_TX                                    FUNC_GPIO1

typedef void (*gokit_key_function)(void);

typedef struct
{
    uint8 gpio_number; 
    uint8 gpio_id;
    uint8 gpio_func;
    uint32 gpio_name;
    gokit_key_function key_cb0;
    gokit_key_function key_cb1;
    uint8 release;
}key_typedef_t; 

typedef struct
{
    uint8 keyTotolNum;
    os_timer_t key_timer;
    uint8 key_timer_ms; 
    key_typedef_t ** singleKey; 
}keys_typedef_t; 

/* Function declaration */
void gokitKeyHandle(keys_typedef_t * keys); 
key_typedef_t * keyInitOne(uint8 gpio_id, uint32 gpio_name, uint8 gpio_func, gokit_key_function long_press, gokit_key_function short_press); 
key_typedef_t * ICACHE_FLASH_ATTR INTRInit_D(uint8 gpio_id,gokit_key_function INTR_press);
void keyParaInit(keys_typedef_t * keys);
void keySensorTest(void);

void gpio16_output_conf(void);
void gpio16_output_set(uint8 value);
void gpio16InputConf(void);
uint8 gpio16InputGet(void);

void ICACHE_FLASH_ATTR gpio_switch(uint8 gpio_id,uint32 *gpio_name,uint8 *gpio_func);

void ICACHE_FLASH_ATTR gpioInit(uint8 gpio_id);
void ICACHE_FLASH_ATTR gpio_out_init(uint8 gpio_id, bool bit_value);
void ICACHE_FLASH_ATTR gpio_disout_init(uint8 gpio_id);

void ICACHE_FLASH_ATTR gpio_wirte(uint8 gpio_id, bool bit_value);
uint32 ICACHE_FLASH_ATTR gpio_read(uint8 gpio_id);

void ICACHE_FLASH_ATTR set_key_num(u8 n);
void ICACHE_FLASH_ATTR key_add(uint8 gpio_id,gokit_key_function long_press, gokit_key_function short_press);
void ICACHE_FLASH_ATTR status_key_add(uint8 gpio_id,gokit_key_function key_down, gokit_key_function key_up);

#endif /*_GPIO_KEY_H*/

