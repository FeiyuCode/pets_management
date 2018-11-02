#include "sys.h"
#include "beep.h"
void Beep_Init(void)	
{
	GPIO_InitTypeDef GPIO_InitStructure;
  	  
 	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	 //IO-->PB8 端口配置
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化PB8
  
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);			             //PB8输出低,关闭蜂鸣器
}


//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
