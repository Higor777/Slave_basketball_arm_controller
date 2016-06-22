#include "stm32f10x.h"
#include "usart2.h"
#include "delay.h"
#include "motorcontrol.h"
#include "bsp.h"
#include "bsp_usart1.h"

 float destpid_angle4;							//上位机输入步进电机一个目的角速度
 int step;                       //步进数，标志位
volatile int flag=0,hold=0;
/********************一号直流电机*********************************/
void direction(int a)        //正为正转，负为反转
{	 

    if(a>0)
	 {
			ln1(1);             //暂时去掉，蜂鸣器太吵
			ln2(1);
	 }
	if(a<0)
	 {

		 ln1(1);
		 ln2(0);
	 }
}

void get_ball()              //得球
{
 hold=0;	
 direction(1);                //上升
 step=8000;
 Stepper_motor();
 while(flag==0);
 flag=0;  	//标志位请零
 
			USART1_SendChar(0xff);			
			USART1_SendChar(0xff);			
			USART1_SendChar(0x02);										
			USART1_SendChar(0x00);	
			USART1_SendChar(0x02);
			USART1_SendChar(0x01);
			USART1_SendChar(0x00);	
			USART1_SendChar(0x05);
			
 TIM8_PWM_Init(999,9999);             //为什么是持球
 step=100;
 hold=1;                       //保持持球位置，现在开始禁止关闭定时器，直到两秒以后
 Stepper_motor();
 delayms(2000);
 hold=0;
 flag=0;
	
 TIM8_PWM_Init(999,10);             //下降
 direction(-1);
 step=7000;                    		 //回来的时候步数小一些，再释放电机
 Stepper_motor();
 while(flag==0); 
 flag=0;
 ln1(0);													//释放电机
}

void get_hold_ball()              //得球并持球状态
{
 direction(1);              //上升
 step=4000;
 TIM8_PWM_Init(999,10);
 Stepper_motor();
 while(flag==0);  
 flag=0;  
	
 TIM8_PWM_Init(999,9999);           // 持球
 step=100;
 hold=1;                       //保持持球位置，现在开始禁止关闭定时器，直到收到getfromhold指令
 Stepper_motor();
}

void down_from_hold()
{
 TIM_Cmd(TIM3, DISABLE);
 TIM_Cmd(TIM8, DISABLE);
 TIM8_PWM_Init(999,10);              //从持球位置下降
 flag=0;                             //标志位清零
 hold=0;                             //解除持球
 direction(-1); 
 step=3000;                          //step值需要测试
 Stepper_motor();
 while(flag==0);  
 flag=0;
 ln1(0);														//释放电机
}
void get_from_hold()              	//从持球位置得球并下降
{	
 TIM_Cmd(TIM3, DISABLE);
 TIM_Cmd(TIM8, DISABLE);
 TIM8_PWM_Init(999,10);              //上升 
 flag=0;                             //标志位清零
 hold=0;                             //解除持球
 direction(1); 
 step=4000;                          
 Stepper_motor();
 while(flag==0);  
 flag=0;
	
 TIM8_PWM_Init(999,9999);           //保持铲球架静止的配置
 step=100;
 hold=1;                            
 Stepper_motor();
 delayms(2000);
 TIM_Cmd(TIM3, DISABLE);
 hold=0;
 flag=0;
	
			USART1_SendChar(0xff);			
			USART1_SendChar(0xff);			
			USART1_SendChar(0x02);										
			USART1_SendChar(0x00);	
			USART1_SendChar(0x02);
			USART1_SendChar(0x01);
			USART1_SendChar(0x00);	
			USART1_SendChar(0x05);
			
 TIM8_PWM_Init(999,10);
 direction(-1);                    //放下铲球架
 step=7000;                        
 Stepper_motor();
 while(flag==0); 
 flag=0;
 ln1(0);													//释放电机
}

void high_lift()              		//上升到最高处
{
 direction(1); 
 step=11000;
 Stepper_motor();
 while(flag==0);  
 flag=0;
}

void high_down()             	 //从最高处落下
{
 direction(-1); 
 step=10000;
 Stepper_motor();
 while(flag==0);  
 flag=0;
 ln1(0);											//释放电机
}


 void Stepper_motor()
	 {
     TIM_Cmd(TIM3, ENABLE);  //使能TIM3
		 TIM_Cmd(TIM8, ENABLE);
   }


