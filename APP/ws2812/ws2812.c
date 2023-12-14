#include "ws2812.h"
#include "SysTick.h"

u8 g_rgb_databuf[3][RGB_LED_XWIDTH][RGB_LED_YHIGH];//RGB

const u8 g_rgb_num_buf[][5]=
{
{0x70,0x88,0x88,0x88,0x70},//0
{0x00,0x48,0xF8,0x08,0x00},//1
{0x48,0x98,0xA8,0x48,0x00},//2
{0x00,0x88,0xA8,0x50,0x00},//3
{0x20,0x50,0x90,0x38,0x10},//4
{0x00,0xE8,0xA8,0xB8,0x00},//5
{0x00,0x70,0xA8,0xA8,0x30},//6
{0x80,0x98,0xA0,0xC0,0x00},//7
{0x50,0xA8,0xA8,0xA8,0x50},//8
{0x40,0xA8,0xA8,0xA8,0x70},//9
{0x38,0x50,0x90,0x50,0x38},//A
{0xF8,0xA8,0xA8,0x50,0x00},//B
{0x70,0x88,0x88,0x88,0x00},//C
{0xF8,0x88,0x88,0x50,0x20},//D
{0xF8,0xA8,0xA8,0xA8,0x00},//E
{0x00,0xF8,0xA0,0xA0,0x00},//F
};

void RGB_LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 
	GPIO_InitStructure.GPIO_Pin = RGB_LED;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOE, &GPIO_InitStructure);					
	GPIO_SetBits(GPIOE,RGB_LED);		
	
	RGB_LED_Clear();
}

void delay(u32 ns)//100ns
{
	while(ns--);
}

/********************************************************/
//
/********************************************************/
void RGB_LED_Write0(void)
{
	RGB_LED_HIGH;
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();
	RGB_LED_LOW;
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();
}

/********************************************************/
//
/********************************************************/

void RGB_LED_Write1(void)
{
	RGB_LED_HIGH;
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();
	RGB_LED_LOW;
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();
}

void RGB_LED_Reset(void)
{
	RGB_LED_LOW;
	delay_us(80);
}

void RGB_LED_Write_Byte(uint8_t byte)
{
	uint8_t i;

	for(i=0;i<8;i++)
	{
		if(byte&0x80)
		{
			RGB_LED_Write1();
		}
		else
		{
			RGB_LED_Write0();
		}
		byte <<= 1;
	}
}

void RGB_LED_Write_24Bits(uint8_t green,uint8_t red,uint8_t blue)
{
	RGB_LED_Write_Byte(green);
	RGB_LED_Write_Byte(red);
	RGB_LED_Write_Byte(blue);
}


//亮灯颜色设定，其他颜色以此类推
void RGB_LED_Red(void)
{
	uint8_t i;
	//LED全彩灯
	for(i=0;i<25;i++)
	{
		RGB_LED_Write_24Bits(0,0xff, 0);
	}
}

void RGB_LED_Green(void)
{
	uint8_t i;

	for(i=0;i<25;i++)
	{
		RGB_LED_Write_24Bits(0xff, 0, 0);
	}
}

void RGB_LED_Blue(void)
{
	uint8_t i;

	for(i=0;i<25;i++)
	{
		RGB_LED_Write_24Bits(0, 0, 0xff);
	}
}

void RGB_LED_Clear(void)
{
	u8 i;
	for(i=0;i<25;i++)
		RGB_LED_Write_24Bits(0,0,0);
	RGB_LED_Reset();
	delay_ms(10);
}

//画点
//x,y:坐标位置
//status：1:点亮，0:熄灭
//color：RGB颜色
void RGB_DrawDotColor(u8 x,u8 y,u8 status,u32 color)
{
	u8 i=0;
	u8 j=0;
	
	RGB_LED_Clear();
	if(status)
	{
		g_rgb_databuf[0][x][y]=color>>16;//r
		g_rgb_databuf[1][x][y]=color>>8;//g
		g_rgb_databuf[2][x][y]=color;//b
	}
	else
	{
		g_rgb_databuf[0][x][y]=0x00;
		g_rgb_databuf[1][x][y]=0x00;
		g_rgb_databuf[2][x][y]=0x00;
	}
		
	for(i=0;i<RGB_LED_YHIGH;i++)
	{
		for(j=0;j<RGB_LED_XWIDTH;j++)
			RGB_LED_Write_24Bits(g_rgb_databuf[1][j][i], g_rgb_databuf[0][j][i], g_rgb_databuf[2][j][i]);
	}
}

void RGB_DrawLine_Color(u16 x1, u16 y1, u16 x2, u16 y2,u32 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		RGB_DrawDotColor(uRow,uCol,1,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 

//画矩形	  
//(x1,y1),(x2,y2):矩形的对角坐标
void RGB_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u32 color)
{
	RGB_DrawLine_Color(x1,y1,x2,y1,color);
	RGB_DrawLine_Color(x1,y1,x1,y2,color);
	RGB_DrawLine_Color(x1,y2,x2,y2,color);
	RGB_DrawLine_Color(x2,y1,x2,y2,color);
}

//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void RGB_Draw_Circle(u16 x0,u16 y0,u8 r,u32 color)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		RGB_DrawDotColor(x0+a,y0-b,1,color);             //5
 		RGB_DrawDotColor(x0+b,y0-a,1,color);             //0           
		RGB_DrawDotColor(x0+b,y0+a,1,color);             //4               
		RGB_DrawDotColor(x0+a,y0+b,1,color);             //6 
		RGB_DrawDotColor(x0-a,y0+b,1,color);             //1       
 		RGB_DrawDotColor(x0-b,y0+a,1,color);             
		RGB_DrawDotColor(x0-a,y0-b,1,color);             //2             
  		RGB_DrawDotColor(x0-b,y0-a,1,color);             //7     	         
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
}

void RGB_ShowCharNum(u8 num,u32 color)
{
	u8 i=0,j=0;
	u8 x=0,y=0;
	u8 temp=0;
	
	for(j=0;j<5;j++)
	{
		temp=g_rgb_num_buf[num][j];
		for(i=0;i<5;i++)
		{
			if(temp&0x80)RGB_DrawDotColor(x,y,1,color);
			else RGB_DrawDotColor(x,y,0,color);
			temp<<=1;
			y++;
			if(y==RGB_LED_YHIGH)
			{
				y=0;
				x++;
				if(x==RGB_LED_XWIDTH)return;
			}
		}
	}
}

