#ifndef _lsens_H
#define _lsens_H


#include "system.h"  


void Lsens_Init(void); 				//初始化光敏传感器
u8 Lsens_Get_Val(void);				//读取光敏传感器的值
#endif 
