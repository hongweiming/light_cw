#include "ws2812.h"
#include "tim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint16_t *rgb_buf = NULL;
volatile uint8_t f_pwm_dma_finish = 1;

void RGB_Color(uint8_t led_num, rgb_color_t color)
{
    if( led_num >= LED_COUNT )
    {
        led_num = LED_COUNT - 1;
    }

    uint8_t r_out = (uint8_t) (color.r%256);
    uint8_t g_out = (uint8_t) (color.g%256);
    uint8_t b_out = (uint8_t) (color.b%256);
    uint16_t *pbuf = rgb_buf+(led_num*RGB_COUNT);
    for(uint8_t i=0; i<RGB_COUNT; i++)
    {
        if( i<8 )
            pbuf[i] = ( (r_out)&(1<<(7-i)) )  ? TIME_HIGH:TIME_LOW;
        else if( i<16 )
            pbuf[i] = ( (g_out)&(1<<(15-i)) ) ? TIME_HIGH:TIME_LOW;
        else
            pbuf[i] = ( (b_out)&(1<<(23-i)) ) ? TIME_HIGH:TIME_LOW;
    }
}

void rgb_black_all(void)
{
    for(uint8_t i=0; i<LED_COUNT; i++)
    {
        RGB_Color(i, BLACK);
    }
}

void rgb_color_all_set(rgb_color_t color)
{
    for( uint8_t i=0; i<LED_COUNT; i++ )
    {
        RGB_Color(i, color);
    }
}

void ws2812_update_display(void)
{
    #if 0
    uint16_t *pbuf;
    for(uint8_t j=0; j<(LED_COUNT+1); j++)
    {
        printf("\r\n");
        pbuf = rgb_buf+(j*RGB_COUNT);
        for(uint8_t k=0; k<RGB_COUNT; k++)
        {
            printf("%02d ", pbuf[k]);
        }
    }
    #endif
    f_pwm_dma_finish = 0;
    // 启动 PWM DMA 传输
    HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t *)rgb_buf, (LED_COUNT+1)*RGB_COUNT); //6ms
    while(!f_pwm_dma_finish);

}

// DMA传输完成回调函数, 每次DMA发送完成后会进入这个回调
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim2)
	{
        // 停止DMA发送, 置标志位
		HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);
        f_pwm_dma_finish = 1;
	}
}

int ws2812_init(void)
{
    printf("ws2812 init turn off all led. \r\n");
    rgb_buf = (uint16_t *)malloc( ((LED_COUNT+1)*RGB_COUNT)*2 );
    if(!rgb_buf)
    {
        printf("malloc rgb_buf fail \r\n");
        return -1;
    }
    printf("rgb_buf %p.\r\n",rgb_buf);
    memset((void *)rgb_buf, 0, ((LED_COUNT+1)*RGB_COUNT)*2);
    rgb_black_all();
    ws2812_update_display();
    HAL_Delay(100);
    return 1;
}
