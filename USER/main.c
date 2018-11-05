#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "lcd.h"  
#include "key.h"     
#include "usmart.h" 
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"   
#include "text.h"
#include "piclib.h"	
#include "string.h"		
#include "math.h"
#include "hc05.h" 
#include "rc522.h"
#include "beep.h"
#include "keyboard.h"
#include "spi.h"
 
/************************************
stm32F103VET6                        //
使用SPI2与模块通信，串口串口打印输出 //
*连线说明：                          //
*1--NSS(SDA)  <----->PF0             //
*2--SCK <----->PB13 (SCK)            //
*3--MOSI<----->PB15 (MOSI)           // 
*4--MISO<----->PB14 (MISO)          //
*5--悬空                             //
*6--GND <----->GND                   //
*7--RST <----->PF1                   //
*8--VCC <----->VCC                   //  
************************************/// 
/***********************************************
*蓝牙模块连线说明
*STM32开发板-->ATK-HC05蓝牙模块
*		   PB10-->RXD
*		   PB11-->TXD
*			 GND-->GND
*		   5V-->VCC 
***********************************************/
/**************************************************
*矩阵键盘接线
*连线：C1<->C0  C2<->C1  C3<->C2   C4<->C3
*      R1<->C4  R2<->C5  R3<->C6   R4<->C7
*****************************************************/
/**全局变量***/
unsigned char CT[2];//卡类型
unsigned char SN[4]; //卡号
unsigned char sn[4];//卡号的字符串
unsigned char RFID[16];			//存放RFID 
unsigned char card1_bit=0;  //card1选中标志
unsigned char card_1[4]={0x94,0xf8,0xcf,0x10};//card1ID号
u8 KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};  //CARD1密码
unsigned char newword[16]={0x05,0x04,0x01,0x08,0x08};//我是你爸爸  扫描过的卡，都给标记一下
unsigned char RFID1[16]={0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};//写数据
char result[16];
u8 t,len,num_wake=0;

u8 flag_function_1;//第一个功能的标志位
u8 flag_hc_05;
u8  Read_Data[16],PassWd[6],WriteData[16],RUSART_RX_BUF[30], MLastSelectedSnr[4],NewKey[16];//串口赋值用的
u8 str[8],dst[8];;//将16进制数转换为字符串

//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	return rval;
}

/****************************************
*函数名：readId
*功能：阅读标签
*返回：空
********************************************/
void readId(void)
{
	u8 status,i;
	vu8 flag=0;
	unsigned flag_function_1=0;
	unsigned char s=0x08;
	status = PcdRequest(PICC_REQALL,CT);/*寻卡*/
	if(status==MI_OK)//寻卡成功
	{
		printf("寻卡成功\r\n");
		Show_Str(40,70,200,16,"寻卡成功",16,0);	
		status=MI_ERR;
		status = PcdAnticoll(SN);/*防冲撞*/
	}
	if (status==MI_OK)//防冲撞成功
	{
		printf("防冲撞成功\r\n");
		Show_Str(40,90,200,16,"防冲撞成功",16,0);	
		status=MI_ERR;		
		printf("ID:%02x %02x %02x %02x\r\n",SN[0],SN[1],SN[2],SN[3]);//发送卡号
		if((SN[0]==card_1[0])&&(SN[1]==card_1[1])&&(SN[2]==card_1[2])&&(SN[3]==card_1[3]))
		{
			card1_bit=1;
			printf("这是已经登记的宠物\r\n");
		}
		status =PcdSelect(SN);
	}
	
	if(status==MI_OK)//选卡成功
	{
		printf("选卡成功\r\n");
		Show_Str(40,110,200,16,"选卡成功",16,0);	
		status=MI_ERR;
		status =PcdAuthState(0x60,0x09,KEY,SN);//卡片的密码存在数组中
	}
	
	if(status==MI_OK)//验证卡片密码成功
	{
		printf("验证卡片密码成功\r\n");
		Show_Str(40,130,200,16,"验证卡片密码成功",16,0);	
		LED0 = 1;
		delay_ms(400);//延时400ms
		LED0 = 0;
		status=MI_ERR;
		status=PcdRead(s,RFID);//读卡s=0x08;读取第2扇区0块
	}

	if(status==MI_OK)//读卡成功
	{
		printf("读卡成功\r\n");
		Show_Str(40,150,200,16,"读卡成功",16,0);	
		for(i=0;i<=15;i++)
		{
			printf("%d",RFID[i]);
		}//扇区2块0的默认值0
		if(newword[0]==RFID[0] && newword[1]==RFID[1] && newword[2]==RFID[2] && newword[3]==RFID[3] && newword[4]==RFID[4]) //只适应于54188这个密码
		{
			printf("已经登记\r\n");
			flag++;
		}
		status=MI_ERR;
		delay_ms(100);
		printf("\r\n");
		if(flag)
			flag_function_1=1;
		else
			status=PcdWrite(s,newword);//写卡
	}
	if(status==MI_OK)
	{
		printf("写卡成功\r\n");
		Show_Str(40,170,200,16,"写卡成功",16,0);	
		status=MI_ERR;
		//num_wake++;
		for(i=0;i<=15;i++)
		{
			printf("%d",RFID[i]);
		}//扇区2块0的默认值0
		printf("\r\n");
		flag=0;
		flag_function_1=1;
		//return 1;
	}					
}


///////////////////////////////////////////////
//名字：hex_to_bi
//功能：将16进制数转换为16进制字符串
///////////////////////////////////////////////
char hex_to_bi(unsigned char data[])
{
		u8 i;
	 for(i = 0; i<5;i++)
    {
        str[2*i] = data[i]>>4;
        str[2*i+1] = data[i]&0xf;
    }
		    for(i = 0; i<8;i++)
    {
        sprintf(&dst[i],"%X/n",str[i]);
    }
}

 int main(void)
 {
	int KeyValue=0;
	u8 i,beep_flag=0,hc_05_flag=0;
	 
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	InitRc522();//初始化射频卡模块
	uart_init(115200);	 	//串口初始化为115200
 	usmart_dev.init(72);		//初始化USMART		
 	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	LCD_Init();			   		//初始化LCD   
	KeyBoard_Init();//初始化矩阵键盘
	Beep_Init();//初始化蜂鸣器
	SPI2_Init();
	delay_ms(1000);			//等待蓝牙模块上电稳定
POINT_COLOR=RED; 
while(font_init()) 		//检查字库
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	
while(HC05_Init()) 		//初始化ATK-HC05模块  
	{
		LCD_DrawRectangle(30, 30, 210, 160);//绘制矩形
		Show_Str(40,70,200,16,"HC05错误",16,0);	
		Show_Str(40,90,200,16,"请检查HC05蓝牙模块",16,0);	
		Show_Str(40,110,200,16,"长按HC05蓝牙模块开关",16,0);	
		delay_ms(500);
		LED1=~LED1;
		delay_ms(100);
		
	}	
	LCD_Clear(WHITE);
		while(1) 
		{
			KeyValue = Keyscan();
			switch(KeyValue){
				case 1:
				{
					while(1){
						readId();
					}
				}
				break;
				case 2:
				{
						
				}
				break;
			}
	}
}		
	 											  
















