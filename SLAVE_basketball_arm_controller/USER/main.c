#include "stm32f10x.h"
#include "string.h"
#include "delay.h"	
#include "queue.h"
#include "usart2.h"			 //usart and 485
#include "motorcontrol.h"
#include "bsp.h"
#include "bsp_usart1.h"

#define MAX_DATA_LEN 100

volatile Queue rx_queue;
static int noselect;			//命令模式选择标记
float x,y,w,distance1,distance2,distance3,distance4;								//坐标和传感器距离
extern float destpid_angle4;            //步进电机速
volatile int hehe;

uint8_t radBufferRS485[99];  		 	//记录转角
void handle_data(uint8_t buf[], int D,int len)    
{
	 
		switch(D)
		{ 
      case 2:      if(buf[0]==1) 
		            	  {
                      if(buf[1]==1) {noselect = 0x04 ;break;}       //get_ball
			           else if(buf[1]==2) {noselect = 0x05 ;break;}       //get_hold_ball
							   else if(buf[1]==3) {noselect = 0x06 ;break;}       //get_from_hold
						     else if(buf[1]==4) {noselect = 0x07 ;break;}       //high_down
							   else if(buf[1]==5) {noselect = 0x08 ;break;}       //high_lift
						     else if(buf[0]==6) {noselect = 0x09 ;break;}       //down_from_hold从持球处落下
			              }
			case 3:      if(buf[0]==1) {noselect = 0x0a ;break;}	      //投球   此处CASE要和协议配合
			
			case 4:      if(buf[0]==1) {noselect = 0x0b ;break;}        //返回四个传感器距离
			
			default:break;
		}
	    
}  


void shot()            //射球
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);	
	delayms(1500);
	GPIO_SetBits(GPIOB,GPIO_Pin_15);	
	delayms(1500);
}

void send(void)
{
#define d1 distance1 
#define d2 distance2 
#define d3 distance3 
#define d4 distance4
	int i;
	char byte[4];		   					
	uint16_t checksumsend1=0x16;				 //校验位初始值（返回传感器距离1 | 返回位置2）
	RS485_TX_MODE   //TX模式
	if(USART_GetFlagStatus(USART1, USART_FLAG_TC)!=RESET)//发送完成标志位//
	{	
      if(noselect==0x04) 							//得球		 
		{
		  get_ball();
          distance1++;			
						
		 noselect=0;
	 	}
		
 if(noselect==0x05) 							//得球并保持持球状态
		{
		  get_hold_ball();				
			USART1_SendChar(0xff);			
			USART1_SendChar(0xff);			
			USART1_SendChar(0x02);										
			USART1_SendChar(0x00);	
			USART1_SendChar(0x02);
			USART1_SendChar(0x01);
			USART1_SendChar(0x00);	
			USART1_SendChar(0x05);						
			noselect=0;
	 	}
	if(noselect==0x06) 							//从持球位置得球			 
		{
		  get_from_hold();				
//			USART1_SendChar(0xff);			
//			USART1_SendChar(0xff);			
//			USART1_SendChar(0x02);										
//			USART1_SendChar(0x00);	
//			USART1_SendChar(0x02);
//			USART1_SendChar(0x01);
//			USART1_SendChar(0x00);	
//			USART1_SendChar(0x05);							
			noselect=0;
	 	}
	if(noselect==0x07) 							//high_down		 
	{				
			USART1_SendChar(0xff);			
			USART1_SendChar(0xff);			
			USART1_SendChar(0x02);										
			USART1_SendChar(0x00);	
			USART1_SendChar(0x02);
			USART1_SendChar(0x01);
			USART1_SendChar(0x00);	
			USART1_SendChar(0x05);
				
		  high_down();
			noselect=0;
	 	}
		if(noselect==0x08) 							//high_lift	 
		{				
			USART1_SendChar(0xff);			
			USART1_SendChar(0xff);			
			USART1_SendChar(0x02);										
			USART1_SendChar(0x00);	
			USART1_SendChar(0x02);
			USART1_SendChar(0x01);
			USART1_SendChar(0x00);	
			USART1_SendChar(0x05);     
			high_lift();			
			noselect=0;
	 	}
		if(noselect==0x09) 							//down_from_hold
		{
			USART1_SendChar(0xff);			
			USART1_SendChar(0xff);			
			USART1_SendChar(0x02);										
			USART1_SendChar(0x00);	
			USART1_SendChar(0x02);
			USART1_SendChar(0x01);
			USART1_SendChar(0x00);	
			USART1_SendChar(0x05);     
			down_from_hold();
			noselect=0;
		}			

 if(noselect==0x0a) 							//投球			 
		{
		  shot();				
			USART1_SendChar(0xff);			
			USART1_SendChar(0xff);			
			USART1_SendChar(0x03);										
			USART1_SendChar(0x00);	
			USART1_SendChar(0x02);
			USART1_SendChar(0x01);
			USART1_SendChar(0x00);	
			USART1_SendChar(0x06);
			noselect=0;
	 	}
		
	if(noselect==0x0b) 							//红外测距			 
		{
		  //LED2(ON);				
			USART1_SendChar(0xff);			
			USART1_SendChar(0xff);			
			USART1_SendChar(0x04);										
			USART1_SendChar(0x00);	
			USART1_SendChar(0x11);
			USART1_SendChar(0x01);	
		  memcpy( byte,&d1,sizeof(float));
		   
			for(i=0;i<4;i++)
			 {
				USART1_SendChar(byte[i]);
				checksumsend1=checksumsend1+byte[i];
			 }		 
		  	memcpy( byte,&d2,sizeof(float));
			for(i=0;i<4;i++)
			 {
			  USART1_SendChar(byte[i]);			
			  checksumsend1=checksumsend1+byte[i];
			 }	
		  	memcpy( byte,&d3,sizeof(float));
			
			for(i=0;i<4;i++)
			 {
				USART1_SendChar(byte[i]);		
				checksumsend1=checksumsend1+byte[i];
			 }
		  	memcpy( byte,&d4,sizeof(float));
	    for(i=0;i<4;i++)
			 {
			  USART1_SendChar(byte[i]);			
			  checksumsend1=checksumsend1+byte[i];
			 }
			checksumsend1=checksumsend1%(0xff);
		
			USART1_SendChar(checksumsend1);						
			noselect=0;
	 	}
		 
     }
    	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);				
	//	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		RS485_RX_MODE	 //RX模式			
}


/*******************************通信部分要求高速，波特率1000000
上位机传来的数据已循环队列的方式在主函数中过状态机，以保证无数据丢失
**********************************************************/
int main()
{	

	static uint16_t checksum;
	uint8_t datalen1;
  int device;
	short data_len;
	int cur=0;
	uint8_t data;
	uint8_t  buffer[MAX_DATA_LEN] = {0};

  uint16_t adcx;
  uint8_t k=4,distance=0;
  uint16_t b=166;
  uint16_t m=34404;
  
    enum Recstate								   		//状态机 
	{
		RECFF1,RECFF2,SENDID,RECID,RECLEN1,RECLEN2,RECSEL,RECCHECK
	} rs485Recstate = RECFF1;
   delayinit(72);	   
	usart1_config(115200);  //usart1初始化
	BSPINIT();
	RS485_RX_MODE			
	queue_init(&rx_queue);

	//USART1_SendChar(0xff);
	
	while(1)	
	{
      if(hehe==1)
			{
				
				hehe=0;
				adcx=Get_Adc_Average(ADC_Channel_1,10);
                distance=(m/(adcx-b))-k;
				if(distance<=10)                           //报警，距离球小于10cm
				{
				  // USART1_SendChar(rx);
				  // USART1_SendChar(rx);
					// USART1_SendChar(rx);
				  // USART1_SendChar(rx);
					// USART1_SendChar(rx);
				  // USART1_SendChar(rx);
					// USART1_SendChar(rx);
				  // USART1_SendChar(rx);
				}
       
       }				
		if(queue_empty(rx_queue))
			            continue;

		data = (uint8_t)queue_get(&rx_queue);
  
		switch (rs485Recstate) 
		{
			case RECFF1:
				if (data == 0xff)	  
				{	
					rs485Recstate = RECFF2;	
					checksum=0;		
					cur = 0;								//校验位清零
 
                }
	
				break;
	
			case RECFF2:
				if (data == 0xff)
					rs485Recstate = RECID;
			    else
					rs485Recstate = RECFF1;
		
				break;
	
	
			case RECID:				 					
//		     if(data==0x01)	                       //设备编号0x01底盘与航迹
//		        {
//				    device=1;
//					    checksum += data;
//					    rs485Recstate = RECLEN1;
//							
//                   }
		
			 if(data==0x02)                       //设备编号0x02捡球 
					{
						device=2;
					    checksum += data;
					    rs485Recstate = RECLEN1;
				      }
			else if(data==0x03)                        //设备编号0x03投球 
				     {
							device=3;
					    checksum += data;
					    rs485Recstate = RECLEN1;
				      }
			else if(data==0x04) 			              //设备编号0x04 测距
				    {
				        device=4;
					    checksum += data;
					    rs485Recstate = RECLEN1;
				   }
			else
				 rs485Recstate = RECFF1;	  
				break;
	
			case RECLEN1:				 				
				checksum += data;
				datalen1 = data;
				rs485Recstate = RECLEN2;	  
				break;
		
			case RECLEN2:	
				checksum += data;			 				
				data_len = (short)datalen1 << 8	 | data;
				rs485Recstate = RECSEL;	  
				break;
	
			case RECSEL:
				checksum += data;
				buffer[cur++] = data;
				if(cur >= data_len)
					rs485Recstate = RECCHECK;
				break;
	
			case RECCHECK:
				checksum=checksum%255;
				if(data == checksum)
				{				
					handle_data(buffer,device, data_len);
  				    send();				
					checksum=0;	
					rs485Recstate = RECFF1;	 
				}
				else
					rs485Recstate = RECFF1;
				break;
			 default:
					rs485Recstate = RECFF1;
			}	   
	   		 
	}		
		     
}



/***********************
初始化成功 led1亮    
通信成功   led2亮

 ff ff 69 04  len1 len2 指令 *********** chenk

case 1: noselect = 0x01 ;break;		   //控制三个电机的角速度
case 2: noselect = 0x02 ;break;		   //得到三个电机的角速度
case 3: noselect = 0x03 ;break;	       //重置编码器数据		 
case 4: noselect = 0x04 ;break;	       //读取x,y,w坐标	



发送 ff ff 69 04 00 0d   01   （一号）**** （二号）**** （三号）****	  chek  控制三个电机到指定角速度
返回 ff ff 04 69 00 02   01     0c	   7c (chk) 

发送 ff ff 69 04 00 01   02    74(chenk)                                         读取三个角速度
返回 ff ff 04 69 00 0d   02   （一号）**** （二号）**** （三号）****	   chek 

发送 ff ff 69 04 00 01	 03	   75(chk)						   重置编码器数据	
返回 ff ff 04 69 00	02	 03     0c    7e（chk）

发送 ff ff 69 04 00 01	 04	   76(chk)					 读取x,y,w坐标
返回 ff ff 04 69 00 0d   04   （一号）**** （二号）**** （三号）****	   chek 

//	   	RS485_TX_MODE   //TX模式
//	    printf("\r\n destpid_angle1=%f destpid_angle2=%f destpid_angle3=%f\r\n ",destpid_angle1, destpid_angle2,destpid_angle3); 
//	    printf("\r\n AV1=%lf AV2=%lf  AV3=%lf \r\n ", AV1 , AV2, AV3);
//      printf("\r\n destpid_length1=%f destpid_length2=%f  destpid_length3=%f \r\n ",destpid_length1,destpid_length2,destpid_length3); 
//      printf("\r\n past_length1=%f past_length2=%f  past_length3=%f \r\n ",past_length1,past_length2,past_length3); 
//	    printf("\r\n x=%f y=%f  w=%f \r\n ",x,y,w); 
//       printf("\r\n rlength1=%f rlength2=%f  rlength3=%f \r\n ",rlength1,rlength2,rlength3);		
// 	    RS485_RX_MODE   //RX模式
//	    NOW_TIME=GET_Nowtime();

*****************************/
