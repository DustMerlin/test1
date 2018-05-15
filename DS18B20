#include<reg52.h>
#include<intrins.h>
typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;
sbit rs=P3^0;
sbit rw=P3^1;
sbit e=P3^2;
sbit dq=P3^7;
uint8 d[16]="   tempertury   ";
uint8 num[10]="0123456789";
uint8 a,c,temp;
void delay(uint16 i)
{
	while(i--);
}
/*
void wrc(uint8 c)
{
	delay(1000);
	rs=0;
	rw=0;
	e=0;
	P0=c;
	e=1;
	delay(10);
	e=0;
	P0=c<<4;
	e=1;
	delay(10);
	e=0;
}
void wrd(uint8 dat)
{
	delay(1000);
	rs=1;
	rw=0;
	e=0;
	P0=dat;
	e=1;
	delay(10);
	e=0;
	P0=dat<<4;
	e=1;
	delay(10);
	e=0;
	rs=0;
}
void lcdinit()
{
	delay(1000);
	wrc(0x32);
	wrc(0x28);
	wrc(0x28);
	wrc(0x06);
	wrc(0x0c);
	wrc(0x01);
}
*/

void wrc(uint8 c)
{
	delay(1000);
	rs=0;
	rw=0;
	e=0;
	P0=c;
	e=1;
	delay(10);
	e=0;
}
void wrd(uint8 dat)
{
	delay(1000);
	rs=1;
	rw=0;
	e=0;
	P0=dat;
	e=1;
	delay(10);
	e=0;
	rs=0;
}
void lcdinit()
{
	delay(1000);
	wrc(0x38);
	wrc(0x38);
	wrc(0x38);
	wrc(0x06);
	wrc(0x0c);
	wrc(0x01);
}
void ds18b20init()		//18b20的初始化
{
	dq=1;
	delay(1);
	dq=0;
	delay(80);
	dq=1;
	delay(5);
	dq=0;
	delay(20);
	dq=1;
	delay(35);
}
void ds18b20wr(uint8 dat)	  //18b20写数据
{
	uint8 i;
	for(i=0;i<8;i++)
	{
		dq=0;
		dq=dat&0x01;
		dat>>=1;
		delay(8);//在时序上只有这一块对时序要求最准确，他的时间必须大于15us
		dq=1;
		delay(1);
	}	
}
uint8 ds18b20rd()		  //18b20读数据
{
	uint8 value,i;
	for(i=0;i<8;i++)	
	{
		dq=0;
		value>>=1;
		dq=1;
		if(dq==1)value|=0x80;
		delay(8);//在这一块也对时间要求特别准确，整段程序必须大于60us		
	}
	return value;
}
uint8 readtemp()			  //读取温度内需要复位的
{
	uint8 b;
	ds18b20init();		//初始化
	ds18b20wr(0xcc);   //发送忽略ROM指令
	ds18b20wr(0x44);   //发送温度转换指令
	delay(100);
	ds18b20init();	   //初始化
	ds18b20wr(0xcc);   //发送忽略ROM指令
	ds18b20wr(0xbe);   //发读暂存器指令
	a=ds18b20rd();	 //温度的低八位
	b=ds18b20rd();	 //温度的高八位
	b<<=4;			 //ssss s***；s为标志位s=0表示温度值为正数，s=1温度值为负数
	c=b&0x80;		//温度正负标志位确认
	b+=(a&0xf0)>>4;
	a=a&0x0f;	  //温度的小数部分
	return b;
}
void display()
{
	uint16 i,k;
	float dio;
	dio=a*0.0625;
	k=dio*10000;//取小数点后两位有效数字
	wrc(0x80+0x00);
	for(i=0;i<16;i++)
	{
		wrd(d[i]);
	}
	wrc(0x80+0x43);
	if(c==0x80)	//读取到负温度即为补码，要将其转换成源码
	{
		wrd('-');
		temp=temp-1;
		temp=(~temp)|0x80;//负数的补码即为反码+1；而负数的反码为其源码取反，除了符号位；正数的补码等于正数的反码等于正数的源码
	}	
	else
	{
		wrd('+');
		temp=(temp);
	}			
	wrd(num[temp/100]);
	wrd(num[temp%100/10]);
	wrd(num[temp%100%10]);
	wrd('.');
	wrd(num[k/1000]);
	wrd(num[k%1000/100]);
	wrd(0xdf);
	wrd('C');
}
void main()
{
	lcdinit();
	while(1)
	{
		temp=readtemp();
		display();	
	}		
}
