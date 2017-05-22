#include "rtos.h"

/******************************************************************************************
 * Переменные модуля
 */
volatile static task TaskArray[MAX_TASKS];      // очередь задач
volatile static u08 arrayTail;                  // "хвост" очереди
extern volatile unsigned int SoftTimer;
extern volatile unsigned char LedSysEnable;
volatile unsigned char CountForSysBlink = 0;


/******************************************************************************************
 * Инициализация РТОС, время тика - 1 мс
 */
inline void RTOS_Init()
{
   TCCR0B       |= (1<<CS01)|(1<<CS00);         // прескалер - 64
   TIFR0         = (1<<TOV0);                   // очищаем флаг прерывания таймера Т0
   TIMSK0       |= (1<<TOIE0);                  // разрешаем прерывание по переполнению
   TIMER_COUNTER = TIMER_START;                 // загружаем начальное зн. в счетный регистр
												// для 1ms
   arrayTail = 0;                               // "хвост" в 0
}

/******************************************************************************************
 * Добавление задачи в список
 */
void RTOS_SetTask (void (*taskFunc)(void), u16 taskDelay, u16 taskPeriod)
{
   u08 i;
   
   if(!taskFunc) return;
   for(i = 0; i < arrayTail; i++)                     // поиск задачи в текущем списке
   {
      if(TaskArray[i].pFunc == taskFunc)              // если нашли, то обновляем переменные
      {
         DISABLE_INTERRUPT;

         TaskArray[i].delay  = taskDelay;
         TaskArray[i].period = taskPeriod;
         TaskArray[i].run    = 0;   

         RESTORE_INTERRUPT;
         return;                                      // обновив, выходим
      }
   }

   if (arrayTail < MAX_TASKS)                         // если такой задачи в списке нет 
   {                                                  // и есть место,то добавляем
      DISABLE_INTERRUPT;
      
      TaskArray[arrayTail].pFunc  = taskFunc;
      TaskArray[arrayTail].delay  = taskDelay;
      TaskArray[arrayTail].period = taskPeriod;
      TaskArray[arrayTail].run    = 0;   

      arrayTail++;                                    // увеличиваем "хвост"
      RESTORE_INTERRUPT;
   }
}

/******************************************************************************************
 * Удаление задачи из списка
 */
void RTOS_DeleteTask (void (*taskFunc)(void))
{
   u08 i;
   
   for (i=0; i<arrayTail; i++)                        // проходим по списку задач
   {
      if(TaskArray[i].pFunc == taskFunc)              // если задача в списке найдена
      {
         
         DISABLE_INTERRUPT;
         if(i != (arrayTail - 1))                     // переносим последнюю задачу
         {                                            // на место удаляемой
            TaskArray[i] = TaskArray[arrayTail - 1];
         }
         arrayTail--;                                 // уменьшаем указатель "хвоста"
         RESTORE_INTERRUPT;
         return;
      }
   }
}

/******************************************************************************************
 * Диспетчер РТОС, вызывается в main
 */

void RTOS_DispatchTask()
{
   u08 i;
   void (*function) (void);
   for (i=0; i<arrayTail; i++)                        // проходим по списку задач
   {
      if (TaskArray[i].run == 1)                      // если флаг на выполнение взведен,
      {                                               // запоминаем задачу, т.к. во
         function = TaskArray[i].pFunc;               // время выполнения может 
                                                      // измениться индекс
         if(TaskArray[i].period == 0)                 
         {                                            // если период равен 0
            RTOS_DeleteTask(TaskArray[i].pFunc);      // удаляем задачу из списка,
            
         }
         else
         {
            TaskArray[i].run = 0;                     // иначе снимаем флаг запуска
            if(!TaskArray[i].delay)                   // если задача не изменила задержку
            {                                         // задаем ее
               TaskArray[i].delay = TaskArray[i].period-1; 
            }                                         // задача для себя может сделать паузу  
         }
         (*function)();                               // выполняем задачу
      }
   }
}

/******************************************************************************************
 * Таймерная служба РТОС (прерывание аппаратного таймера)
 */
ISR(RTOS_ISR) 
{
   u08 i;

   TIMER_COUNTER = TIMER_START;                       // задаем начальное значение таймера
   
   for (i=0; i<arrayTail; i++)                        // проходим по списку задач
   {
      if  (TaskArray[i].delay == 0)                   // если время до выполнения истекло
           TaskArray[i].run = 1;                      // взводим флаг запуска,
      else TaskArray[i].delay--;                      // иначе уменьшаем время
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




