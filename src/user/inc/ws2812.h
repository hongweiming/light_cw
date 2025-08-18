#ifndef _WS_2812_H_
#define _WS_2812_H_
#include <stdint.h>

#define LED_COUNT   (200)
#define RGB_COUNT   (24) // 每个LED所需的RGB 24位
#define TIME_PERIOD (90) // pwm 周期
#define TIME_HIGH   (60) // pwm 占空比表示高电平 逻辑1  
#define TIME_LOW    (30) // pwm 占空比表示低电平 逻辑0

#define WHITE	(rgb_color_t){255,255,255}
#define BLACK   (rgb_color_t){0,0,0}
#define RED 	(rgb_color_t){255,0,0}
#define GREEN   (rgb_color_t){0,255,0}
#define BLUE	(rgb_color_t){0,0,255}


typedef struct{
    uint8_t r; //cool_ch  6500K
    uint8_t g; //warm_ch 2700k 
    uint8_t b; //no use
}rgb_color_t;

void RGB_Color(uint8_t led_num, rgb_color_t color);
void rgb_black_all(void);
void rgb_color_all_set(rgb_color_t color);

int ws2812_init(void);
void ws2812_update_display(void);

#endif
