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
ʹ��SPI2��ģ��ͨ�ţ����ڴ��ڴ�ӡ��� //
*����˵����                          //
*1--NSS(SDA)  <----->PF0             //
*2--SCK <----->PB13 (SCK)            //
*3--MOSI<----->PB15 (MOSI)           // 
*4--MISO<----->PB14 (MISO)          //
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
*			 GND-->GND
*		   5V-->VCC 
***********************************************/
/**************************************************
*������̽���
*���ߣ�C1<->C0  C2<->C1  C3<->C2   C4<->C3
*      R1<->C4  R2<->C5  R3<->C6   R4<->C7
*****************************************************/
/**ȫ�ֱ���***/
unsigned char CT[2];//������
unsigned char SN[4]; //����
u8 RFID[11];			//���RFID 
unsigned char SNCheck[4];//����ϴζ����ǲ������ſ�
unsigned char sn[4];//���ŵ��ַ���
unsigned char card1_bit=0;  //card1ѡ�б�־
unsigned char card_1[4]={0x94,0xf8,0xcf,0x10};//card1ID��
u8 KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};  //CARD1����
unsigned char newword[16]={0x05,0x04,0x01,0x08,0x08};//������ְ�  ɨ����Ŀ����������һ��
unsigned char RFID1[16]={0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};//д����
char result[16];
u8 t,len,num_wake=0;

u8 flag_function_1;//��һ�����ܵı�־λ
u8 flag_hc_05;
u8  Read_Data[16],PassWd[6],WriteData[16],RUSART_RX_BUF[30], MLastSelectedSnr[4],NewKey[16];//���ڸ�ֵ�õ�
u8 str[8],dst[8];//��16������ת��Ϊ�ַ���

u8 setPhone[11];//��������ֻ���
u8 getPhone[11];//��Ŷ������ֻ���
u8 phoneStr[11]; //�����ֻ���ת��Ϊ�ַ�������ʾ��LCD

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

///////////////////////////////////////////////
//���֣�hex_to_bi
//���ܣ���16������ת��Ϊ16�����ַ���
///////////////////////////////////////////////
char hex_to_bi(unsigned char data[])
{
		u8 i;
	 for(i = 0; i<4;i++)
    {
        str[2*i] = data[i]>>4;
        str[2*i+1] = data[i]&0xf;
    }
		    for(i = 0; i<8;i++)
    {
        sprintf(&dst[i],"%X",str[i]);
    }
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
	unsigned char s=0x08;
	status = PcdRequest(PICC_REQALL,CT);/*Ѱ��*/
	if(status==MI_OK)//Ѱ���ɹ�
	{
		printf("Ѱ���ɹ�\r\n");
		status=MI_ERR;
		status = PcdAnticoll(SN);/*����ײ*/
	}
	if (status==MI_OK)//����ײ�ɹ�
	{
		printf("����ײ�ɹ�\r\n");
		status=MI_ERR;		
		printf("ID:%02x %02x %02x %02x\r\n",SN[0],SN[1],SN[2],SN[3]);//���Ϳ���
		hex_to_bi(SN);
		Show_Str(40,90,200,16,"���ţ�",16,0);
		Show_Str(40,110,200,16,dst,16,0);
		if((SN[0]==card_1[0])&&(SN[1]==card_1[1])&&(SN[2]==card_1[2])&&(SN[3]==card_1[3]))
		{
			card1_bit=1;
			printf("�����Ѿ��Ǽǵĳ���\r\n");
		}
		status =PcdSelect(SN);
	}
	
	if(status==MI_OK)//ѡ���ɹ�
	{
		printf("select\r\n");
		status=MI_ERR;
		status =PcdAuthState(0x60,0x09,KEY,SN);//��Ƭ���������������
	}
	
	if(status==MI_OK)//��֤��Ƭ����ɹ�
	{
		printf("��֤��Ƭ����ɹ�\r\n");	
		LED0 = 1;
		delay_ms(400);//��ʱ400ms
		LED0 = 0;
		status=MI_ERR;
		status=PcdRead(0x09,getPhone);//����s=0x08;��ȡ��2����0��
	}

	if(status==MI_OK)//�����ɹ�
	{
		printf("�����ɹ�\r\n");
		for(i=0;i<11;i++)
		{
			printf("%d",getPhone[i]);
			sprintf(&phoneStr[i],"%X",getPhone[i]);
		}//����2��0��Ĭ��ֵ0
		if(newword[0]==RFID[0] && newword[1]==RFID[1] && newword[2]==RFID[2] && newword[3]==RFID[3] && newword[4]==RFID[4]) //ֻ��Ӧ��54188�������
		{
			printf("�Ѿ��Ǽ�\r\n");
			flag++;
		}
		status=MI_ERR;
		delay_ms(100);
		printf("\r\n");
		//if(flag){
		Show_Str(40,130,200,16,"�ֻ����룺",16,0);
		Show_Str(40,150,200,16,getPhone,16,0);
			flag_function_1=1;
		//}
		//else
			//status=PcdWrite(s,newword);//д��
	}
	/*if(status==MI_OK)
	{
		printf("д���ɹ�\r\n");
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
	}	*/				
}


void imageLoader(){
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
	 my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
 	f_mount(fs[0],"0:",1); 		//����SD�� 
 	f_mount(fs[1],"1:",1); 		//����FLASH.
	
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
	if(res==FR_OK)//�򿪳ɹ�
	{	
		dir_sdi(&picdir,picindextbl[curindex]);			//�ı䵱ǰĿ¼����	   
        res=f_readdir(&picdir,&picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
       // if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
     	fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
		strcpy((char*)pname,"0:/PICTURE/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Clear(BLACK);
		t=0;
		ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);//��ʾ����ͼƬ
	}
}

 int main(void)
 {
	int KeyValue=0;
	u8 i,beep_flag=0,hc_05_flag=0;
	u8 j;//j�ֻ����룬kɾ������ 
	 u8 SNMark;//
	 u8 status;
	 u8 hasRead;
	 
	vu8 flag=0;
	 
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	InitRc522();//��ʼ����Ƶ��ģ��
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 	usmart_dev.init(72);		//��ʼ��USMART		
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD   
	KeyBoard_Init();//��ʼ���������
	Beep_Init();//��ʼ��������
	SPI2_Init();
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
	imageLoader();//����ͼƬ
	LCD_DrawRectangle(30, 30, 210, 160);//���ƾ���
	Show_Str(40,70,200,16,"������Ķ���",16,0);	
	Show_Str(40,90,200,16,"KEY1-����",16,0);	
	Show_Str(40,110,200,16,"KEY2-д���ֻ�����",16,0);
	Show_Str(40,130,200,16,"KEY3-�˸�",16,0);
	delay_ms(500);
	LED1=~LED1;
	delay_ms(100);
		while(1) 
		{
			KeyValue = Keyscan();
			if(KeyValue==10){//����
					while(1){
						LCD_Clear(WHITE);
						//readId();
						//if(flag_function_1==1){
							hasRead=1;
							break;
					//	}
						//else
						//{
						//	Show_Str(40,110,200,16,"����ʧ��"��16);
						//}
					}
					flag_function_1=0;
				}
			else if(KeyValue==13&&hasRead==1){
						while(1){
						LCD_Clear(WHITE);
						readId();
						if(flag_function_1==1){
							hasRead=1;
							break;
						}
						//else
						//{
						//	Show_Str(40,110,200,16,"����ʧ��"��16);
						//}
					}
					flag_function_1=0;
					//	readId();
						/*Show_Str(40,50,200,16,"�����ֻ��ţ�",16,0);
						for(j=0;j<12;j++)
						{
							KeyValue = Keyscan();
							if(KeyValue<10&&j<11)
							{	
								LCD_ShowxNum(40+j*10,70,KeyValue,1,16,0);
								setPhone[j]=KeyValue;
							}else if(KeyValue==14){//������ˣ�ɾ��һλ
								if(j>0)
									{
										LCD_Fill(40+(j-1)*10,70,40+(j-1)*10+10,70+16,WHITE);//ɾ��һλ
									}
									j-=2;
							}else if(KeyValue==15){
								break;
							}					
							}	
							status=PcdWrite(0x09,setPhone);//д��
							if(status==MI_OK)
							{
								printf("д���ɹ�\r\n");
								Show_Str(40,70,200,16,"�ֻ���д��ɹ�!",16,0);
								status=MI_ERR;
							}	
							//hasRead=0;
							//delay_ms(300);
							imageLoader();
							//LCD_DrawRectangle(30, 30, 210, 160);//���ƾ���
							//Show_Str(40,70,200,16,"������Ķ���",16,0);	
							//Show_Str(40,90,200,16,"KEY1-����",16,0);	
							//Show_Str(40,110,200,16,"KEY2-д���ֻ�����",16,0);
							//Show_Str(40,130,200,16,"KEY3-�˸�",16,0);
							//delay_ms(500);
							//LED1=~LED1;
							//delay_ms(100);	*/						
				}
				
	}
}		
	 											  
















