#ifndef _cst716_H
#define _cst716_H


#include "system.h"


//与电容触摸屏连接的芯片引脚(未包含IIC引脚) 
//IO操作函数	 
#define CST_RST    				PFout(11)	//FT5206复位引脚
#define CST_INT    				PFin(10)	//FT5206中断引脚	

//I2C读写命令	
#define CST_CMD_WR 				0X2A    	//写命令
#define CST_CMD_RD 				0X2B		//读命令
  
//部分寄存器定义 
#define CST_DEVIDE_MODE 		0x00   		//模式控制寄存器
#define CST_REG_NUM_FINGER      0x02		//触摸状态寄存器

#define CST_TP1_REG 			0X03	  	//第一个触摸点数据地址
#define CST_TP2_REG 			0X09		//第二个触摸点数据地址
 
 
#define	CST_ID_G_LIB_VERSION	0xA1		//版本		
#define CST_ID_G_MODE 			0xA4   		//FT5206中断模式控制寄存器
#define CST_ID_G_THGROUP		0x80   		//触摸有效值设置寄存器
#define CST_ID_G_PERIODACTIVE	0x88   		//激活状态周期设置寄存器


u8 CST716_WR_Reg(u16 reg,u8 *buf,u8 len);
void CST716_RD_Reg(u16 reg,u8 *buf,u8 len);
u8 CST716_Init(void);
u8 CST716_Scan(u8 mode);

#endif
