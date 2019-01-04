#include "stm32f10x.h" 
#include "keyboard.h" 
#include "delay.h"

void KeyBoard_Init(void) 
	{ 
		GPIO_InitTypeDef GPIO_InitStructure; 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB,PE端口时钟
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1|GPIO_Pin_2 | GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure); 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2 | GPIO_Pin_3);
		GPIO_ResetBits(GPIOC, GPIO_Pin_4|GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
	}
	
int Keyscan(void)	
{
  int KeyValue=0;
	int y=0;
	int f=5;
	while(f)
	{
		if((GPIO_ReadInputData(GPIOC)&0xff)!=0x0f) 
		{ 
			delay_ms(20);
			 if((GPIO_ReadInputData(GPIOC)&0xff)!=0x0f) 
			{ 
				GPIO_SetBits(GPIOC,GPIO_Pin_0); 

				GPIO_ResetBits(GPIOC,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3);

				switch(GPIO_ReadInputData(GPIOC)&0xff)
           { 
                case 0x11:KeyValue=10;printf("KEY=%d\n",KeyValue);break; 
 
                case 0x21:KeyValue=11;printf("KEY=%d\n",KeyValue);break; 
 
                case 0x41:KeyValue=0;printf("KEY=%d\n",0);break; 
 
                case 0x81:KeyValue=12;printf("KEY=%d\n",KeyValue);break;
            } 
                GPIO_SetBits(GPIOC,GPIO_Pin_1);
 
                GPIO_ResetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3);
 
                 switch(GPIO_ReadInputData(GPIOC)&0xff)
               { 
                 case 0x12:KeyValue=13;printf("KEY=%d\n",KeyValue);break;
 
                 case 0x22:KeyValue=3;printf("KEY=%d\n",3);break; 
 
 
                 case 0x42:KeyValue=2;printf("KEY=%d\n",2);break;
 
                 case 0x82:KeyValue=1;printf("KEY=%d\n",1);break; 
 
                } 
                 GPIO_SetBits(GPIOC,GPIO_Pin_2); 


                 GPIO_ResetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_3);


                  switch(GPIO_ReadInputData(GPIOC)&0xff)
                   { 
                    case 0x14:KeyValue=14;printf("KEY=%d\n",KeyValue);break;
 
                    case 0x24:KeyValue=6;printf("KEY=%d\n",6);break;
 
                    case 0x44:KeyValue=5;printf("KEY=%d\n",5);break;
 
                    case 0x84:KeyValue=4;printf("KEY=%d\n",4);break;
                   } 
                    GPIO_SetBits(GPIOC,GPIO_Pin_3); 
 
                    GPIO_ResetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2);
                                              
                   switch(GPIO_ReadInputData(GPIOC)&0xff)
                    {
                     case 0x18:KeyValue=15;printf("KEY=%d\n",KeyValue);break;

                     case 0x28:KeyValue=9;printf("KEY=%d\n",9);break; 

                     case 0x48:KeyValue=8;printf("KEY=%d\n",8);break;

                     case 0x88:KeyValue=7;printf("KEY=%d\n",7);break;
                     } 
                        GPIO_SetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2 |GPIO_Pin_3); 
                        GPIO_ResetBits(GPIOC, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 |GPIO_Pin_7);
                        while((GPIO_ReadInputData(GPIOC)&0xff)!=0x0f);
                      switch(KeyValue) 
                      { 
                        case 0:f=3;break;
												case 1:f=3;break;
												case 2:f=3;break;
												case 4: f=3;break;
												case 5: f=3;break;
												case 6: f=3;break;
												case 8:f=3;break;
												case 9: f=3;break;

												 case 10: f=3; break;
												 case 12:  f=3;break;
												case 13:  f=3;break;
													case 14: f=3; break;
												 case 15:  f=3;break;
												case 16:  f=3;break;
                       }	
									return KeyValue;
            }
		}
	
  } 	
}
	

				

