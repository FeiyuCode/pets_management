#include "sys.h"
#include "beep.h"
void Beep_Init(void)	
{
	GPIO_InitTypeDef GPIO_InitStructure;
  	  
 	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTBʱ��ʹ�� 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	 //IO-->�PPB8 �˿�����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��PB8
  
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);			             //PB8�����,�رշ�����
}


//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
