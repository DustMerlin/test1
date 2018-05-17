#include<reg51.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned int  
#define fosc 11059200                  	//宏定义晶体振荡频率 
#define timer1ms 1000
#define on 0                            //电磁继电器，宏定义 吸合 
#define off 1														//断开 
                   	 
sbit DQ = P3^3;     					//DS18B20 串口定义 
bit sflag;						   	//DS18B20 中断函数定义 
uint scnt = 0;
uchar buf[2];
uint factem = 0;                        //DS18B20读到的真实值 



sbit RS = P3^0;							//LCD串口定义 
sbit RW = P3^1;
sbit EP = P3^2; 
uchar dis1[] = {" Now:__ C       "};   //修改值dis1[5]，dis1[6] 
uchar dis2[] = {"      Set:__ C  "};	//修改值dis2[10]，dis2[11]



sbit ctrl = P3^4;						//电磁继电器串口你定义 

	

	
	
/*                
void delay(uint k);                 	//函数声明部分
uchar scankey(void);                	//扫描键盘
bit reset_DS18B20(void);            	//DS复位时序函数 
void write_DS18B20(uchar value);    	//写DS命令函数
uchar read_DS18B20(void);          		//读取DS存储器中的数据函数 
void T0_ISR(void); interrupt 1			//DS温度数据处理  
void delay_(uchar ms);					//LCD延时子函数
uchar busy_check(void);					//LCD状态读取函数
void lcd_wcmd(uchar cmd);				//写入指令数据到LCD 
void led_pos(uchar pos);				//设定显示位置
void lcd_wdat(uchar dat);				//写入字符显示数据到LCD 
void lcd_disp(void);					//显示字符位置函数
void lcd_init(void);					//LCD初始化设定
 */





//延时 (k =100   --> 10us ) 
void delay(uint k)
{
	uint m,n;
	for(m=0;m<k;m++)    
		for(n=0;n<120;n++);
}





//键盘扫描 
uchar scankey(void)                                
{
	uchar temp,temp1,key;
	temp = P1;
	temp &= 0x0F ;
	switch(temp)
	{
		case 0x0E:
		{	
			P1 = 0xF0;
			delay(100);
			temp1 = P1;
			temp1 &= P1;
			switch(temp1)
			{
				case 0x70:	key =  0;	break;
				case 0xB0:	key =  1;	break;
				case 0xD0:	key =  2;	break;
				case 0xE0:	key =  3;	break;
				default: 	key =  0;	break;			   
			}
			P1 = 0x0F;
			break;
		}
		case 0x0D:
		{	
			P1 = 0xF0;
			delay(100);
			temp1 = P1;
			temp1 &= P1;
			switch(temp1)
			{
				case 0x70:	key =  4;	break;
				case 0xB0:	key =  5;	break;
				case 0xD0:	key =  6;	break;
				case 0xE0:	key =  7;	break;
				default: 	key =  0;	break;			   
			}
			P1 = 0x0F;
			break;
		}
		case 0x0B:
		{	
			P1 = 0xF0;
			delay(100);
			temp1 = P1;
			temp1 &= P1;
			switch(temp1)
			{
				case 0x70:	key =  8;	break;
				case 0xB0:	key =  9;	break;
				case 0xD0:	key = 10;	break;
				case 0xE0:	key = 11;	break;
				default: 	key =  0;	break;			   
			}
			P1 = 0x0F;
			break;
		}
		case 0x07:
		{	
			P1 = 0xF0;
			delay(100);
			temp1 = P1;
			temp1 &= P1;
			switch(temp1)
			{
				case 0x70:	key = 12;	break;
				case 0xB0:	key = 13;	break;
				case 0xD0:	key = 14;	break;
				case 0xE0:	key = 15;	break;
				default: 	key =  0;	break;			   
			}
			P1 = 0x0F;
			break;
		}
		default:			key =  0;	break;
	}
	return key;
} 




//DS相关函数 
bit reset_DS18B20(void)            //DS复位时序函数 
{
	uchar i;
	DQ = 0;
	for(i=255; i>0; i--);
	DQ = 1; 
	for(i=60; i>0; i--);
	for(i=255; i>0; i--);
	return DQ;
}


void write_DS18B20(uchar value)   //写DS命令函数
{
	uchar i,j;
	for(i=0; i<8; i++)
	{
		if((value & 0x01) == 0)
		{
			DQ = 0;						
			for(j=35; j>0; j--);		//40us 
			DQ = 1;						
		}
		else 
		{
			DQ = 0;
			_nop_();					//15us 
			_nop_();
			_nop_();
			_nop_();
			DQ = 1;					
			for(j=35; j>0; j--);		//45us			
		}
		value >>= 1;
	}
}


uchar read_DS18B20(void)          //读取DS存储器中的数据函数 
{
	uchar i,j,temp = 0;
	for(i=0; i<8; i++)
	{
		temp >>= 1;
		DQ = 0;					
		_nop_();				
		_nop_();
		_nop_();
		_nop_();
		DQ= 1;					
		for(j=15; j>0; j--);	
		if(DQ == 1) 				
		{
			temp = temp | 0x80;
		}
		else
		{
			temp = temp | 0x00;
		}
		for(j=20; j>0; j--);	
	}
	 	return temp;
}





//DS温度数据处理 
void T0_ISR(void) interrupt 1
{
	uchar x,i = 6;                                    //数据定义x,result
	uint result;
	TH0 = (65536-fosc/12/timer1ms)/256;			//T0定时1ms的初值装入TH0，TL0 ,重装初值
	TL0 = (65536-fosc/12/timer1ms)%256;
	//LED?????? 
	scnt ++;
	if(scnt == 1000)
	{
		scnt = 0;	
		while(reset_DS18B20());					//复位DS18B20
		write_DS18B20(0xcc);
		write_DS18B20(0xbe);					//向DS18B20发送读命令
		buf[0] = read_DS18B20();				//从DS18B20内读数 
		buf[1] = read_DS18B20();
		                                        
		                                        
		                                        
//		sflag = 0;
//		if((buf[1] & 0xF8) != 0x00)				//判断温度是否为负（无用） 
//		{
//			sflag = 1;
//			buf[1] = ~buf[1];					
//			buf[0] = ~buf[0];
//			result = buf[0] + 1;
//			buf[0] = result;
//			if(result > 255) buf[1]++; 
//		}  
		buf[1] <<= 4;							//取得有效值 
		buf[1] &= 0x70;
		x = buf[0];
		x >>= 4;
		x &= 0x70;
		buf[1] |= x;
		x = 2;
		result = buf[1];
		factem = result; 
//	uchar i = 6;
//	EG:	while(result/10)                        //送入显示缓冲，下同 
//	 	{
//	 		LEDBuffer[x] = result % 10;
//	 		result = result / 10;
//	 		x ++;	
//		}

		while(result / 10)
		{
			dis1[i] = result % 10 + '0';
			result = result / 10 + '0';
			i --; 
		} 

		i = 6;

//		LEDBuffer[x] = result;
//		if(sflag == 1)								//小数部分 
//				LEDBuffer[x + 1] = 17;
//		x = buf[0] & 0x0f;
//		x <<= 1;
//		LEDBuffer[0] = (dotcode[x]) % 10;		
//		LEDBuffer[1] = (doctode[x]) / 10;
		while(reset_DS18B20());						//复位DS18B20b 
		write_DS18B20(0xcc);
		write_DS18B20(0x44);						//发送温度转换命令 	  	 
	}  
} 





//LCD延时子函数
void delay_(uchar ms)
{
	uchar i;
	while( ms-- )
	{
		for(i = 0; i < 120; i ++);	
	} 
} 


uchar busy_check(void) 						//LCD状态读取函数
{
	uchar LCD_Status;
	RS = 0;
	RW = 1;
	EP = 1;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	LCD_Status = P2 & 0x80;
	EP = 0;
	return LCD_Status;
}


void lcd_wcmd(uchar cmd) 				//写入指令数据到LCD  
{
	while(busy_check());				//LCD状态查询 
	RS = 0;
	RW = 0;
	EP = 0;
	_nop_();
	_nop_();
	P2 = cmd;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	EP = 1;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	EP = 0;	
} 


void lcd_pos(uchar pos) 				//设定显示位置
{
	lcd_wcmd( pos | 0x80);				//设置LCD当前光标的位置  
} 


void lcd_wdat(uchar dat) 				//写入字符显示数据到LCD
{
	while(busy_check());
	RS = 1;
	RW = 0;
	EP = 0;
	P2 = dat;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	EP = 1;
	_nop_();
	_nop_();
	_nop_();
	_nop_(); 
	EP = 0;	
} 
 
void lcd_disp(void) 					//显示字符函数 
{
	uchar i;
	lcd_pos(1);
	i = 0;
	while(dis1[i] != '\0')
	{
		lcd_wdat(dis1[i]);
		i ++;
	}
	lcd_pos(0x41);
	i = 0;
	while(dis2[i] != '\0');
	{
		lcd_wdat(dis2[i]);	
		i ++;
	}		
} 


void lcd_init(void)						//LCD初始化设定
{
	lcd_wcmd(0x38);						//设置显示格式为16*2行，5*7点阵，8位数据接口
	delay_(1);
	lcd_wcmd(0x0f);						//0x0f ——显示开关设置，显示光标并闪烁
	delay_(1);
	lcd_wcmd(0x06);						//0x06 ——读写后指针加1 
	delay_(1);
	lcd_wcmd(0x01);						//清除LCD的显示内容 
	delay_(1);                           
}






int main()
{
	
	//初始化 
	bit key_flag;                      //读到键位按下的标记 ，键盘扫描相关数据初始化 
	uchar temp,keynum,set=0,c=0,sign=0;
	int swap[16]={1,2,3,-1,4,5,6,-1,7,8,9,-1,0,11,12,-1},factnum;
	uchar save[2];
	uint i = 0,youset;
	dis1[7] = 248;
	dis2[12] = 248;
	ctrl = off;



	
	//LCD初始化
	lcd_init();						//LCD初始化设定
	delay_(10);
	lcd_disp();						//显示字符函数             




	
	
	while(1)
	{	
		
		
		 
		 

		P1 = 0x0F;
		while(1)
		{
			temp = P1;                     
			temp &= 0x0F;
			if(temp == 0x0F)   
			{
				key_flag = 0;
			}
			else
			{
				delay(100);             	
				temp = P1;                  
				temp &= 0x0F;				
			} 
			if(temp == 0x0F)
			{
				key_flag = 0;
			}
			else 
			{
				key_flag = 1;
				keynum = scankey();		
			}
			while(temp != 0x0F)             
			{                                      		
				temp = P1;                         	
				temp &= 0x0F;					   		
			}
			
			 									   		
			factnum = swap[keynum];
			if(swap[keynum] == 11) set++;
			if(set == 1 || set ==2 ) 
				{											//11为set,12为C清除 
					switch(swap[keynum])                   	//swap[16]={1,2,3,-1,4,5,6,-1,7,8,9,-1,0,11,12,-1}
					{
						case -1: continue; 
						case 11: 
						{
							if(set == 2 ) 
							{
								set = 0; 
								sign = 1;                               				//标记
								dis2[10] = save[0] + '0';
								dis2[11] = save[1] + '0';
							}
							break;
						}
						case 12: 
						{
							save[0] = '_';
							save[1] = '_';
							i = 0;
							break; 
						}              
						default: 
						{
							save[i++] = factnum;
							if(i == 2) 
							{
								i = 0;
								youset = save[0]*10 + save[1];
							}
						}
					}
				}
			if(sign) 
			{
				sign = 0;
				break;
			}
		}
		
		
		
		
		//读取DS18B20温度传感器的数据
		TMOD = 0x01;
		TH0 = (65536-fosc/12/timer1ms)/256;			
		TL0 = (65536-fosc/12/timer1ms)%256;	
		TR0 = 1;
		ET0 = 1;
		EA  = 1;
		while(reset_DS18B20());
		write_DS18B20(0xcc);
		write_DS18B20(0x44);

		
		
		
		
		
		// LCD 显示 
		lcd_init();							//LCD初始化设定
		delay_(10);
		lcd_disp();							//显示字符函数  
		
		//电磁继电器 
		if(factem < youset )     			//youset 为设定温度 
			ctrl = on;
		else ctrl = off;
		
		
		
	}
}
