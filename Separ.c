#include <avr/io.h>
#include <avr/interrupt.h>
#include "rtos.h"
#include "Separ.h"

#include <util/delay.h>
#include "light_ws2812.h"
//#include "Adafruit_NeoPixel.h"

// DDR - "1" -->, "0" <--
//A3 - Relay1
//A4 - Relat2
//A5 - sw_sensor

volatile unsigned char BufferForUART[32];
unsigned char BufferForState[32];
unsigned char CountReciveByte   	= 0;
unsigned char NoRepetitionByte  	= 1;
unsigned char ASK[4]		    	= {0xF7, 0x00, 0x00, 0xF7};
unsigned char NAK[4]		    	= {0xF7, 0x00, 0xFF, 0xF6};
unsigned char Version[4]			= {0xF7, 0x00, 0x01, 0xF8};
unsigned char BusyFlag          	= BUSY;
unsigned char WorkState         	= NO_OPERANION;
unsigned char CountByteBuffer   	= 2;
volatile unsigned char LedSysEnable = DISABLE;
volatile unsigned int SoftTimer	 	= 0;

/**********************************************************************************/
void InitPins(void)
{
	DDRD &= ~(1<<PD6);//на вход
	DDRD &= ~(1<<PD7);//на вход
}
/**********************************************************************************/
void InitUSART(void)
{
	UCSR0B |= (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01);	
	UBRR0L = 0x67;
	UBRR0H = 0x00;
}
/**********************************************************************************/

unsigned char CheckCRC(void)//проверяем КС
{
	unsigned char CountByte;

	for(CountByte = 0; CountByte <= (BufferForUART[SIZE] + 4); CountByte++)
	{
		BufferForUART[CRC8 + (BufferForUART[SIZE])] -= BufferForUART[CountByte];
	}
	return BufferForUART[CRC8 + (BufferForUART[SIZE])];
}
/**********************************************************************************/
unsigned char CountCRC(unsigned char *pBufferCRC, unsigned char size)//считаем КС
{
	unsigned char i;
	unsigned char Result = 0;
	
	for(i = 0; i <= (size - 1); i++)
	{
		Result += pBufferCRC[i];
	}
	return Result;
}


//    [ CTL ][ CHK ][ SIZE ][ CMD ][ DATA ][ CRC ]

#define MAXPIX 30
#define COLORLENGTH MAXPIX/2
#define FADE 256/COLORLENGTH

struct cRGB colors[8];
struct cRGB led[MAXPIX];

int RGB=1;

////////////////////////////////////// RGB INIT //////////////////////////////
void rgb_leds_init()
{
	uint8_t j = 1;
	uint8_t k = 1;

	DDRB|=_BV(ws2812_pin);
		
    uint8_t i;
    for(i=MAXPIX; i>0; i--)
    {    
        led[i-1].r=0;led[i-1].g=0;led[i-1].b=0;
    }
		int del=10;

     //Rainbowcolors
    colors[0].r=150/del; colors[0].g=150/del; colors[0].b=150/del;
    colors[1].r=255/del; colors[1].g=000; colors[1].b=000;           //red
    colors[2].r=255/del; colors[2].g=100/del; colors[2].b=000;       //orange
    colors[3].r=100/del; colors[3].g=255/del; colors[3].b=000;       //yellow
    colors[4].r=000; colors[4].g=255/del; colors[4].b=000;           //green
    colors[5].r=000; colors[5].g=100/del; colors[5].b=255/del;       //light blue (tьrkis)
    colors[6].r=000; colors[6].g=000; colors[6].b=255/del;           //blue
    colors[7].r=100/del; colors[7].g=000; colors[7].b=255/del;       //violet
    
	while(1)
    {
        //shift all vallues by one led
        uint8_t i=0;           
        for(i=MAXPIX; i>1; i--) 
            led[i-1]=led[i-2];
        //change colour when colourlength is reached   
        if(k>COLORLENGTH)
        {
            j++;
            if(j>7)
            {
              j=0;
            }

            k=0;
        }
        k++;
        //loop colouers
        
        //fade red
        if(led[0].r<(colors[j].r-FADE))
            led[0].r+=FADE;
            
        if(led[0].r>(colors[j].r+FADE))
            led[0].r-=FADE;

        if(led[0].g<(colors[j].g-FADE))
            led[0].g+=FADE;
            
        if(led[0].g>(colors[j].g+FADE))
            led[0].g-=FADE;

        if(led[0].b<(colors[j].b-FADE))
            led[0].b+=FADE;
            
        if(led[0].b>(colors[j].b+FADE))
            led[0].b-=FADE;

		 _delay_ms(10);
		 ws2812_sendarray((uint8_t *)led,MAXPIX*3);
    }
}


//////////////////////////////////// READ BUFER ///////////////////////////////

		char num_start= 0;
		char num_end= 0;
		char r= 0;
		char g= 0;
		char b= 0;

void rgb_read_data()
{
	uint8_t j = 1;
	uint8_t k = 1;
	

	DDRB|=_BV(ws2812_pin);

  
   

	for(int i=0; i<MAXPIX; i++)
		{
			led[i].r=0; led[i].g=0;  led[i].b=0;

		}

		int rr=r;
		int gg=g;
		int bb=b;
		int start=num_start;
		int end=num_end;

		for(int i=start; i<=end; i++)
		{
			led[i].r=rr; led[i].g=gg;  led[i].b=bb;

			//led[i].r=100; led[i].g=100;  led[i].b=100;
			//BufferForUART[DATA]
		}

		 _delay_ms(10);
		 ws2812_sendarray((uint8_t *)led,MAXPIX*3);
    
}


/**********************************************************************************/
void ReciveNewData(void)
{	

	if(!(CheckCRC()))
	{


		if( BufferForUART[CMD] != RESET  )//? (NoRepetitionByte != BufferForUART[CHK]) : 1
		{

			switch(BufferForUART[CMD])
			{
				case RESET:
					asm ("rjmp 0");
					break;

			//	case POLL:
			//		RTOS_SetTask(ProcPOLL, 0, 0);
			//		break;

				case OPEN_1:

					if(PIND & ~(1<<PD6))//PIND == (1<<PD6)
					{
						DDRC =  0x00;
   						PORTC = 0x00;
						//////////////////
						DDRC =   (1<<PC4);
   						PORTC = ~(1<<PC4); 
					}
					else
					{
						DDRC =  0x00;
   						PORTC = 0x00;
						USART1_SendStr(NAK);
					}
					break;

				case CLOSE_1:

					
					if(PIND & ~(1<<PD7))//
					{
						DDRC =  0x00;
   						PORTC = 0x00;
						///////////////////
						DDRC =   (1<<PC5);
   						PORTC = ~(1<<PC5); 
					}
					else
					{
						DDRC =  0x00;
   						PORTC = 0x00;
						USART1_SendStr(NAK);
					}
					break;

//--------------------------------------------------------leds
				case LEDS:
					
						RTOS_DeleteTask( rgb_read_data);
						num_start= BufferForUART[DATA];
						num_end= BufferForUART[DATA+1];
						r= BufferForUART[DATA+2];
						g= BufferForUART[DATA+3];
						b= BufferForUART[DATA+4];
					//	RTOS_SetTask( rgb_read_data, 0, 10);
					break;

				case NO_LEDS:
			
						//num_start= 0;
						//num_end= MAXPIX;
						//r= 0;
						//g= 0;
						//b= 0;

					break;
//------------------------------------------------------ end leds				

				default:
					break;
			}
			NoRepetitionByte = BufferForUART[CHK];
		}
		//BufferForUART[CTL] = 0;
		//UCSR0B |= (1<<RXCIE0);
	}
	BufferForUART[CTL] = 0;
	UCSR0B |= (1<<RXCIE0);
}


/**********************************************************************************/
void ProcPOLL(void)
{
	if(WorkState != NO_OPERANION)
	{
		BufferForState[CountByteBuffer++] = WorkState;
		BufferForState[CountByteBuffer++] = 0;
		BufferForState[CountByteBuffer++] = 0;
	}
	if(CountByteBuffer == 2)
	{
		USART1_SendStr(ASK);
	}
	else
	{
		BufferForState[CTL] = 0xF7;
		BufferForState[SIZE - 1] = CountByteBuffer - 3;
		BufferForState[CountByteBuffer] = CountCRC(BufferForState, (CountByteBuffer - 1));
		USART1_SendStr(BufferForState);
		CountByteBuffer = 2;
	}
}
/**********************************************************************************/
void ProcASCROW(void)
{
	static unsigned char AscrowStateMachine = 0;

	switch(AscrowStateMachine)
	{
		case 0:
			BusyFlag = BUSY;//включаем мотор, если не выполняетя движение
			if(WorkState == IN_ASCROWING)
				MOTOR_RIGHT;
			else if (WorkState == IN_NO_ASCROWING)
				MOTOR_LEFT;
			SoftTimer = 0;
			AscrowStateMachine = 1;
			LED_SYS_ON;
			RTOS_SetTask(ledoff, 500, 0);
			break;

		case 1:
			if(SoftTimer >= 2500)//время для надатия концевика
			{
				AscrowStateMachine = 2;
				SoftTimer = 0;
			}
			break;

		case 2:
			if(PINC & (1<<PC5))//ждем, когда концевиек отожмется
			{
				MOTOR_STOP;
				AscrowStateMachine = 0;
				RTOS_DeleteTask(ProcASCROW);
				BusyFlag  = FREE;
				WorkState = NO_OPERANION;
				break;
			}
			if(SoftTimer >= 5000)//таймаут для определения сработки концевика
			{
				AscrowStateMachine = 3;
				break;
			}
			break;

		case 3:
			MOTOR_STOP;
			WorkState = GENERAL_FAILURE;//ошибка, если концевик так и не сработал(сломался)
			RTOS_DeleteTask(ProcASCROW);
			RTOS_SetTask(BlinkFailure, 0, 10);
			LedSysEnable = DISABLE;
			break;
			

		default:
			break;
	}
}
/**********************************************************************************/
void ledoff(void)
{
	LED_SYS_OFF;
}
/**********************************************************************************/
void BlinkFailure(void)//мигаем по 3 раза в случае поломки концевика
{
	static unsigned char FailureStateMachine = 0;
	static unsigned char CountBlink = 0;

	switch(FailureStateMachine)
	{
		case 0:
			LED_SYS_ON;
			SoftTimer = 0;
			FailureStateMachine = 1;
			break;

		case 1:
			if(SoftTimer >= 200)
			{
				LED_SYS_OFF;
				SoftTimer = 0;
				FailureStateMachine = 2;
			}
			break;

		case 2:
			if(SoftTimer >= 200)
			{
				CountBlink++;
				if(CountBlink >= 3)
				{
					SoftTimer = 0;
					FailureStateMachine = 3;
					CountBlink = 0;
					break;
				}
				else
					FailureStateMachine = 0;
			}
			break;

		case 3:
			if(SoftTimer >= 1000)
			{
				FailureStateMachine = 0;
			}
			break;

		default:
			break;
			
	}
}


/**********************************************************************************/
void USART1_SendChar(unsigned char value)
{
	while(!(UCSR0A & (1<<UDRE0))){;}
		UDR0 = value;
}
/*
[ CTL ][ SIZE ][ CMD ][ DATA ][ CRC ]
                      |_size_|
|_____________crc____________|
*/
void USART1_SendStr(unsigned char *str)
{
	unsigned char CountChar = 0;
	unsigned char NumberOfBytes = ((str[SIZE - 1]) + 3);

	for(CountChar = 0; CountChar <= NumberOfBytes; CountChar++)
	{ 
		USART1_SendChar(*str++);
	}
}
/**********************************************************************************/
void MotorInit(void)
{
	static unsigned char MotorInitStateMachine = 0;

	switch(MotorInitStateMachine)
	{
		case 0:
			if(PINC & (1<<PC5))					//если концевик в исходном - уходим
			{
				MOTOR_STOP;
				BusyFlag = FREE;
				RTOS_DeleteTask(MotorInit);
				LedSysEnable = ENABLE;
				break;
			}
			else
			{
				MotorInitStateMachine = 1;
				break;
			}
			break;
		
		case 1:
			SoftTimer = 0;
			MOTOR_LEFT;
			MotorInitStateMachine = 2;
			break;

		case 2:
			if(PINC & (1<<PC5))//ждем, когда концевиек отожмется
			{
				MOTOR_STOP;
				MotorInitStateMachine = 0;
				RTOS_DeleteTask(MotorInit);
				BusyFlag  = FREE;
				WorkState = NO_OPERANION;
				break;
			}
			if(SoftTimer >= 5000)//таймаут для определения сработки концевика
			{
				MOTOR_STOP;
				WorkState = GENERAL_FAILURE;//ошибка, если концевик так и не сработал(сломался)
				RTOS_SetTask(BlinkFailure, 0, 10);
				RTOS_DeleteTask(MotorInit);
				LedSysEnable = DISABLE;
				break;
			}
			break;			

		default:
			break;
	}

}


/**********************************************************************************/
int main(void)
{
	InitPins();
	LED_SYS_OFF;
	InitUSART();
	RTOS_Init();
	sei();
	USART1_SendStr(Version);

	RTOS_SetTask(MotorInit, 0, 10);
	//RTOS_SetTask( rgb_leds_init, 0, 10);

   
	while(1)
	{
		rgb_read_data();
			
		if(PIND & (1<<PD6))
		{
					DDRC  =  0x00;
   					PORTC = 0x00;
		}
		if(PIND & (1<<PD7))
		{
					DDRC  =  0x00;
   					PORTC = 0x00;
		}
	}

}

/**********************************************************************************/
/*                                   Прерывания                                   */
/**********************************************************************************/
/*
[ CTL ][ CHK ][ SIZE ][ CMD ][ DATA ][ CRC ]
                             |_size_|
|________________ crc ______________|
*/
ISR(USART_RX_vect)
{
	
	BufferForUART[CountReciveByte++] = UDR0;
	if(BufferForUART[CTL] == 0xF7)
	{//                           5 - kol-vo bayt
		if((BufferForUART[SIZE] + 5) == CountReciveByte)//если приняли весь пакет(5 - это клово байт минус data и плюс size т.е. плюс колво data)
		{
			CountReciveByte = 0;
			UCSR0B &= ~(1<<RXCIE0);
			ReciveNewData();
			return;				
		}	
		if(CountReciveByte==31)
		{
			CountReciveByte = 0;
		}

	}
	else
	{
		CountReciveByte = 0;
		//LED_SYS_ON;
		//RTOS_SetTask(ledoff, 500, 0);
	}
}



