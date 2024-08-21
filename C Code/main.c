/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------
#define QUEUESIZE 10
// ----------------------------------------------------------------------------
/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"
#include <time.h>
#include <string.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#define CCM_RAM __attribute__((section(".ccmram")))

// Sample paragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
//-----------------------------------------------------------------------------
//Creating Senders,Receiver,Queue and Semaphore handle tasks
TaskHandle_t SenderOneTaskHandle;
TaskHandle_t SenderTwoTaskHandle;
TaskHandle_t SenderThreeTaskHandle;
TaskHandle_t ReceiverTaskHandle;
SemaphoreHandle_t SenderSem[3];
SemaphoreHandle_t ReceiverSem;
QueueHandle_t Queue;
//-----------------------------------------------------------------------------

//Creating Variables
int MessagetransmittedCounter[3]={0};
int MessageBlockedCounter[3]={0};
int MessageReceivedCounter=0;
int iterationsCounter=0;
int TotalBlockedTimeSenderThree;
int upperBoundaries[6]={150,200,250,300,350,400};
int LowerBoundaries[6]={50,80,110,140,170,200};
int TotalTimeSenderOne=0;
int TotalTimeSenderTwo=0;
int TotalTimeSenderThree=0;
int TotalBlockedTimeSenderOne = 0;
int TotalBlockedTimeSenderTwo = 0;

//-----------------------------------------------------------------------------
//Creating Timers and Startup Periods
TimerHandle_t SenderOneTimer;
TimerHandle_t SenderTwoTimer;
TimerHandle_t SenderThreeTimer;
TimerHandle_t ReceiverTimer;
TickType_t SenderOnePeriod;
TickType_t SenderTwoPeriod;
TickType_t SenderThreePeriod;
TickType_t ReceiverTimerPeriod=pdMS_TO_TICKS(100);
BaseType_t SenderOneStartupTime=0;
BaseType_t SenderTwoStartupTime=0;
BaseType_t SenderThreeStartupTime=0;
BaseType_t ReceiverStartupTime=0;
//-----------------------------------------------------------------------------
//Reset Function
void Reset(void)
{
	static int FirstCall=0;
	if(FirstCall!=0)//Not First Call
	{
		int SumTransmitted=0;
		int SumBlocked=0;
		printf("----------------------------------------------------------------------------\n");
		printf("Iteration number : %i\n" , iterationsCounter+1);
		for(int i=0 ;i<3;i++)
		{
			SumTransmitted+=MessagetransmittedCounter[i];
			SumBlocked+=MessageBlockedCounter[i];
		}
		//1-Print the number of total successfully sent messages, blocked messages and statistics for each task
		printf("Total number of successfully sent messages :%i\n",SumTransmitted);
		printf("Total number of blocked messages :%i\n",SumBlocked);
		//2-Print the statistics per sender task (the high priority and the two lower priority tasks)
	    PrintStatistics();
		    iterationsCounter++;
			}
	  else//First Call
		{
		  FirstCall++;
		}
	//3-Reset the total number of successfully sent messages, the total number of blocked messages and received  message
	  for(int i=0 ;i<3;i++)
			{
				MessagetransmittedCounter[i]=0;
				MessageBlockedCounter[i]=0;
			}
	  //3-Reset the total sending time for senders task that used to calculate the average sending time*/
	  TotalTimeSenderOne=0;
	  TotalTimeSenderTwo=0;
	  TotalTimeSenderThree=0;
	  TotalBlockedTimeSenderOne = 0;
	  TotalBlockedTimeSenderTwo = 0;
	  TotalBlockedTimeSenderThree = 0;
	  MessageReceivedCounter=0;
		//4-Clear the queue
			xQueueReset(Queue);
		//5-Configure the sender timer period values using two arrays, If all values are used, print "Game Over" and stop execution.
			if(iterationsCounter >= 6)
			{
				printf(" Game Over !! \n ");
				//Delete all timers
				xTimerDelete(SenderOneTimer , NULL);
				xTimerDelete(SenderTwoTimer , NULL);
				xTimerDelete(SenderThreeTimer, NULL);
				xTimerDelete(ReceiverTimer, NULL);
				exit (0);//exit the program
				vTaskEndScheduler();
			}

		}
//-----------------------------------------------------------------------------
//Function (PrintStatistics) Prints the statistics per sender task (the high priority and the two lower priority tasks)
void PrintStatistics()
{
	//Statistics per sender 1
	int AvgOne=TotalTimeSenderOne/(MessagetransmittedCounter[0]+MessageBlockedCounter[0]);
    int AvgBlockedOne = TotalBlockedTimeSenderOne/(MessagetransmittedCounter[0]+MessageBlockedCounter[0]);
	printf("Number of successfully sent messages for the sender task1 (higher priority) :%i\n",MessagetransmittedCounter[0]);
	printf("Number of Blocked messages for the sender task1 (higher priority) :%i\n",MessageBlockedCounter[0]);
	printf("Total Average sending time for the sender task1 (higher priority) :%i\n",AvgOne);
	printf("Total Average sending time for blocked messages for sender task1 (higher priority):%i\n", AvgBlockedOne);
	//Statistics per sender 2
	int AvgTwo=TotalTimeSenderTwo/(MessagetransmittedCounter[1]+MessageBlockedCounter[1]);
	int AvgBlockedTwo = TotalBlockedTimeSenderTwo/(MessagetransmittedCounter[1]+MessageBlockedCounter[1]);
	printf("Number of successfully sent messages for the sender task2 (Lower priority) :%i\n",MessagetransmittedCounter[1]);
	printf("Number of Blocked messages for the sender task2 (Lower priority) :%i\n",MessageBlockedCounter[1]);
	printf("Total Average sending time for the sender task2 (Lower priority) :%i\n",AvgTwo);
	printf("Total Average sending time for blocked messages for sender task2 :%i\n", AvgBlockedTwo);
	//Statistics per sender 3
	int AvgThree=TotalTimeSenderThree/(MessagetransmittedCounter[2]+MessageBlockedCounter[2]);
	int AvgBlockedThree = TotalBlockedTimeSenderThree/(MessagetransmittedCounter[2]+MessageBlockedCounter[2]);
	printf("Number of successfully sent messages for the sender task3 (Lower priority) :%i\n",MessagetransmittedCounter[2]);
	printf("Number of Blocked messages for the sender task3 (Lower priority) :%i\n",MessageBlockedCounter[2]);
	printf("Total Average sending time for the sender task3 (Lower priority) :%i\n",AvgThree);
	printf("Total Average sending time for blocked messages for sender task3 :%i\n", AvgBlockedThree);
	printf("----------------------------------------------------------------------------\n");

}
//-----------------------------------------------------------------------------
//Callback Functions
void SenderOneTimerCallback(TimerHandle_t Timer)
{
	xSemaphoreGive(SenderSem[0]);//Release Semaphore
}

void SenderTwoTimerCallback(TimerHandle_t Timer)
{
	xSemaphoreGive(SenderSem[1]);//Release Semaphore
}

void SenderThreeTimerCallback(TimerHandle_t Timer)
{
	xSemaphoreGive(SenderSem[2]);//Release Semaphore
}

void ReceiverTimerCallback(TimerHandle_t Timer)
{
	xSemaphoreGive(ReceiverSem);
	if(MessageReceivedCounter>=1000)
	{
		// When the receiver receives 1000 messages,the receiver timer callback function calls Reset()
		Reset();
	}
}
//-----------------------------------------------------------------------------
//Senders and Receiver Tasks Functions
void Sender1Task(void* pvParameters)//Higher priority
{
	TickType_t xStartTime = xTaskGetTickCount();
	while(1)
	{
		xSemaphoreTake(SenderSem[0], portMAX_DELAY);//Take Semaphore released by callback
		if (uxQueueSpacesAvailable(Queue) > 0)//Checks if queue has a free space
		{
			char SenderMessage[20];
			sprintf(SenderMessage, "Time is %lu", xTaskGetTickCount());
			if (xQueueSendToBack(Queue, SenderMessage, 0) == pdPASS)// Send the string to the queue
			{
				MessagetransmittedCounter[0]++;/* Comment this section to hide time of message transmitted by sender 1*/
				TickType_t xCurrentTime = xTaskGetTickCount();
				TickType_t timeSpent = xCurrentTime - xStartTime;
				TotalTimeSenderOne += timeSpent;
				printf("Message transmitted ,Sent by Sender 1 at time: %d ms\n", xCurrentTime);
			}
		}
		else
		{
			MessageBlockedCounter[0]++; /* Comment this section to hide time of message blocked by sender 1*/
			TickType_t xCurrentTime = xTaskGetTickCount();
			TickType_t timeSpent = xCurrentTime - xStartTime;
			TotalTimeSenderOne += timeSpent;
			printf("Message blocked ,Sent by Sender 1 at time: %d ms\n", xCurrentTime);
		}
		//Get a random value using normal distribution

		int SenderRandomValue=RandomNumbersUniformDist();
		TotalTimeSenderOne+=SenderRandomValue;
		TotalBlockedTimeSenderOne += SenderRandomValue;

		SenderOnePeriod=pdMS_TO_TICKS(SenderRandomValue);//Convert the random values to ticks
		xTimerChangePeriod(SenderOneTimer,SenderOnePeriod, 0);//Change the period to the converted random value

	}
}

void Sender2Task(void* pvParameters)//Lower priority
{
	TickType_t xStartTime = xTaskGetTickCount();
	while(1)
	{
		xSemaphoreTake(SenderSem[1], portMAX_DELAY);//Take Semaphore released bt callback
		if (uxQueueSpacesAvailable(Queue) > 0)//Checks if queue has a free space
		{
			char SenderMessage[20];
			sprintf(SenderMessage, "Time is %lu", xTaskGetTickCount());
			if (xQueueSendToBack(Queue, SenderMessage, 0) == pdPASS)// Send the string to the queue
			{
				MessagetransmittedCounter[1]++;/* Comment this section to hide time of message transmitted by sender 2*/
				TickType_t xCurrentTime = xTaskGetTickCount();
				TickType_t timeSpent = xCurrentTime - xStartTime;
				TotalTimeSenderTwo += timeSpent;
				printf("Message transmitted,Sent by Sender 2 at time: %d ms\n", xCurrentTime);
			}
		}
		else
		{
			MessageBlockedCounter[1]++;/* Comment this section to hide time of message blocked by sender 2*/
			TickType_t xCurrentTime = xTaskGetTickCount();
			TickType_t timeSpent = xCurrentTime - xStartTime;
			TotalTimeSenderOne += timeSpent;
			printf("Message blocked ,Sent by Sender 2 at time: %d ms\n", xCurrentTime);
		}
		//Get a random value using normal distribution

		int SenderRandomValue=RandomNumbersUniformDist();
		TotalTimeSenderTwo+=SenderRandomValue;
		TotalBlockedTimeSenderTwo += SenderRandomValue;

		SenderTwoPeriod=pdMS_TO_TICKS(SenderRandomValue);//Convert the random values to ticks
		xTimerChangePeriod(SenderTwoTimer,SenderTwoPeriod, 0);//Change the period to the converted random value

	}
}

void Sender3Task(void* pvParameters)//Lower priority
{
	while(1)
	{
		TickType_t xStartTime = xTaskGetTickCount();
		xSemaphoreTake(SenderSem[2], portMAX_DELAY);//Take Semaphore released bt callback
		if (uxQueueSpacesAvailable(Queue) > 0)//Checks if queue has a free space
		{
			char SenderMessage[20];
			sprintf(SenderMessage, "Time is %lu", xTaskGetTickCount());
			if (xQueueSendToBack(Queue, SenderMessage, 0) == pdPASS)// Send the string to the queue
			{
				MessagetransmittedCounter[2]++;/* Comment this section to hide time of message transmitted by sender 3*/
				TickType_t xCurrentTime = xTaskGetTickCount();
				TickType_t timeSpent = xCurrentTime - xStartTime;
				TotalTimeSenderThree += timeSpent;
				printf("Message transimtted,Sent by Sender 3 at time: %d ms\n", xCurrentTime);
			}
		}
		else
		{
			MessageBlockedCounter[2]++;/* Comment this section to hide time of message transmitted by sender 3*/
			TickType_t xCurrentTime = xTaskGetTickCount();
			TickType_t timeSpent = xCurrentTime - xStartTime;
			TotalTimeSenderOne += timeSpent;
			printf("Message blocked ,Sent by Sender 3 at time: %d ms\n", xCurrentTime);
		}
		//Get a random value using normal distribution

		int SenderRandomValue=RandomNumbersUniformDist();
		TotalTimeSenderThree+=SenderRandomValue;
		TotalBlockedTimeSenderThree+= SenderRandomValue;

		SenderThreePeriod=pdMS_TO_TICKS(SenderRandomValue);//Convert the random values to ticks
		xTimerChangePeriod(SenderThreeTimer,SenderThreePeriod, 0);//Change the period to the converted random value

	}
}

void ReceiverTask(void* pvParameters)
{
	while(1)
	{
		xSemaphoreTake(ReceiverSem, portMAX_DELAY);//Take Semaphore released bt callback
		if (uxQueueMessagesWaiting(Queue) > 0)//Checks if queue has a free space
		{
			char ReceiverString[20];
			if (xQueueReceive(Queue, ReceiverString, 0)==pdPASS)// Send the string to the queue
			{
				MessageReceivedCounter++;/* Comment this section to hide time of message received by Receiver */
				TickType_t xCurrentTime = xTaskGetTickCount();
				printf("Message received at time: %d ms\n", xCurrentTime);
			}

		}

	}
}

//-----------------------------------------------------------------------------
//Function (RandomNumbersUniformDist) generates random numbers using normal distribution
int RandomNumbersUniformDist()
{
	int NumbersRange =upperBoundaries[iterationsCounter]-LowerBoundaries[iterationsCounter]+1;
	return ((rand()%NumbersRange)+LowerBoundaries[iterationsCounter]);
}
//-----------------------------------------------------------------------------
//Function (Initialization) generates Tasks ,Queue ,Semaphores and Timers
void Initialization()
{
	Queue=xQueueCreate(QUEUESIZE,sizeof(char[20]));//Create Queue
	if(Queue!=NULL)
	{
		//Create senders and receiver tasks
		SenderOneTaskHandle=xTaskCreate(Sender1Task,"First Sender",2048,NULL,2,NULL);
		SenderTwoTaskHandle=xTaskCreate(Sender2Task,"Second Sender",2048,NULL,1,NULL);
		SenderThreeTaskHandle=xTaskCreate(Sender3Task,"Third Sender",2048,NULL,1,NULL);
		ReceiverTaskHandle=xTaskCreate(ReceiverTask,"Receiver",2048,NULL,3,NULL);;
	}
	int SenderStartupTime[3];
	for(int i=0;i<3;i++)
	{
		//Get the initial value from the random number generator*/
		SenderStartupTime[i]=RandomNumbersUniformDist();
		SenderSem[i] = xSemaphoreCreateBinary();//Create the senders' semaphore
	}
	ReceiverSem=xSemaphoreCreateBinary();//Create the receiver's semaphore
	//Convert the random values to ticks
	    SenderOnePeriod=pdMS_TO_TICKS(SenderStartupTime[0]);
	    SenderTwoPeriod=pdMS_TO_TICKS(SenderStartupTime[1]);
		SenderThreePeriod=pdMS_TO_TICKS(SenderStartupTime[2]);
	//Create sender timers and assign startup periods
		SenderOneTimer=xTimerCreate( "First Sender Timer", SenderOnePeriod, pdTRUE, ( void * ) 0, SenderOneTimerCallback);
		SenderTwoTimer=xTimerCreate( "Second Sender Timer", SenderTwoPeriod, pdTRUE, ( void * ) 0, SenderTwoTimerCallback);
		SenderThreeTimer=xTimerCreate( "Third Sender Timer", SenderThreePeriod, pdTRUE, ( void * ) 0, SenderThreeTimerCallback);
		ReceiverTimer=xTimerCreate( "Receiver Timer", ReceiverTimerPeriod , pdTRUE, ( void * ) 0, ReceiverTimerCallback);
}
//-----------------------------------------------------------------------------
// ----- main() ---------------------------------------------------------------

int main()
{
	Initialization();
	Reset();
	if( ( SenderOneTimer!= NULL ) && ( SenderTwoTimer != NULL ) && ( SenderThreeTimer != NULL )&& (ReceiverTimer != NULL ) && Queue != NULL )
		{
		SenderOneStartupTime=xTimerStart( SenderOneTimer, 0 );
		SenderTwoStartupTime=xTimerStart( SenderTwoTimer, 0 );
		SenderThreeStartupTime=xTimerStart( SenderThreeTimer, 0 );
		ReceiverStartupTime=xTimerStart(ReceiverTimer, 0 );
		}
		if( SenderOneStartupTime== pdPASS && SenderTwoStartupTime == pdPASS && SenderThreeStartupTime == pdPASS &&ReceiverStartupTime==pdPASS)
		{
			vTaskStartScheduler();
		}
		else
		{
			printf("Error !! \n");
			printf("Can't start Task Scheduler \n");
			exit(0);
		}
		return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------


void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;
/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
