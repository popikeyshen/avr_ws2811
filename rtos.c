#include "rtos.h"

/******************************************************************************************
 * ���������� ������
 */
volatile static task TaskArray[MAX_TASKS];      // ������� �����
volatile static u08 arrayTail;                  // "�����" �������
extern volatile unsigned int SoftTimer;
extern volatile unsigned char LedSysEnable;
volatile unsigned char CountForSysBlink = 0;


/******************************************************************************************
 * ������������� ����, ����� ���� - 1 ��
 */
inline void RTOS_Init()
{
   TCCR0B       |= (1<<CS01)|(1<<CS00);         // ��������� - 64
   TIFR0         = (1<<TOV0);                   // ������� ���� ���������� ������� �0
   TIMSK0       |= (1<<TOIE0);                  // ��������� ���������� �� ������������
   TIMER_COUNTER = TIMER_START;                 // ��������� ��������� ��. � ������� �������
												// ��� 1ms
   arrayTail = 0;                               // "�����" � 0
}

/******************************************************************************************
 * ���������� ������ � ������
 */
void RTOS_SetTask (void (*taskFunc)(void), u16 taskDelay, u16 taskPeriod)
{
   u08 i;
   
   if(!taskFunc) return;
   for(i = 0; i < arrayTail; i++)                     // ����� ������ � ������� ������
   {
      if(TaskArray[i].pFunc == taskFunc)              // ���� �����, �� ��������� ����������
      {
         DISABLE_INTERRUPT;

         TaskArray[i].delay  = taskDelay;
         TaskArray[i].period = taskPeriod;
         TaskArray[i].run    = 0;   

         RESTORE_INTERRUPT;
         return;                                      // �������, �������
      }
   }

   if (arrayTail < MAX_TASKS)                         // ���� ����� ������ � ������ ��� 
   {                                                  // � ���� �����,�� ���������
      DISABLE_INTERRUPT;
      
      TaskArray[arrayTail].pFunc  = taskFunc;
      TaskArray[arrayTail].delay  = taskDelay;
      TaskArray[arrayTail].period = taskPeriod;
      TaskArray[arrayTail].run    = 0;   

      arrayTail++;                                    // ����������� "�����"
      RESTORE_INTERRUPT;
   }
}

/******************************************************************************************
 * �������� ������ �� ������
 */
void RTOS_DeleteTask (void (*taskFunc)(void))
{
   u08 i;
   
   for (i=0; i<arrayTail; i++)                        // �������� �� ������ �����
   {
      if(TaskArray[i].pFunc == taskFunc)              // ���� ������ � ������ �������
      {
         
         DISABLE_INTERRUPT;
         if(i != (arrayTail - 1))                     // ��������� ��������� ������
         {                                            // �� ����� ���������
            TaskArray[i] = TaskArray[arrayTail - 1];
         }
         arrayTail--;                                 // ��������� ��������� "������"
         RESTORE_INTERRUPT;
         return;
      }
   }
}

/******************************************************************************************
 * ��������� ����, ���������� � main
 */

void RTOS_DispatchTask()
{
   u08 i;
   void (*function) (void);
   for (i=0; i<arrayTail; i++)                        // �������� �� ������ �����
   {
      if (TaskArray[i].run == 1)                      // ���� ���� �� ���������� �������,
      {                                               // ���������� ������, �.�. ��
         function = TaskArray[i].pFunc;               // ����� ���������� ����� 
                                                      // ���������� ������
         if(TaskArray[i].period == 0)                 
         {                                            // ���� ������ ����� 0
            RTOS_DeleteTask(TaskArray[i].pFunc);      // ������� ������ �� ������,
            
         }
         else
         {
            TaskArray[i].run = 0;                     // ����� ������� ���� �������
            if(!TaskArray[i].delay)                   // ���� ������ �� �������� ��������
            {                                         // ������ ��
               TaskArray[i].delay = TaskArray[i].period-1; 
            }                                         // ������ ��� ���� ����� ������� �����  
         }
         (*function)();                               // ��������� ������
      }
   }
}

/******************************************************************************************
 * ��������� ������ ���� (���������� ����������� �������)
 */
ISR(RTOS_ISR) 
{
   u08 i;

   TIMER_COUNTER = TIMER_START;                       // ������ ��������� �������� �������
   
   for (i=0; i<arrayTail; i++)                        // �������� �� ������ �����
   {
      if  (TaskArray[i].delay == 0)                   // ���� ����� �� ���������� �������
           TaskArray[i].run = 1;                      // ������� ���� �������,
      else TaskArray[i].delay--;                      // ����� ��������� �����
   }
   SoftTimer++;
   CountForSysBlink++;
   if(LedSysEnable != DISABLE)
   {
		switch(CountForSysBlink)
		{
			case 100:
				LED_SYS_ON;
				break;
			case 200:
				LED_SYS_OFF;
				CountForSysBlink = 0;
				break;
		
			default:
				break;
		}
	}	
}




