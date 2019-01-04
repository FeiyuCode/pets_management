# 基于stm32的宠物RFID阅读器

------
**使用：** 
> * 在sd卡\PICTURE目录下面修改背景图片，支持多张图片扫描显示。


**实验器材：**
> * 战舰V3\精英STM32F103开发板。
> * MFRC522模块，FRID标签卡。
> * LCD屏幕。
> * 矩阵键盘模块、HC05蓝牙模块。
> * SD卡。
	
**实验目的：**
> * 学习MFRC522模块、蓝牙模块、stm32文件系统，字库的使用。

**硬件资源：**
	1,DS0(连接在PB5)。
	2,串口1(波特率:115200,PA9/PA10连接在板载USB转串口芯片CH340上面)。
	3,串口3(波特率:115200，PB10/PB11)。
	4,ALIENTEK 2.8/3.5/4.3/7寸TFTLCD模块(通过FSMC驱动,FSMC_NE4接LCD片选/A10接RS)。
	5,按键KEY0(PE4)/KEY1(PE3)/KEY_UP(PA0,也称之为WK_UP)。
	6,SD卡,通过SDIO(SDIO_D0~D4(PC8~PC11),SDIO_SCK(PC12),SDIO_CMD(PD2))连接 。
	7,W25Q128(SPI FLASH芯片,连接在SPI2上)。
	8,24C02。
	9,HC05蓝牙模块一个。
	10,矩阵键盘一个，MFRC522模块一个。

	
**连接方式：**
	

 1. MFRC522模块
*1--NSS(SDA)  <----->PF0             
*2--SCK <----->PB13 (SCK)            
*3--MOSI<----->PB15 (MOSI)           
*4--MISO<----->PB14 (MISO)          
*5--悬空                            
*6--GND <----->GND                   
*7--RST <----->PF1                   
*8--VCC <----->VCC  
 2. HC05蓝牙模块
 PB10-->RXD
 PB11-->TXD
 GND-->GND
 5V-->VCC 
 3. 矩阵键盘
C1<->PC0  C2<->PC1  C3<->PC2   C4<->PC3
R1<->PC4  R2<->PC5  R3<->PC6   R4<->PC7

		
**实验现象:**
> * 	KEY1可以读卡的标签号和卡内存储的主人手机号码。
> * 	KEY2往卡内写入主人的电话号码，支持输错退格。
![宠物RC522标签阅读器][1]


  [1]: https://github.com/wongnoubo/pets_management/blob/master/images/result.jpg
