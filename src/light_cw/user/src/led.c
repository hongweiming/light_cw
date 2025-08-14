#include "led.h"
#include <stdio.h>
#include <string.h>
#include "ws2812.h"
#include "stm32f1xx_hal.h"
#include <math.h>
#include "com_manage.h"
#include "gpio.h"
#include "le_be.h"

#define DELAY_5MS   (5)
#define DELAY_9MS   (9)
#define DELAY_15MS  (15)

typedef enum{
    MODE1,
    MODE2,
    MODE3,
    MODE4,
    MODE5,
    MODE6,
	MODE7,
    MODE8,
    MODE9,
    MODE10,
    MODE11,
    MODE12,
    MODE13,
    MODE14,
    MODE_MAX,
}MODE_E;

typedef struct{
    MODE_E   mode;
    uint16_t cw;
    uint16_t brightness;
}screne_mode_t;

const screne_mode_t screne_mode_table[] = {
    {MODE1, 2700, 0},
    {MODE2, 2700, 100},
    {MODE3, 2700, 0},
    {MODE4, 3000, 100},
    {MODE5, 3000, 0},
    {MODE6, 3500, 100},
    {MODE7, 3500, 0},
    {MODE8, 4000, 100},
    {MODE9, 4000, 0},
    {MODE10, 5000, 100},
    {MODE11, 5000, 0},
	{MODE12, 6000, 100},
    {MODE13, 6000, 0},
    {MODE14, 6500, 100},
};

led_handle_t led_handle = {
    .target_cw = 2700,
    .target_brightness = 0,
    .target_duty = {0,0},
    .current_duty = {0,0},
    .update = 0,
    .current_cw = 2700,
    .current_brightness = 0,
    .onoff_f = 0,
    .app_set_cw = 2700,
    .app_set_brightness = 100,
};

uint8_t led_get_on_off_status(void)
{
    return led_handle.onoff_f;
}
uint8_t led_get_brightness(void)
{
    return led_handle.app_set_brightness;
}

void led_target_duty_cal(uint16_t target_cw, uint8_t target_brightness, uint8_t *target_duty)
{
    float ww = (float)(CW-target_cw)/(CW-WW);
    float cw = (float)(1-ww);
	float duty[2]={0,0};

	duty[0] = (2*target_brightness*cw*255)/200;
	duty[1] = (2*target_brightness*ww*255)/200;

    target_duty[0] = round(duty[0]);//四舍五入
    target_duty[1] = round(duty[1]);//四舍五入
}

void led_cw_brightness_set(led_handle_t *handle, uint16_t cw, uint16_t brightness)
{
    handle->target_cw = cw;
    handle->target_brightness = brightness;
    handle->update = 1;
}
///////////////////////////////////////////////////////////////////////
void app_led_turn_off(void)
{
    led_cw_brightness_set(&led_handle,led_handle.app_set_cw,0);
    printf( "[%s %d] real set [%d,0], app set [%d, %d].\r\n", 
            __func__,__LINE__,led_handle.app_set_cw, led_handle.app_set_cw,led_handle.app_set_brightness);
}

void app_led_turn_on(uint16_t cw, uint16_t brightness)
{
    uint8_t real_set_brightness = 0;
    //参数合法性判断
    if((cw>=WW)&&(cw<=CW))
    {
        led_handle.app_set_cw = cw;
    }
    if((brightness>=1)&&(brightness<=100))
    {
        led_handle.app_set_brightness = brightness;
    }

    if((led_handle.app_set_brightness>=1)&&(led_handle.app_set_brightness<=5))//最低亮度限制5%
    {
        real_set_brightness = 5;
    }
    else
    {
        real_set_brightness = led_handle.app_set_brightness;
    }

    led_cw_brightness_set(&led_handle,led_handle.app_set_cw,real_set_brightness);
    printf( "[%s %d] real set [%d,%d], app set [%d, %d].\r\n", 
            __func__,__LINE__, led_handle.app_set_cw,real_set_brightness,led_handle.app_set_cw,led_handle.app_set_brightness);
}

void app_led_cw_brightness_set(uint16_t cw, uint16_t brightness)
{
    uint8_t real_set_brightness=0;
    //参数合法性判断
    if((cw>=WW)&&(cw<=CW))
    {
        led_handle.app_set_cw = cw;
    }
    if((brightness>=1)&&(brightness<=100))
    {
        led_handle.app_set_brightness = brightness;
    }

    if((led_handle.app_set_brightness>=1)&&(led_handle.app_set_brightness<=5))//最低亮度限制5%
    {
        real_set_brightness = 5;
    }
    else
    {
        real_set_brightness = led_handle.app_set_brightness;
    }

    led_cw_brightness_set(&led_handle,led_handle.app_set_cw,real_set_brightness);
    printf( "[%s %d] real set [%d,%d], app set [%d, %d].\r\n", 
            __func__,__LINE__, led_handle.app_set_cw,real_set_brightness,led_handle.app_set_cw,led_handle.app_set_brightness);
}

void app_led_cw_brightness_get(void)
{
    com_cw_report(led_handle.app_set_cw, led_handle.app_set_brightness);
}
////////////////////////////////////////////////////////////////////////////

void led_cw_dimmer_task(void)
{
    static int init_f = 0;

    if( init_f == 0 )
    {
        //memset(&led_handle, 0, sizeof(led_handle_t));
        if(ws2812_init()==1)
        {
            init_f = 1;
        }
        else
        {
            init_f = -1;
        }
    }

    if( init_f != 1 ) return;

    if( led_handle.update )
    {
        led_handle_t *handle = &led_handle;
        uint8_t *current_duty = (uint8_t *)(handle->current_duty);
        uint8_t *target_duty = (uint8_t *)(handle->target_duty);
        uint8_t duty_update = 0;
        rgb_color_t rgb;

        led_target_duty_cal(handle->target_cw, handle->target_brightness, handle->target_duty);
        //printf("target_duty_cw %d, target_duty_ww %d, current_duty_cw %d, current_duty_ww %d .\r\n", target_duty[DUTY_C], target_duty[DUTY_W], current_duty[DUTY_C], current_duty[DUTY_W]);
        for(DUTY_CW_E ch=DUTY_C;  ch<DUTY_INDEX_MAX; ch++)
        {
            if( current_duty[ch] > target_duty[ch] )
            {
                current_duty[ch]--;
                duty_update = 1;
            }
            else if( current_duty[ch] < target_duty[ch] )
            {
                current_duty[ch]++;
                duty_update = 1;
            }
        }

        if(duty_update)
        {
            duty_update = 0;
            rgb.r = current_duty[DUTY_C];
            rgb.g = current_duty[DUTY_W];
            rgb.b = 0;
            rgb_color_all_set(rgb);
            ws2812_update_display();
            if((rgb.r + rgb.g) < 40)
            {
                HAL_Delay(DELAY_9MS);
            }
            else
            {
                HAL_Delay(DELAY_5MS);
            }
        }
        else
        {
            handle->update = 0;
            handle->current_cw = handle->target_cw;
            handle->current_brightness = handle->target_brightness;
            handle->onoff_f = ((handle->current_brightness==0)? 0:1);
            printf("current_duty_cw %d, current_duty_ww %d . \r\n", current_duty[DUTY_C], current_duty[DUTY_W]);
            printf("cw dimier finish . \r\n");
            com_cw_report(led_handle.app_set_cw, led_handle.app_set_brightness);
        }
    }
}

//流水灯模式 开灯 从左到右
void led_serial_mode_on(void)
{
    //turn on
    for(int on_ic_index=0; on_ic_index<LED_COUNT; on_ic_index++)
    {
        for(int i=0; i<LED_COUNT; i++)
        {
			if(i <= on_ic_index)
			{
				RGB_Color(i, RED);
			}
			else
			{
				RGB_Color(i, BLACK);
			}
        }
        ws2812_update_display();
        HAL_Delay(DELAY_15MS);
    }
}
//流水灯模式 关灯 从右到左
void led_serial_mode_off(void)
{
    //turn off
    for(int off_ic_index=(LED_COUNT-1); off_ic_index>=0; off_ic_index--)
    {
        for(int i=0; i<LED_COUNT; i++)
        {
			if(i < off_ic_index)
			{
				RGB_Color(i, RED);
			}
			else
			{
				RGB_Color(i, BLACK);
			}
        }
        ws2812_update_display();
        HAL_Delay(DELAY_15MS);
    }
}
//流水灯模式测试
void led_serial_mode(void)
{
    static uint8_t on_off_cmd = 0;
    if( on_off_cmd==0 )
    {
        on_off_cmd = 1;
        led_serial_mode_on();
    }
    else if( on_off_cmd==1 )
    {
        on_off_cmd = 0;
        led_serial_mode_off();
    }
}
void led_channel_test(void);
void led_user_test(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    // led_serial_mode();
    // led_channel_test();
    return;
}

void led_channel_test(void)
{
    RGB_Color(0, RED);
    ws2812_update_display();
    HAL_Delay(1000);

    RGB_Color(0, GREEN);
    ws2812_update_display();
    HAL_Delay(1000);

    RGB_Color(0, BLUE);
    ws2812_update_display();
    HAL_Delay(1000);

    rgb_black_all();
    ws2812_update_display();
    HAL_Delay(5000);
}
