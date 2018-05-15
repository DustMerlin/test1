#include<reg51.h>  
#include<intrins.h>  
#define uint unsigned int   
#define uchar unsigned char  
  
/*LCD1602显示模块*/  
/*用于PROTUES仿真 
#define LCD1602_DATAPINS P2  
sbit LCD1602_RS=P3^0;    
sbit LCD1602_RW=P3^1;                                      
sbit LCD1602_E=P3^2;      */  
  
  
#define LCD1602_DATAPINS P0  
sbit LCD1602_RS=P2^6;  
sbit LCD1602_RW=P2^5;                                       
sbit LCD1602_E=P2^7;  
  
/*按键调节模块*/  
sbit k1=P1^1;//功能键  
sbit k2=P1^2;//加  
sbit k3=P1^3;//减   
void keyscan();  
void key1_switch();  
void key2_add();  
void key3_minus();  
  
uchar k_num;  
void LCD1602_SetTemp(uchar add,uchar dat);  
uchar high=90,low=10,set_t=30,set_d=2;//这里的数据类型可能有问题！！  
  
/*其他模块接口*/  
sbit DSPORT=P3^7; //温度传感器单总线端  
sbit SPEAKER=P1^5;//蜂鸣器  
sbit RELAY=P1^4;//继电器  
sbit COOLER=P1^7;//降温装置  
  
/*LCD1602相关函数*/  
void Delay1us(uint a);  
void LCD1602_WriteCom(uchar com);//LCD1602写入8位命令子函数   
void LCD1602_WriteData(uchar dat);        
void LCD1602_Init();//LCD1602初始化子程序                         
void LCD1602_ShowTemp(int temp);  
  
/*DS18B20温度传感器相关函数*/  
void  Delay15us(uchar aa);  
uchar Ds18b20Init();  
void  Ds18b20WriteByte(uchar dat);  
uchar Ds18b20ReadByte();  
void  Ds18b20SwitchTemp();  
int   Ds18b20ReadTemp();  
  
float temp_max=125.00,temp_min=-55.00;  
uchar ii=0,jj=0,kk=0;  
uchar words_h[2]={"H="};  
uchar words_l[2]={"L="};  
uchar words_s[4]={"Set="};  
void Sound();  
void Temp_control(int temp);  
uchar count=0;//用来解决主程序执行太快，按键光标来不及显示的问题  
  
void Temp_control(int temp)  
{     
    unsigned int kk=400;  
    uchar set_h,set_l;  
    set_h=set_t+set_d;  
    set_l=set_t-set_d;  
      
        if (temp<=set_h&&temp>=set_l) //设定温度范围之内  
        {  
              RELAY = 0; //继电器断开  
             COOLER=0;//风扇关闭  
        }  
        else   
        {     
            TR0=0;  
              
            while (kk--)  
            {         
            Sound();  
            }  
            RELAY = 1; //继电器打开，外电路（常开触电）闭合加热  
            if (temp>=set_h)//温度过高  
            {  
                RELAY = 0; //继电器断开  
                 COOLER=1;//打开  
            }  
            TR0=1;  
        }  
}  
  
/*其他模块相关函数*/  
void Sound()//蜂鸣器  
{        
    SPEAKER=1;  
    Delay15us(40);  
    SPEAKER=0;  
    Delay15us(40);  
}  
  
void Delay1us(uint a)  
{  
    uint b,c;  
    for(c=a;c>0;c--)  
        for(b=110;b>0;b--);  
}  
  
void LCD1602_WriteCom(uchar com)      //写入命令  
{  
    LCD1602_E=0;  
    LCD1602_RS=0;//0就写指令  
    LCD1602_RW=0; //0就是写  
    LCD1602_DATAPINS=com;  
    Delay1us(10);  
    LCD1602_E=1;  
    Delay1us(10);  
    LCD1602_E=0;  
}  
             
void LCD1602_WriteData(uchar dat)           //写入数据  
{  
    LCD1602_E=0;  
    LCD1602_RS=1;  
    LCD1602_RW=0;  
    LCD1602_DATAPINS=dat;  
    Delay1us(10);  
    LCD1602_E=1;  
    Delay1us(10);  
    LCD1602_E=0;  
}  
  
void LCD1602_Init()          //LCD初始化子程序  
{  
//  uint num;  
    LCD1602_WriteCom(0x38);  //开显示  
    LCD1602_WriteCom(0x0f);  //开显示不显示光标  
    LCD1602_WriteCom(0x06);  //写一个指针加1  
    LCD1602_WriteCom(0x01);  //清屏  
    LCD1602_WriteCom(0x80);  //设置数据指针起点  
  
    LCD1602_WriteCom(0x80+0x0B);  
    for (ii=0;ii<2;ii++)  
    {     
            LCD1602_WriteData(words_h[ii]);  
    }  
    LCD1602_SetTemp(13,high);  
  
    LCD1602_WriteCom(0x80+0x40);  
    for (kk=0;kk<4;kk++)  
    {     
            LCD1602_WriteData(words_s[kk]);  
    }  
    LCD1602_SetTemp(4+0x40,set_t);  
    LCD1602_WriteCom(0x80+0x47);  
  
    LCD1602_WriteData(0x23);  //显示正负号  
  
    LCD1602_WriteCom(0x80+0x48);  
    LCD1602_WriteData(0x30+set_d);  
              
    LCD1602_WriteCom(0x80+0x4B);  
    for (jj=0;jj<2;jj++)  
    {     
    LCD1602_WriteData(words_l[jj]);  
    }  
    LCD1602_SetTemp(13+0x40,low);  
  
      
    TMOD=0X01;  
    TH0=(65536-50000)/256;  
    TL0=(65536-50000)%256;  
    EA=1;//总中断  
    ET0=1;  
    TR0=1;   
}  
  
void  Delay15us(uchar aa)  
{  
    do{  
    _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();  
    _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();  
    aa--;  
    }while(aa);  
      
}  
  
/*DS18B20温度传感器相关函数*/  
uchar Ds18b20Init()  
{  
    uchar flag;  
    DSPORT=0;     //总线拉低  
    Delay15us(40);//延时480~960us  
    DSPORT=1;  
    Delay15us(2);  
    flag=DSPORT;  
    Delay15us(6);//Ds18b20发出应答信号，延时60~240us  
    Delay15us(25);  
    return  flag;  
}   
  
  
uchar Ds18b20ReadByte()  
{  
    uchar byte,bi;   
    uint i;  
    for (i=0;i<8;i++)  
    {  
        DSPORT=0;  
        _nop_();_nop_();_nop_();_nop_(); //保持低电平至少1us,但不能低于15us  
        DSPORT=1;  
        bi= DSPORT;//读取一位数据,循环8次为一个字节  
        byte = (byte >> 1) | (bi << 7);//将byte左移一位，然后与上右移7位后的bi,8次合成一个字节  
        Delay15us(4);  
  
    }  
    return  byte;  
}   
  
void  Ds18b20WriteByte(uchar dat)  
{  
     uint i;  
     for (i=0;i<8;i++)  
     {  
        DSPORT=0;_nop_();  
        DSPORT=dat&0x01;//一位一位得读，从最低位开始  
        Delay15us(5);  
        DSPORT=1;_nop_();  
        dat>>=1;  
     }  
}   
void  Ds18b20SwitchTemp()  
{  
        Ds18b20Init();  
        Delay15us(7);  
        Ds18b20WriteByte(0xcc);//跳过ROM指令  
        Ds18b20WriteByte(0x44);//启动温度转换指令  
}   
int   Ds18b20ReadTemp()  
{  
    int temp=0;  
    uchar temp_h,temp_l;      
    Ds18b20SwitchTemp();  
    Ds18b20Init();  
    Delay1us(1);  
    Ds18b20WriteByte(0xcc);  //跳过ROM操作命令  
    Ds18b20WriteByte(0xbe);  //发送读取温度命令  
    temp_l = Ds18b20ReadByte();     //读取温度值共16位，先读低字节  
    temp_h = Ds18b20ReadByte();       
    temp= temp_h;  
    temp<<=8;  
    temp|= temp_l;  
    return temp;  
}   
  
void LCD1602_ShowTemp(int temp)  
{  
    float tt;  
    uchar show[5]={0,0,0,0,0};  
    if (temp<0)//温度为负  
    {  
        LCD1602_WriteCom(0x80);     //写地址 80表示初始地址  
        LCD1602_WriteData('-');         //显示负  
        temp=temp-1;temp=~temp;tt=temp; temp=tt*0.0625*100;  
          
    }  
    else//温度为正  
    {  
        LCD1602_WriteCom(0x80);     //写地址 80表示初始地址  
        LCD1602_WriteData('+');         //显示正  
        tt=temp;              
        temp=tt*0.0625*100;  
          
    }  
      
    show[0]=temp/10000;  
    show[1]=temp%10000/1000;  
    show[2]=temp%1000/100;  
    show[3]=temp%100/10;  
    show[4]=temp%10;  
  
    LCD1602_WriteCom(0x82);  
    LCD1602_WriteData('0'+show[0]); //百位   
  
    LCD1602_WriteCom(0x83);  
    LCD1602_WriteData('0'+show[1]); //十位   
  
    LCD1602_WriteCom(0x84);  
    LCD1602_WriteData('0'+show[2]); //个位   
  
    LCD1602_WriteCom(0x85);  
    LCD1602_WriteData('.');  
  
    LCD1602_WriteCom(0x86);  
    LCD1602_WriteData('0'+show[3]); //十分位  
  
    LCD1602_WriteCom(0x87);  
    LCD1602_WriteData('0'+show[4]); //百分位  
  
    LCD1602_WriteCom(0x88);   
    LCD1602_WriteData(0xdf);  
  
    LCD1602_WriteCom(0x89);   
    LCD1602_WriteData('C');  
    LCD1602_WriteCom(0x0C);  
  
    temp=temp/100;  
    Temp_control(temp);  
      
}  
void key1_switch()//功能切换  
{  
    uchar set_h,set_l;  
    set_h=set_t+set_d;  
    set_l=set_t-set_d;  
    //k1键指令  
    if(k1==0)     
    {  
        Delay1us(10);//延时消抖  
            if(k1==0)  
            {     
                k_num++;  
                while(!k1); //按键没有松开  
  
                if(k_num==1)  
                {  
                    TR0=0; //关掉计时器，使得光标得以显示  
                    LCD1602_WriteCom(0x80+0x46);  
                    LCD1602_WriteCom(0x0f);   
                    Delay15us(6);  
                }  
                if(k_num==2)  
                {  
                    LCD1602_WriteCom(0x80+0x48);  
                    LCD1602_WriteCom(0x0f);   
                    Delay15us(6);  
                }  
                if(k_num==3)  
                {  
                    LCD1602_WriteCom(0x80+0x0F);  
                    LCD1602_WriteCom(0x0f);   
                    Delay15us(6);  
                }  
                if(k_num==4)  
                {  
                    LCD1602_WriteCom(0x80+0x4F);  
                    LCD1602_WriteCom(0x0f);   
                    Delay15us(6);  
                }  
                if(k_num==5)  
                {  
                    k_num=0;  
                      
                    LCD1602_WriteCom(0x0C);//光标不显示                    
                    if (high<low)  //用来default  
                    {  
                         high=90;  
                         low=40;//恢复默认值  
                    }  
  
                     if(set_h>high||set_l<low)  
                     {  
                         high=90;  
                         low=10;//恢复默认值   
                          set_t=30;  
                          set_d=2;//恢复默认值  
                     }  
                     TR0=1;//计时器重新打开，使得屏幕得以刷新  
                }  
            }  
  
    }  
}  
void key2_add()//功能加  
{  
        if(k_num!=0)  
        {  
            if(k2==0)  
            {  
                Delay1us(5);  
                if(k2==0)  
                {  
                    while(!k2);  
                    if(k_num==1)  
                    {  
                        set_t++;  
                        if (set_t>=high)  
                          set_t=high;  
                          LCD1602_SetTemp(4+0x40,set_t);  
                          LCD1602_WriteCom(0x80+0x46);                                        
                    }  
                    if(k_num==2)  
                    {  
                        set_d++;  
                        if (set_d>9)  
                          set_d=0;  
                        LCD1602_WriteCom(0x80+0x48);  
                        LCD1602_WriteData(0x30+set_d);  
                        LCD1602_WriteCom(0x80+0x48);                                          
                    }  
                    if(k_num==3)  
                    {  
                        high++;  
                        if (high>=temp_max)  
                          high=0;  
                          LCD1602_SetTemp(13,high);  
                          LCD1602_WriteCom(0x80+0x0F);  
//                        LCD1602_WriteCom(0x80+0x40+8);                                          
                    }  
                    if(k_num==4)  
                    {  
                        low++;  
                        if (low>=temp_max)  
                         low=0;  
                          LCD1602_SetTemp(13+0x40,low);  
                          LCD1602_WriteCom(0x80+0x4F);  
//                        LCD1602_WriteCom(0x80+0x40+13);                             
                    }  
                }  
            }  
        }  
}  
  
void key3_minus()//功能减  
{  
        if(k_num!=0)  
        {  
            if(k3==0)  
            {  
                Delay1us(5);  
                if(k3==0)  
                {  
                    while(!k3);  
                    if(k_num==1)  
                    {  
                        if (set_t<=0)  
                        set_t=high+1;  
                        set_t--;  
  
                          LCD1602_SetTemp(4+0x40,set_t);  
                          LCD1602_WriteCom(0x80+0x46);                                        
                    }  
                    if(k_num==2)  
                    {  
                        if (set_d<=0)  
                         set_d=10;  
                        set_d--;  
  
                        LCD1602_WriteCom(0x80+0x48);  
                        LCD1602_WriteData(0x30+set_d);  
                        LCD1602_WriteCom(0x80+0x48);                                          
                    }  
                    if(k_num==3)  
                    {  
                        if (high<0)  
                         high=temp_max;  
                        high--;  
                          LCD1602_SetTemp(13,high);  
                          LCD1602_WriteCom(0x80+0x0F);  
//                        LCD1602_WriteCom(0x80+0x40+8);                                          
                    }  
  
                    if(k_num==4)  
                    {  
                        if (low<=0)  
                        low=high;  
                        low--;  
  
                          LCD1602_SetTemp(13+0x40,low);  
                          LCD1602_WriteCom(0x80+0x4F);  
//                        LCD1602_WriteCom(0x80+0x40+16);                             
                    }  
                }  
            }  
        }     
}  
void keyscan()//按键检测  
{  
      key1_switch();  
      key2_add();  
      key3_minus();  
}  
  
void LCD1602_SetTemp(uchar add,uchar dat)//数码管制定位置显示  
{  
      uchar head,body,end;  
      head=dat/100;  
      body=dat%100/10;  
      end=dat%10;  
      LCD1602_WriteCom(0x80+add);  
      LCD1602_WriteData(0x30+head);  
      LCD1602_WriteData(0x30+body);  
      LCD1602_WriteData(0x30+end);  
}  
  
  
void main()  
{  
  
    LCD1602_Init();  
    while(1)  
    {  
        keyscan();                
    }  
}  
void timer0() interrupt 1  
{  
    TH0=(65536-50000)/256;  
    TL0=(65536-50000)%256;  
    count++;  
        if (count==10)//让温度每0.5s刷新一次  
        {  
            count=0;  
            LCD1602_ShowTemp(Ds18b20ReadTemp());  
        }  
}  
