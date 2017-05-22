#ifndef COMMONS_H_
#define COMMONS_H_

#include <avr/io.h>
#include <avr/interrupt.h>

/******************************************************************************************
 * Битовые макросы
 */
#define SetBit(x,y)    do{ x |=  (1 << (y));} while(0)
#define ClrBit(x,y)    do{ x &= ~(1 << (y));} while(0)
#define InvBit(x,y)    do{(x)^=  (1 << (y));} while(0)
#define IsBit(x,y)        (x &   (1 << (y)))

/******************************************************************************************
 * Переопределения типов
 */
typedef unsigned char      u08;
typedef unsigned short     u16;
typedef unsigned long      u32;
typedef unsigned long long u64;

typedef   signed char      s08;
typedef   signed short     s16;
typedef   signed long      s32;
typedef   signed long long s64;

/******************************************************************************************
 * Макросы прерываний
 */
static volatile u08 saveRegister;

#define ENABLE_INTERRUPT do{sei();}while(0)
#define DISABLE_INTERRUPT do{saveRegister = SREG; cli();}while(0)
#define RESTORE_INTERRUPT do{SREG = saveRegister;}while(0)
//использовать RESTORE только после DISABLE


/******************************************************************************************
 * Логические значения
 */
#define TRUE  1
#define FALSE 0

#define HIGH  1
#define LOW   0

#define ON    1
#define OFF   0

#define NULL  0

/******************************************************************************************
 * Максимальные значения типов
 */
#define MAX08U	255
#define MAX16U	65535
#define MAX32U	4294967295

#define MIN08S	-128
#define MAX08S	 127
#define MIN16S	-32768
#define MAX16S	 32767
#define MIN32S	-2147483648
#define MAX32S	 2147483647




/*--------------------------------------------------------------------------*/
#endif /* COMMONS_H_ */
