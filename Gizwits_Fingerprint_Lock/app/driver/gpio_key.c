/*
 * gpio_key driver
 * Author: HoGC 
 */
#include "driver/gpio_key.h"
#include "mem.h"

//#define GPIO_KEY_DEBUG_ON

#if defined(GPIO_KEY_DEBUG_ON)
#define KEY_INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define KEY_INFO( format, ... )
#endif


uint32 keyCountTime = 0;
static int8_t keyTotolNum = 0;
static uint8_t key_num = 0;
bool longinit = 0;

LOCAL key_typedef_t * singleKey[14];
LOCAL keys_typedef_t keys;

/**
 * @brief Read the GPIO state
 * @param [in] keys Key Function Global structure pointer
 * @return uint16_t type GPIO status value
 */
static ICACHE_FLASH_ATTR uint16_t keyValueRead(keys_typedef_t * keys) {
	uint8_t i = 0;
	uint16_t read_key = 0;

	//GPIO Cyclic scan
	for (i = 0; i < keys->keyTotolNum; i++) {
		if(keys->singleKey[i]->gpio_id == D0){
			if(!gpio16InputGet()){
				G_SET_BIT(read_key, keys->singleKey[i]->gpio_number);
			}
		}else if(!GPIO_INPUT_GET(keys->singleKey[i]->gpio_id)) {
			G_SET_BIT(read_key, keys->singleKey[i]->gpio_number);
		}
	}

	return read_key;
}

/**
 * @brief Read the KEY value
 * @param [in] keys Key Function Global structure pointer
 * @return uint16_t type key state value
 */
static uint16_t ICACHE_FLASH_ATTR keyStateRead(keys_typedef_t * keys) {
	static uint8_t Key_Check = 0;
	static uint8_t Key_State = 0;
	static uint16_t Key_LongCheck = 0;
	uint16_t Key_press = 0;
	uint16_t Key_return = 0;
	static uint16_t Key_Prev = 0;

	//Accumulate key time
	keyCountTime++;
	//Press to shake 30MS
	if (keyCountTime >= (DEBOUNCE_TIME / keys->key_timer_ms)) {
		keyCountTime = 0;
		Key_Check = 1;
	}

	if (Key_Check == 1) {
		Key_Check = 0;

		//Gets the current key trigger value
		Key_press = keyValueRead(keys);

		switch (Key_State) {
		//"First capture key" state
		case 0:

			if (Key_press != 0) {
				Key_Prev = Key_press;
				Key_State = 1;
			}

			break;

			//"Capture valid key" status
		case 1:
			if (Key_press == Key_Prev) {
				Key_State = 2;
				Key_return = Key_Prev | KEY_DOWN;
				KEY_INFO("KEY_DOWN\n");
				return Key_return;
			} else {
				//Button lift, jitter, no response button
				Key_State = 0;
			}
			break;

			//"Capture long press" status
		case 2:

			if (Key_press != Key_Prev) {
				Key_State = 0;
				Key_LongCheck = 0;
				Key_return = Key_Prev | KEY_SHORT_UP;
				KEY_INFO("KEY_SHORT_OR_UP\n");
				return Key_return;
			}

			if (Key_press == Key_Prev) {
				Key_LongCheck++;
				if (Key_LongCheck >= (PRESS_LONG_TIME / DEBOUNCE_TIME)){
					Key_LongCheck = 0;
					Key_State = 3;
					Key_return = Key_press | KEY_LONG;
					KEY_INFO("KEY_LONG\n");
					return Key_return;
				}
			}
			break;

			//"Restore the initial" state
		case 3:
			if (Key_press != Key_Prev) {
				Key_State = 0;
				Key_return = Key_Prev | KEY_UP;
				KEY_INFO("KEY_LONG_UP\n");
				return Key_return;
			}
			break;
		}
	}
	return NO_KEY;
}

/**
 * @brief button callback function
 * Call the corresponding callback function after completing the key state monitoring in the function
 * @param [in] keys Key Function Global structure pointer
 * @return none
 */
void ICACHE_FLASH_ATTR gokitKeyHandle(keys_typedef_t * keys) {
	uint8_t i = 0;
	uint16_t key_value = 0;

	key_value = keyStateRead(keys);

	if (!key_value)
		return;

	//Check short press button
	if (key_value & KEY_SHORT_UP) {
		//Valid key is detected
		for (i = 0; i < keys->keyTotolNum; i++) {
			if (G_IS_BIT_SET(key_value, keys->singleKey[i]->gpio_number)) {
				//key callback function of short press
				if (keys->singleKey[i]->key_cb1) {
					keys->singleKey[i]->key_cb1();
					if (keys->singleKey[i]->release == 0) {
						KEY_INFO("[zs] callback short key: [%d][%d] \r\n",
								keys->singleKey[i]->gpio_id,
								keys->singleKey[i]->gpio_number);
					} else if (keys->singleKey[i]->release == 1) {
						KEY_INFO("[zs] callback up key: [%d][%d] \r\n",
								keys->singleKey[i]->gpio_id,
								keys->singleKey[i]->gpio_number);
					}
				}
			}
		}
	}

	//Check short long button
	if (key_value & KEY_LONG) {
		//Valid key is detected
		for (i = 0; i < keys->keyTotolNum; i++) {
			if (keys->singleKey[i]->release == 0) {
				if (G_IS_BIT_SET(key_value, keys->singleKey[i]->gpio_number)) {
					//key callback function of long press
					if (keys->singleKey[i]->key_cb0) {
						keys->singleKey[i]->key_cb0();

						KEY_INFO("[zs] callback long key: [%d][%d] \r\n",
								keys->singleKey[i]->gpio_id,
								keys->singleKey[i]->gpio_number);
					}
				}
			}
		}
	}

	if (key_value & KEY_DOWN) {
		//Valid key is detected
		for (i = 0; i < keys->keyTotolNum; i++) {
			if (keys->singleKey[i]->release == 1) {
				if (G_IS_BIT_SET(key_value, keys->singleKey[i]->gpio_number)) {
					//key callback function of short press
					if (keys->singleKey[i]->key_cb0) {
						keys->singleKey[i]->key_cb0();

						KEY_INFO("[zs] callback down key: [%d][%d] \r\n",
								keys->singleKey[i]->gpio_id,
								keys->singleKey[i]->gpio_number);
					}
				}
			}
		}
	}

	if (key_value & KEY_UP) {
		//Valid key is detected
		for (i = 0; i < keys->keyTotolNum; i++) {
			if (keys->singleKey[i]->release == 1) {
				if (G_IS_BIT_SET(key_value, keys->singleKey[i]->gpio_number)) {
					//key callback function of short press
					if (keys->singleKey[i]->key_cb1) {
						keys->singleKey[i]->key_cb1();

						KEY_INFO("[zs] callback up key: [%d][%d] \r\n",
								keys->singleKey[i]->gpio_id,
								keys->singleKey[i]->gpio_number);
					}
				}
			}
		}
	}
}

/**
 * @brief single button initialization
 * In this function to complete a single key initialization, here need to combine the ESP8266 GPIO register description document to set the parameters
 * @param [in] gpio_id ESP8266 GPIO number
 * @param [in] gpio_name ESP8266 GPIO name
 * @param [in] gpio_func ESP8266 GPIO function
 * @param [in] long_press Long press the callback function address
 * @param [in] short_press Short press state callback function address
 * @return single-button structure pointer
 */
key_typedef_t * ICACHE_FLASH_ATTR keyInitOne(uint8 gpio_id, uint32 gpio_name,
		uint8 gpio_func, gokit_key_function long_press,
		gokit_key_function short_press) {

	key_typedef_t * singleKey = (key_typedef_t *) os_zalloc(
			sizeof(key_typedef_t));

	singleKey->gpio_number = keyTotolNum;

	//Platform-defined GPIO
	singleKey->gpio_id = gpio_id;
	singleKey->gpio_name = gpio_name;
	singleKey->gpio_func = gpio_func;
	singleKey->release = 0;
	//Button trigger callback type
	singleKey->key_cb0 = long_press;
	singleKey->key_cb1 = short_press;
	if (long_press == NULL) {
		longinit = 0;
	} else {
		longinit = 1;
	}

	keyTotolNum++;

	return singleKey;
}

/**
 * @brief button driver initialization

 * In the function to complete all the keys GPIO initialization, and open a timer to start the key state monitoring
 * @param [in] keys Key Function Global structure pointer
 * @return none
 */
void ICACHE_FLASH_ATTR keyParaInit(keys_typedef_t * keys) {
	uint8 tem_i = 0;

	if (NULL == keys) {
		return;
	}

	//init key timer
	keys->key_timer_ms = KEY_TIMER_MS;
	os_timer_disarm(&keys->key_timer);
	os_timer_setfn(&keys->key_timer, (os_timer_func_t *) gokitKeyHandle, keys);

	keys->keyTotolNum = keyTotolNum;

	//Limit on the number keys (Allowable number: 0~12)
	if (KEY_MAX_NUMBER < keys->keyTotolNum) {
		keys->keyTotolNum = KEY_MAX_NUMBER;
	}

	//GPIO configured as a high level input mode
	for (tem_i = 0; tem_i < keys->keyTotolNum; tem_i++) {
		if (keys->singleKey[tem_i]->gpio_id == D0) {
			gpio16_output_conf();
			gpio16_output_set(1);
			KEY_INFO("gpio_name %d \r\n", keys->singleKey[tem_i]->gpio_id);
		} else {
			PIN_FUNC_SELECT(keys->singleKey[tem_i]->gpio_name,
					keys->singleKey[tem_i]->gpio_func);
			GPIO_OUTPUT_SET(GPIO_ID_PIN(keys->singleKey[tem_i]->gpio_id), 1);
			PIN_PULLUP_EN(keys->singleKey[tem_i]->gpio_name);
			GPIO_DIS_OUTPUT(GPIO_ID_PIN(keys->singleKey[tem_i]->gpio_id));

			KEY_INFO("gpio_name %d \r\n", keys->singleKey[tem_i]->gpio_id);
		}
	}

	//key timer start
	os_timer_arm(&keys->key_timer, keys->key_timer_ms, 1);
}

void ICACHE_FLASH_ATTR
gpio16_output_conf(void) {
	WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
			(READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); // mux configuration for XPD_DCDC to output rtc_gpio0

	WRITE_PERI_REG(RTC_GPIO_CONF,
			(READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

	WRITE_PERI_REG(RTC_GPIO_ENABLE,
			(READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe) | (uint32)0x1);//out enable
}

void ICACHE_FLASH_ATTR
gpio16_output_set(uint8 value) {
	WRITE_PERI_REG(RTC_GPIO_OUT,
			(READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe) | (uint32)(value & 1));
}

void ICACHE_FLASH_ATTR
gpio16InputConf(void) {
	WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
			(READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); // mux configuration for XPD_DCDC and rtc_gpio0 connection

	WRITE_PERI_REG(RTC_GPIO_CONF,
			(READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

	WRITE_PERI_REG(RTC_GPIO_ENABLE,
			READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe);//out disable
}

uint8 ICACHE_FLASH_ATTR
gpio16InputGet(void) {
	return (uint8) (READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
}

//gpio初始化
void ICACHE_FLASH_ATTR gpioInit(uint8 gpio_id) {
	uint32 gpio_name;
	uint8 gpio_func;
	if(gpio_id == D0)
	{
		gpio16_output_conf();
	}else{
		gpio_switch(gpio_id, &gpio_name, &gpio_func);
		PIN_FUNC_SELECT(gpio_name, gpio_func);
	}
}
 //gpio输出初始化
void ICACHE_FLASH_ATTR gpio_out_init(uint8 gpio_id, bool bit_value) {
	uint32 gpio_name;
	uint8 gpio_func;
	if(gpio_id == D0)
	{
		gpio16_output_conf();
		gpio16_output_set(bit_value);
	}else{
		gpio_switch(gpio_id, &gpio_name, &gpio_func);
		PIN_FUNC_SELECT(gpio_name, gpio_func);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(gpio_id), bit_value);
	}
}

//gpio失能初始化
void ICACHE_FLASH_ATTR gpio_disout_init(uint8 gpio_id) {
	uint32 gpio_name;
	uint8 gpio_func;
	if(gpio_id == D0)
	{
		gpio16InputConf();

	}else{
		gpio_switch(gpio_id, &gpio_name, &gpio_func);
		PIN_PULLUP_DIS(gpio_name);
		PIN_FUNC_SELECT(gpio_name, gpio_func);
	  	GPIO_DIS_OUTPUT(gpio_id);
	}
}

//gpio写
void ICACHE_FLASH_ATTR gpio_wirte(uint8 gpio_id, bool bit_value){
	if(gpio_id == D0)
	{
		gpio16_output_set(bit_value);
	}else{
		GPIO_OUTPUT_SET(GPIO_ID_PIN(gpio_id), bit_value);
	}
}

//gpio读
uint32 ICACHE_FLASH_ATTR gpio_read(uint8 gpio_id){
	if(gpio_id == D0)
	{
		return gpio16InputGet();
	}else{
		return GPIO_INPUT_GET(GPIO_ID_PIN(gpio_id));
	}
}

//设置按键个数
void ICACHE_FLASH_ATTR set_key_num(u8 n) {
	key_num = n;
}

//长按短按按键回调添加
void ICACHE_FLASH_ATTR key_add(uint8 gpio_id, gokit_key_function long_press,
		gokit_key_function short_press) {
	uint32 gpio_name;
	uint8 gpio_func;

	key_typedef_t * singleKeyOne = (key_typedef_t *) os_zalloc(
			sizeof(key_typedef_t));

	singleKeyOne->gpio_number = keyTotolNum;

	gpio_switch(gpio_id, &gpio_name, &gpio_func);
	//Platform-defined GPIO
	singleKeyOne->gpio_id = gpio_id;
	singleKeyOne->gpio_name = gpio_name;
	singleKeyOne->gpio_func = gpio_func;
	singleKeyOne->release = 0;
	//Button trigger callback type
	singleKeyOne->key_cb0 = long_press;
	singleKeyOne->key_cb1 = short_press;
	if (long_press == NULL) {
		longinit = 0;
	} else {
		longinit = 1;
	}
	singleKey[keyTotolNum] = singleKeyOne;

	keyTotolNum++;

	if (keyTotolNum == key_num) {
		keys.singleKey = singleKey;
		keyParaInit(&keys);
	}
}

//按下松开按键回调添加
void ICACHE_FLASH_ATTR status_key_add(uint8 gpio_id,
		gokit_key_function key_down, gokit_key_function key_up) {
	uint32 gpio_name;
	uint8 gpio_func;

	key_typedef_t * singleKeyOne = (key_typedef_t *) os_zalloc(
			sizeof(key_typedef_t));

	singleKeyOne->gpio_number = keyTotolNum;

	gpio_switch(gpio_id, &gpio_name, &gpio_func);
	//Platform-defined GPIO
	singleKeyOne->gpio_id = gpio_id;
	singleKeyOne->gpio_name = gpio_name;
	singleKeyOne->gpio_func = gpio_func;
	//Button trigger callback type
	singleKeyOne->key_cb0 = key_down;
	singleKeyOne->key_cb1 = key_up;

	singleKeyOne->release = 1;

	singleKey[keyTotolNum] = singleKeyOne;

	keyTotolNum++;

	if (keyTotolNum == key_num) {
		keys.singleKey = singleKey;
		keyParaInit(&keys);
	}
}

//DI mini引脚判断
void ICACHE_FLASH_ATTR gpio_switch(uint8 gpio_id, uint32 *gpio_name,
		uint8 *gpio_func) {
	switch (gpio_id) {
	case D0:
		*gpio_name = 0;
		*gpio_func = 0;
		break;
	case D1:
		*gpio_name = M_D1;
		*gpio_func = F_D1;
		break;
	case D2:
		*gpio_name = M_D2;
		*gpio_func = F_D2;
		break;
	case D3:
		*gpio_name = M_D3;
		*gpio_func = F_D3;
		break;
	case D4:
		*gpio_name = M_D4;
		*gpio_func = F_D4;
		break;
	case D5:
		*gpio_name = M_D5;
		*gpio_func = F_D5;
		break;
	case D6:
		*gpio_name = M_D6;
		*gpio_func = F_D6;
		break;
	case D7:
		*gpio_name = M_D7;
		*gpio_func = F_D7;
		break;
	case D8:
		*gpio_name = M_D8;
		*gpio_func = F_D8;
		break;
	case RX:
		*gpio_name = M_RX;
		*gpio_func = F_RX;
		break;
	case TX:
		*gpio_name = M_TX;
		*gpio_func = F_TX;
		break;
	}
}

