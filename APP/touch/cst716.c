#include "cst716.h"
#include "touch.h"
#include "ctiic.h"
#include "usart.h"
#include "SysTick.h" 
#include "string.h" 


//写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
u8 CST716_WR_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i;
	u8 ret=0;
	CT_IIC_Start();	 
	CT_IIC_Send_Byte(CST_CMD_WR);	//发送写命令 	 
	CT_IIC_Wait_Ack(); 	 										  		   
	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  	//发数据
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
    CT_IIC_Stop();					//产生一个停止条件	    
	return ret; 
}
//读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度			  
void CST716_RD_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(CST_CMD_WR);   	//发送写命令 	 
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(CST_CMD_RD);   	//发送读命令		   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //发数据	  
	} 
    CT_IIC_Stop();//产生一个停止条件     
} 

u8 CIP[5]; //用来存放触摸IC
//初始化触摸屏
//返回值:0,初始化成功;1,初始化失败 
u8 CST716_Init(void)
{
	u8 temp[5];  		
 	GPIO_InitTypeDef  GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);	 //使能PF端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 // PF11端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);//PF11推挽输出
	GPIO_SetBits(GPIOF,GPIO_Pin_11);//上拉
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				 // PB2端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //上拉输入
	GPIO_Init(GPIOF, &GPIO_InitStructure);//PF10上拉输入
	GPIO_SetBits(GPIOF,GPIO_Pin_10);//上拉		

	CT_IIC_Init();      	//初始化电容屏的I2C总线  
	CST_RST=0;				//复位
	delay_ms(20);
 	CST_RST=1;				//释放复位		    
	delay_ms(50);  
	
	CST716_RD_Reg(0xA7,&temp[0],1);
	printf("temp[0]=%d\r\n",temp[0]);
	
	temp[0]=0;
	CST716_WR_Reg(CST_DEVIDE_MODE,temp,1);	//进入正常操作模式 
	CST716_WR_Reg(CST_ID_G_MODE,temp,1);		//查询模式 
	temp[0]=22;								//触摸有效值，22，越小越灵敏	
	CST716_WR_Reg(CST_ID_G_THGROUP,temp,1);	//设置触摸有效值
	temp[0]=12;								//激活周期，不能小于12，最大14
	CST716_WR_Reg(CST_ID_G_PERIODACTIVE,temp,1); 
	//读取版本号，参考值：0x3003
	CST716_RD_Reg(CST_ID_G_LIB_VERSION,&temp[0],2);  
//	if(temp[0]==0X30&&temp[1]==0X03)//版本:0X3003
	{ 
		printf("CTP ID:%x\r\n",((u16)temp[0]<<8)+temp[1]);
	} 
    return 0;
}

const u16 CST716_TPX_TBL[5]={CST_TP1_REG,CST_TP2_REG};
u8 g_gt_tnum=2;      //默认支持的触摸屏点数(2点触摸)

//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 CST716_Scan(u8 mode)
{
	u8 buf[4];
	u8 i=0;
	u8 res=0;
	u8 temp;
    u16 tempsta;
	static u8 t=0;//控制查询间隔,从而降低CPU占用率   
	t++;
	if((t%10)==0||t<10)//空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
	{
        CST716_RD_Reg(CST_REG_NUM_FINGER,&mode,1);//读取触摸点的状态  
        
		if((mode&0XF)&&((mode&0XF)<=g_gt_tnum))
		{
			temp=0XFF<<(mode&0XF);//将点的个数转换为1的位数,匹配tp_dev.sta定义 
			tempsta=tp_dev.sta;    //保存当前的tp_dev.sta值
            tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES;
            tp_dev.x[g_gt_tnum-1]=tp_dev.x[0];//保存触点0的数据,保存在最后一个上
            tp_dev.y[g_gt_tnum-1]=tp_dev.y[0];

            delay_ms(4);    //必要的延时，否则老是认为有按键按下

			for(i=0;i<g_gt_tnum;i++)
			{
				if(tp_dev.sta&(1<<i))		//触摸有效?
				{
					CST716_RD_Reg(CST716_TPX_TBL[i],buf,4);	//读取XY坐标值 
					if(tp_dev.touchtype&0X01)//横屏
					{
						tp_dev.y[i]=((u16)(buf[0]&0X0F)<<8)+buf[1];
						tp_dev.x[i]=(((u16)(buf[2]&0X0F)<<8)+buf[3]);
					}else
					{
						tp_dev.x[i]=(((u16)(buf[0]&0X0F)<<8)+buf[1]);
						tp_dev.y[i]=((u16)(buf[2]&0X0F)<<8)+buf[3];
					} 
                    
					printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
				}			
			} 
			res=1;
            if(tp_dev.x[0]>tftlcd_data.width||tp_dev.y[0]>tftlcd_data.height)  //非法数据(坐标超出了)
            {
                if((mode&0XF)>1)   // 有其他点有数据,则复第二个触点的数据到第一个触点.
                {
                    tp_dev.x[0]=tp_dev.x[1];
                    tp_dev.y[0]=tp_dev.y[1];
                    t=0;  // 触发一次,则会最少连续监测10次,从而提高命中率 
                }
                else        // 非法数据,则忽略此次数据(还原原来的) 
                {
                    tp_dev.x[0]=tp_dev.x[g_gt_tnum-1];
                    tp_dev.y[0]=tp_dev.y[g_gt_tnum-1];
                    mode=0X80;
                    tp_dev.sta=tempsta;   // 恢复tp_dev.sta 
                }
            }
            else t=0;      // 触发一次,则会最少连续监测10次,从而提高命中率 
		}
	}
 
	if((mode&0X1F)==0)//无触摸点按下
	{ 
		if(tp_dev.sta&TP_PRES_DOWN)	//之前是被按下的
		{
			tp_dev.sta&=~TP_PRES_DOWN;	//标记按键松开
		}else						//之前就没有被按下
		{ 
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
			tp_dev.sta&=0XE0;	//清除点有效标记	
		}
	}
    
	if(t>240)t=10;//重新从10开始计数
	return res;
}
