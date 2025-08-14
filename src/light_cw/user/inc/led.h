#ifndef _LED_H_
#define _LED_H_
#include <stdint.h>

#define WW  2700
#define CW  6500
//#define STEP 100

typedef enum{
    DUTY_C,
    DUTY_W,
    DUTY_INDEX_MAX,
}DUTY_CW_E;

typedef struct{
    uint16_t    target_cw;
    uint16_t    target_brightness;
    uint8_t     target_duty[DUTY_INDEX_MAX];
    uint8_t     current_duty[DUTY_INDEX_MAX];
    uint8_t     update;
    uint16_t    current_cw;
    uint16_t    current_brightness;
    uint8_t     onoff_f;                        //开关机状态
    uint16_t    app_set_cw;                     //APP设置色温值
    uint8_t     app_set_brightness;             //APP设置亮度值
}led_handle_t;

uint8_t led_get_on_off_status(void);
uint8_t led_get_brightness(void);
void app_led_turn_off(void);
void app_led_turn_on(uint16_t cw, uint16_t brightness);
void app_led_cw_brightness_set(uint16_t cw, uint16_t brightness);
void app_led_cw_brightness_get(void);
void led_cw_dimmer_task(void);
void led_user_test(void);


#endif
