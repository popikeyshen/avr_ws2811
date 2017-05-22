#ifndef SEPAR_H_
#define SEPAR_H_

#include <avr/io.h>
#include <avr/interrupt.h>

/******************************************************************************************
 * Назначения
 */
//Byte Names
#define	CTL				0x00
#define	CHK				0x01
#define	SIZE			0x02
#define	CMD				0x03
#define	DATA			0x04
#define	CRC8			0x04
#define	NO_OPERANION	0x00

#define OFF				0x00
#define ON				0x01


//Commands from Master
#define	ACK				0x00
#define	RESET			0x01
#define	POLL			0x02
#define	OPEN_1 			0x03
#define	CLOSE_1			0x04
#define	LEDS		    0x05
#define	NO_LEDS 		0x06

//Commands from Slave
#define	IDLE 			0x00
#define	GENERAL_FAILURE 0x02
#define	DISPENSING		0x05
#define	DISPENSED		0x06
#define	FINISHED		0x06
#define	ERROR			0x07
#define	KEY_PRESS		0x08
#define	KEY_RELEASE		0x09
#define	STAY_ON			0x10
#define	MOVING			0x11
#define	SEND_CLIMATE	0x12
#define	IN_ASCROWING	0x13
#define	IN_NO_ASCROWING	0x14

//Flags
#define	FREE	0
#define BUSY	1
#define	ENABLE	0
#define DISABLE	1

/******************************************************************************************
 * Макросы
 */
#define	MOTOR_STOP	do{PORTC |= (1<<PC3)|(1<<PC4);} while(0)
#define	MOTOR_LEFT	do{MOTOR_STOP; PORTC &= ~(1<<PC3);} while(0)
#define	MOTOR_RIGHT	do{MOTOR_STOP; PORTC &= ~(1<<PC4);} while(0)

#define	LED_SYS_ON	PORTB |= (1<<PB5);
#define	LED_SYS_OFF	PORTB &= ~(1<<PB5);

//#define	LED_SYS1_ON		PORTD |= (1<<PD2);
//#define	LED_SYS1_OFF	PORTD &= ~(1<<PD2);
/******************************************************************************************
 * Прототипы фукнций
 */
void InitPins(void);
void InitUSART(void);
unsigned char CheckCRC(void);
unsigned char CountCRC(unsigned char *pBufferCRC, unsigned char size);
void ReciveNewData(void);
void ProcPOLL(void);
void ProcASCROW(void);
void USART1_SendChar(unsigned char value);
void USART1_SendStr(unsigned char *str);
void MotorInit(void);
void BlinkFailure(void);

void ledoff(void);

#endif
