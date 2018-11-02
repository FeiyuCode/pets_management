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
 
/************************************
stm32F103VET6                        //
使用SPI2与模块通信，串口串口打印输出 //
*连线说明：                          //
*1--NSS(SDA)  <----->PF0             //
*2--SCK <----->PB13                  //
*3--MOSI<----->PB15                  // 
*4--MISO<----->PB14                  //
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
*			GND-->GND
*		     5V-->VCC 
***********************************************/
/**************************************************
*矩阵键盘接线
*连线：C1<->C0  C2<->C1  C3<->C2   C4<->C3
*      R1<->C4  R2<->C5  R3<->C6   R4<->C7
*****************************************************/
/**全局变量***/
unsigned char card_1[4]={0x94,0xf8,0xcf,0x10};//card1ID号
unsigned char SN[4]; //卡号
unsigned char CT[2];//卡类型
unsigned char card1_bit=0;  //card1选中标志
unsigned char RFID[16];			//存放RFID 
u8 KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};  //CARD1密码
unsigned char newword[16]={0x05,0x04,0x01,0x08,0x08};//标记已经做过标签的卡

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


 int main(void)
 {	 
	u8 res;
 	DIR picdir;	 		//图片目录
	FILINFO picfileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totpicnum; 		//图片文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值
	u8 pause=0;			//暂停标记
	u8 t;
	u16 temp;
	u16 *picindextbl;	//图片索引表 
	int KeyValue=0;
	 
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
 	usmart_dev.init(72);		//初始化USMART		
 	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	LCD_Init();			   		//初始化LCD   
	W25QXX_Init();				//初始化W25Q128
 	my_mem_init(SRAMIN);		//初始化内部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
 	f_mount(fs[0],"0:",1); 		//挂载SD卡 
 	f_mount(fs[1],"1:",1); 		//挂载FLASH.
	KeyBoard_Init();//初始化矩阵键盘
	Beep_Init();//初始化蜂鸣器
	InitRc522();//初始化射频卡模块
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
  LCD_DrawRectangle(40, 30, 200, 160);//绘制矩形
 	Show_Str(50,50,200,16,"宠物管家标签阅读器",16,0);		
	Show_Str(50,70,200,16,"操作事项：",16,0);		
	Show_Str(50,90,200,16,"KEY1阅读标签",16,0);			
	Show_Str(50,110,200,16,"KEY2设置电话号码",16,0);		
	Show_Str(50,130,200,16,"KEY3一键清零",16,0);		
 	while(f_opendir(&picdir,"0:/PICTURE"))//打开图片文件夹
 	{	    
		Show_Str(30,170,240,16,"PICTURE文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);//清除显示	     
		delay_ms(200);				  
	}  
	totpicnum=pic_get_tnum("0:/PICTURE"); //得到总有效文件数
  	while(totpicnum==NULL)//图片文件为0		
 	{	    
		Show_Str(30,170,240,16,"没有图片文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);//清除显示	     
		delay_ms(200);				  
	}
  	picfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	picfileinfo.lfname=mymalloc(SRAMIN,picfileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,picfileinfo.lfsize);				//为带路径的文件名分配内存
 	picindextbl=mymalloc(SRAMIN,2*totpicnum);				//申请2*totpicnum个字节的内存,用于存放图片索引
 	while(picfileinfo.lfname==NULL||pname==NULL||picindextbl==NULL)//内存分配出错
 	{	    
		Show_Str(30,170,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	
	//记录索引
    res=f_opendir(&picdir,"0:/PICTURE"); //打开目录
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=picdir.index;								//记录当前index
	        res=f_readdir(&picdir,&picfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				picindextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	delay_ms(1500);
	piclib_init();										//初始化画图	   	   
	curindex=0;											//从0开始显示
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
		dir_sdi(&picdir,picindextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&picdir,&picfileinfo);       		//读取目录下的一个文件
        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
		strcpy((char*)pname,"0:/PICTURE/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Clear(BLACK);
		t=0;
		while(1) 
		{
			ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);//显示背景图片
			KeyValue = Keyscan();
			switch(KeyValue){
				case 1:
				{
					readId();
					Show_Str(50,70,200,16,"操作事项1：",16,0);	
				}
				break;
				case 2:
				{
					Show_Str(50,70,200,16,"操作事项2：",16,0);	
				}
				break;
			}
		}					    
		res=0;  
	} 											  
	myfree(SRAMIN,picfileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,picindextbl);			//释放内存		
}
















