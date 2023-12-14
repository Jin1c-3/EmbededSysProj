#ifndef _ws2812_H
#define _ws2812_H

#include "system.h"

#define RGB_LED GPIO_Pin_6
#define RGB_LED_HIGH (GPIO_SetBits(GPIOE, RGB_LED))
#define RGB_LED_LOW (GPIO_ResetBits(GPIOE, RGB_LED))

#define RGB_LED_XWIDTH 5
#define RGB_LED_YHIGH 5

#define RGB_COLOR_RED 0XFF0000
#define RGB_COLOR_GREEN 0X008000
#define RGB_COLOR_BLUE 0X0000FF
#define RGB_COLOR_WHITE 0XFFFFFF
#define RGB_COLOR_YELLOW 0XFFFF00
#define RGB_COLOR_PINK 0XFFC0CB

void RGB_LED_Init(void);
void RGB_LED_Reset(void);
void RGB_LED_Write_24Bits(uint8_t green, uint8_t red, uint8_t blue);
void RGB_LED_Red(void);
void RGB_LED_Green(void);
void RGB_LED_Blue(void);
void RGB_LED_Clear(void);
void RGB_DrawDotColor(u8 x, u8 y, u8 status, u32 color);
void RGB_DrawLine_Color(u16 x1, u16 y1, u16 x2, u16 y2, u32 color);
void RGB_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u32 color);
void RGB_Draw_Circle(u16 x0, u16 y0, u8 r, u32 color);
void RGB_ShowCharNum(u8 num, u32 color);
#endif
