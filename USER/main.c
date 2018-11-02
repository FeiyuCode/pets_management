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
ʹ��SPI2��ģ��ͨ�ţ����ڴ��ڴ�ӡ��� //
*����˵����                          //
*1--NSS(SDA)  <----->PF0             //
*2--SCK <----->PB13                  //
*3--MOSI<----->PB15                  // 
*4--MISO<----->PB14                  //
*5--����                             //
*6--GND <----->GND                   //
*7--RST <----->PF1                   //
*8--VCC <----->VCC                   //  
************************************/// 
/***********************************************
*����ģ������˵��
*STM32������-->ATK-HC05����ģ��
*		   PB10-->RXD
*		   PB11-->TXD
*			GND-->GND
*		     5V-->VCC 
***********************************************/
/**************************************************
*������̽���
*���ߣ�C1<->C0  C2<->C1  C3<->C2   C4<->C3
*      R1<->C4  R2<->C5  R3<->C6   R4<->C7
*****************************************************/
/**ȫ�ֱ���***/
unsigned char card_1[4]={0x94,0xf8,0xcf,0x10};//card1ID��
unsigned char SN[4]; //����
unsigned char CT[2];//������
unsigned char card1_bit=0;  //card1ѡ�б�־
unsigned char RFID[16];			//���RFID 
u8 KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};  //CARD1����
unsigned char newword[16]={0x05,0x04,0x01,0x08,0x08};//����Ѿ�������ǩ�Ŀ�

//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	return rval;
}

/****************************************
*��������readId
*���ܣ��Ķ���ǩ
*���أ���
********************************************/
void readId(void)
{
	u8 status,i;
	vu8 flag=0;
	unsigned flag_function_1=0;
	unsigned char s=0x08;
	status = PcdRequest(PICC_REQALL,CT);/*Ѱ��*/
	if(status==MI_OK)//Ѱ���ɹ�
	{
		printf("Ѱ���ɹ�\r\n");
		Show_Str(40,70,200,16,"Ѱ���ɹ�",16,0);	
		status=MI_ERR;
		status = PcdAnticoll(SN);/*����ײ*/
	}

	if (status==MI_OK)//����ײ�ɹ�
	{
		printf("����ײ�ɹ�\r\n");
		Show_Str(40,90,200,16,"����ײ�ɹ�",16,0);	
		status=MI_ERR;		
		printf("ID:%02x %02x %02x %02x\r\n",SN[0],SN[1],SN[2],SN[3]);//���Ϳ���
		if((SN[0]==card_1[0])&&(SN[1]==card_1[1])&&(SN[2]==card_1[2])&&(SN[3]==card_1[3]))
		{
			card1_bit=1;
			printf("�����Ѿ��Ǽǵĳ���\r\n");
		}
		status =PcdSelect(SN);
	}
	
	if(status==MI_OK)//ѡ���ɹ�
	{
		printf("ѡ���ɹ�\r\n");
		Show_Str(40,110,200,16,"ѡ���ɹ�",16,0);	
		status=MI_ERR;
		status =PcdAuthState(0x60,0x09,KEY,SN);//��Ƭ���������������
	}
	
	if(status==MI_OK)//��֤��Ƭ����ɹ�
	{
		printf("��֤��Ƭ����ɹ�\r\n");
		Show_Str(40,130,200,16,"��֤��Ƭ����ɹ�",16,0);	
		LED0 = 1;
		delay_ms(400);//��ʱ400ms
		LED0 = 0;
		status=MI_ERR;
		status=PcdRead(s,RFID);//����s=0x08;��ȡ��2����0��
	}

	if(status==MI_OK)//�����ɹ�
	{
		printf("�����ɹ�\r\n");
		Show_Str(40,150,200,16,"�����ɹ�",16,0);	
		for(i=0;i<=15;i++)
		{
			printf("%d",RFID[i]);
		}//����2��0��Ĭ��ֵ0
		if(newword[0]==RFID[0] && newword[1]==RFID[1] && newword[2]==RFID[2] && newword[3]==RFID[3] && newword[4]==RFID[4]) //ֻ��Ӧ��54188�������
		{
			printf("�Ѿ��Ǽ�\r\n");
			flag++;
		}
		status=MI_ERR;
		delay_ms(100);
		printf("\r\n");
		if(flag)
			flag_function_1=1;
		else
			status=PcdWrite(s,newword);//д��
	}
	if(status==MI_OK)
	{
		printf("д���ɹ�\r\n");
		Show_Str(40,170,200,16,"д���ɹ�",16,0);	
		status=MI_ERR;
		//num_wake++;
		for(i=0;i<=15;i++)
		{
			printf("%d",RFID[i]);
		}//����2��0��Ĭ��ֵ0
		printf("\r\n");
		flag=0;
		flag_function_1=1;
		//return 1;
	}					
}


 int main(void)
 {	 
	u8 res;
 	DIR picdir;	 		//ͼƬĿ¼
	FILINFO picfileinfo;//�ļ���Ϣ
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 totpicnum; 		//ͼƬ�ļ�����
	u16 curindex;		//ͼƬ��ǰ����
	u8 key;				//��ֵ
	u8 pause=0;			//��ͣ���
	u8 t;
	u16 temp;
	u16 *picindextbl;	//ͼƬ������ 
	int KeyValue=0;
	 
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 	usmart_dev.init(72);		//��ʼ��USMART		
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD   
	W25QXX_Init();				//��ʼ��W25Q128
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
 	f_mount(fs[0],"0:",1); 		//����SD�� 
 	f_mount(fs[1],"1:",1); 		//����FLASH.
	KeyBoard_Init();//��ʼ���������
	Beep_Init();//��ʼ��������
	InitRc522();//��ʼ����Ƶ��ģ��
	delay_ms(1000);			//�ȴ�����ģ���ϵ��ȶ�
POINT_COLOR=RED; 
while(font_init()) 		//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  	
while(HC05_Init()) 		//��ʼ��ATK-HC05ģ��  
	{
		LCD_DrawRectangle(30, 30, 210, 160);//���ƾ���
		Show_Str(40,70,200,16,"HC05����",16,0);	
		Show_Str(40,90,200,16,"����HC05����ģ��",16,0);	
		Show_Str(40,110,200,16,"����HC05����ģ�鿪��",16,0);	
		delay_ms(500);
		LED1=~LED1;
		delay_ms(100);
	}	
  LCD_Clear(WHITE);
  LCD_DrawRectangle(40, 30, 200, 160);//���ƾ���
 	Show_Str(50,50,200,16,"����ܼұ�ǩ�Ķ���",16,0);		
	Show_Str(50,70,200,16,"�������",16,0);		
	Show_Str(50,90,200,16,"KEY1�Ķ���ǩ",16,0);			
	Show_Str(50,110,200,16,"KEY2���õ绰����",16,0);		
	Show_Str(50,130,200,16,"KEY3һ������",16,0);		
 	while(f_opendir(&picdir,"0:/PICTURE"))//��ͼƬ�ļ���
 	{	    
		Show_Str(30,170,240,16,"PICTURE�ļ��д���!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  
	totpicnum=pic_get_tnum("0:/PICTURE"); //�õ�����Ч�ļ���
  	while(totpicnum==NULL)//ͼƬ�ļ�Ϊ0		
 	{	    
		Show_Str(30,170,240,16,"û��ͼƬ�ļ�!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}
  	picfileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	picfileinfo.lfname=mymalloc(SRAMIN,picfileinfo.lfsize);	//Ϊ���ļ������������ڴ�
 	pname=mymalloc(SRAMIN,picfileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
 	picindextbl=mymalloc(SRAMIN,2*totpicnum);				//����2*totpicnum���ֽڵ��ڴ�,���ڴ��ͼƬ����
 	while(picfileinfo.lfname==NULL||pname==NULL||picindextbl==NULL)//�ڴ�������
 	{	    
		Show_Str(30,170,240,16,"�ڴ����ʧ��!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  	
	//��¼����
    res=f_opendir(&picdir,"0:/PICTURE"); //��Ŀ¼
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=picdir.index;								//��¼��ǰindex
	        res=f_readdir(&picdir,&picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
			{
				picindextbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
	delay_ms(1500);
	piclib_init();										//��ʼ����ͼ	   	   
	curindex=0;											//��0��ʼ��ʾ
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//��Ŀ¼
	while(res==FR_OK)//�򿪳ɹ�
	{	
		dir_sdi(&picdir,picindextbl[curindex]);			//�ı䵱ǰĿ¼����	   
        res=f_readdir(&picdir,&picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
     	fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
		strcpy((char*)pname,"0:/PICTURE/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Clear(BLACK);
		t=0;
		while(1) 
		{
			ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);//��ʾ����ͼƬ
			KeyValue = Keyscan();
			switch(KeyValue){
				case 1:
				{
					readId();
					Show_Str(50,70,200,16,"��������1��",16,0);	
				}
				break;
				case 2:
				{
					Show_Str(50,70,200,16,"��������2��",16,0);	
				}
				break;
			}
		}					    
		res=0;  
	} 											  
	myfree(SRAMIN,picfileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�			    
	myfree(SRAMIN,picindextbl);			//�ͷ��ڴ�		
}
















