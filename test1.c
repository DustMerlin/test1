#include<reg51.h>
#include<intris.h>
#define uchar unsigned char
#define uint unsigned int  
#define fosc 11059200                          //宏定义晶振频率 11.0592MHZ
#define timer1ms 1000
//#define on 0
//#define off 1        
                            
sbit DQ = P3^3;                                             //部分必要全局定义      //DS18B20管脚声明
bit sflag;                                                                   //DS18B20数据初始化                           ？？？ 
uint scnt = 0;
uchar buf[2];
    
        
                         
void delay(uint k);                         //函数声明部分 
uchar scankey(void);                        //扫描键盘 
bit resetDS(void);                                    //DS复位时序函数 
void writeDS(uchar value);                            //写DS命令函数
uchar readDS(void);                                  //读取DS存储器中的数据函数 
void T0_ISR(void) interrupt 1;  
void startdisplay(void);           
void diaplay(uchar); 



int main()
{
        
        //数据初始化 
        bit key_flag;                      //读到键位按下的标记 ，键盘扫描相关数据初始化 
        uchar temp,keynum,set=0,c=0,factnum=0;
        uchar swap[16]={1,2,3,-1,4,5,6,-1,7,8,9,-1,0,11,12,-1};
        



        
        //显示LCD初始状态
        dispaly(factnum);
        
        while(1)
        {        
                
                
                 
                //4*4键盘扫描函数(防抖动处理机制，(查询方式)）
                P1 = 0x0F;
                while(1)
                {
                        temp = P1;                      //判断是否有键按下 
                        temp &= 0x0F;
                        if(temp == 0x0F)   
                        {
                                key_flag = 0;
                        }
                        else
                        {
                                delay(100);                     //10ms延时去抖 
                                temp = P1;                  //判断是否有键按下
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
//                        while(temp != 0x0F)             
//                        {                                                      //其余键值转换，都为最终实际factnum 
//                                temp = P1;                                         //键值-1，暂为空键 
//                                temp &= 0x0F;                                                           //键值12 为c  , 清除数值，变量c   标记 
//                        }                                                                                            //键值11 为set，设置入口，变量set 标记
                        factnum = swap[keynum];
                        if(factnum == 11) set++;
                        if(set == 1 ) 
                        {
                                switch(swap[keynum])                           //swap[16]={1,2,3,-1,4,5,6,-1,7,8,9,-1,0,11,12,-1}
                                {
                                        case -1: factnum = 0; continue; 
                                        case 11: 
                                        {
                                                if(set == 2 ) 
                                                {
                                                        set =0; 
                                                        factnum = 0;
                                                }
                                                continue;
                                        }
                                        case 12: factnum = 0; break;              // !!!!!!!!!!!!!!!!!!!!!注意，在display函数中，记得修改 
                                        default: break;
                                }
                        }
                        else continue;
                        if(set == 1 ) 
                                display(factnum);
                }
                
                
                
                
                
                //读取DS18B20温度传感器的数据
                TMOD = 0x01;
                TH0 = (65536-fosc/12/timer1ms)/256;                        //T0定时1ms的初值装入TH0，TL0 
                TL0 = (65536-fosc/12/timer1ms)%256;        
                TR0 = 1;
                ET0 = 1;
                EA  = 1;
                while(reset_DS18B20());
                write_DS18B20(0xcc);
                write_DS18B20(0x44);
//                while(1)
//                {
//                        
//                        
//                } !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!问题 
                T0_ISR();
                
                
                
                
                // 显示至 LCD 液晶显示屏
                display(); 
                
                
                
                
                
                //判断DS18B20读数是否与键盘录入的值有出入,电磁继电器控制 
                
                
        }
}





//延时函数 （k =100   --> 10us 延时） 
void delay(uint k)
{
        uint m,n;
        for(m=0;m<k;m++)    
                for(n=0;n<120;n++);
}





//键盘扫描 
uchar scankey(void)                                
{
        uchar temp ,templ,key;
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
                                case 0x70:        key =  0;        break;
                                case 0xB0:        key =  1;        break;
                                case 0xD0:        key =  2;        break;
                                case 0xE0:        key =  3;        break;
                                default:         key =  0;        break;                           
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
                                case 0x70:        key =  4;        break;
                                case 0xB0:        key =  5;        break;
                                case 0xD0:        key =  6;        break;
                                case 0xE0:        key =  7;        break;
                                default:         key =  0;        break;                           
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
                                case 0x70:        key =  8;        break;
                                case 0xB0:        key =  9;        break;
                                case 0xD0:        key = 10;        break;
                                case 0xE0:        key = 11;        break;
                                default:         key =  0;        break;                           
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
                                case 0x70:        key = 12;        break;
                                case 0xB0:        key = 13;        break;
                                case 0xD0:        key = 14;        break;
                                case 0xE0:        key = 15;        break;
                                default:         key =  0;        break;                           
                        }
                        P1 = 0x0F;
                        break;
                }
                default:                        key =  0;        break;
        }
        reruen key;
} 




//DS18B20相关函数 
bit resetDS(void)            //DS复位时序函数 
{
        uchar i;
        DQ = 0;
        for(i=255; i>0; i--);
        DQ = 1; 
        for(i=60; i>0; i--);
        for(i=255; i>0; i--);
        return DQ;
}


void writeDS(uchar value)   //写DS命令函数
{
        uchar i,j;
        for(i=0; i<8; i++)
        {
                if((value & 0x01) == 0)
                {
                        DQ = 0;                                                //DQ 引脚拉低 
                        for(j=35; j>0; j--);                //延时不超过40us 
                        DQ = 1;                                                //DQ 引脚变高 
                }
                else 
                {
                        DQ = 0;
                        _nop_();                                        //延时不超过15us 
                        _nop_();
                        _nop_();
                        _nop_();
                        DQ = 1;                                                //DQ 引脚变高 
                        for(j=35; j>0; j--);                //延时不超过45us                        
                }
                value >>= 1;
        }
}


uchar readDS(void)          //读取DS存储器中的数据函数 
{
        uchar i,j,k;
        temp = 0;
        for(i=0; i<8; i++)
        {
                temp >>= 1;
                DQ = 0;                                        //DQ 引脚拉低 
                _nop_();                                //延时不超过10us的极短延时
                _nop_();
                _nop_();
                _nop_();
                DQ= 1;                                        //DQ 引脚变高 
                for(j=15; j>0; j--);        //极短延时
                if(DQ = 1)                                 //检测DQ引脚上的电平状态 
                {
                        temp = temp | 0x80;
                }
                else
                {
                        temp = temp | 0x00;
                }
                for(j=20; j>0; j--);        //极短延时        
                return temp;                        //返回读到的数据        
        }
         
}





//DS18B20中断申请，读数 
void T0_ISR(void) interrupt 1
{
        uchar x;                                    //数据定义x,result
        uint result;
        TH0 = (65536-fosc/12/timer1ms)/256;                        //T0定时1ms的初值装入TH0，TL0 ,重装初值 
        TL0 = (65536-fosc/12/timer1ms)%256;
        //LED扫描相关程序 
        scnt ++;
        if(scnt == 1000)
        {
                scnt 0;        
                while(resetDS());                                                //复位DS18B20
                writeDS(0xcc);
                writeDS(0xbe);                                                        //向DS18B20发送读命令
                buf[0] = readDS();                                                //从DS18B20内读数 
                buf[1] = readDS();
                                                        //！！！！！！！！！//缓冲显示相关部分:清除缓冲显示？？？？
                                                        
                                                        
                                                        
                sflag = 0;
                if((buf[1] & 0xF8) != 0x00)                                //判断数据是否为负 
                {
                        sflag = 1;
                        buf[1] = ~buf[1];                                        //数据取反，加1处理 
                        buf[0] = ~buf[0];
                        result = buf[0] + 1;
                        buf[0] = result;
                        if(result > 255) buf[1]++; 
                }  
                buf[1] <<= 4;                                                        //转换为有效的数值 
                buf[1] &= 0x70;
                x = buf[0];
                x >>= 4;
                x &= 0x70;
                buf[1] |= x;
                x = 2;
                result = buf[1];
                
                
//        EG:        while(result/10)                        //将有效值送往显示缓冲区域
//                 {
//                         LEDBuffer[x] = result % 10;
//                         result = result / 10;
//                         x ++;        
//                }
//                LEDBuffer[x] = result;
//                if(sflag == 1)                                                        //若是负温度，则在有效值前显示"-"号 
//                                LEDBuffer[x + 1] = 17;
//                x = buf[0] & 0x0f;
//                x <<= 1;
//                LEDBuffer[0] = (dotcode[x]) % 10;                //有效的小数值送显示缓冲区 
//                LEDBuffer[1] = (doctode[x]) / 10;
                while(resetDS());                                                //复位DS18B20b 
                writeDS(0xcc);
                writeDS(0x44);                                                        //发送温度转换命令          
        }  
} 



//LCD 显示函数 
void display(uchar factnum)
{
        
}        
