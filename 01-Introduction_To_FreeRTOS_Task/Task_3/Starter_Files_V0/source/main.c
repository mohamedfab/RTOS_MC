/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

#define LED_PORT										PORT_0
#define LED_PIN											PIN0

#define BUTTON_PORT									PORT_0
#define BUTTON_PIN									PIN1

#define ZERO_VALUE									(0u)
#define BUTTON_SCAN_PERIODICITY			(100u)
#define TIME_2000MS									(2000u)
#define TIME_4000MS									(4000u)

typedef enum{
	LED_TOGGLE_100MS,
	LED_TOGGLE_400MS,
	LED_TOGGLE_OFF
}ledToggleStatus_t;

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
/*	Global Variables	*/
TaskHandle_t xHandleLED = NULL;
TaskHandle_t xHandleBUTTON = NULL;
ledToggleStatus_t ledTogglStatus = LED_TOGGLE_OFF;

/*	LED Task Implementation Code	*/
void vidLED_Task(void * pvParameters)
	{
		for(;;)
		{
			switch((uint8_t)ledTogglStatus)
			{
				case LED_TOGGLE_OFF:
						GPIO_write(LED_PORT, LED_PIN, PIN_IS_LOW);
						break;
				case LED_TOGGLE_100MS:
						GPIO_write(LED_PORT, LED_PIN, PIN_IS_HIGH);
						vTaskDelay(100);
						GPIO_write(LED_PORT, LED_PIN, PIN_IS_LOW);
						vTaskDelay(100);
						break;
				case LED_TOGGLE_400MS:
						GPIO_write(LED_PORT, LED_PIN, PIN_IS_HIGH);
						vTaskDelay(400);
						GPIO_write(LED_PORT, LED_PIN, PIN_IS_LOW);
						vTaskDelay(400);
						break;
			}
		}
	}
	
/*	BUTTON Task Implementation Code	*/
void vidBUTTON_Task(void * pvParameters)
		{
				static pinState_t loc_ButtonCurrentStatus;
				static pinState_t loc_ButtonPreviousStatus;
				static uint8_t loc_TimeCounter = ZERO_VALUE;
			for(;;)
			{
					loc_ButtonCurrentStatus = GPIO_read(BUTTON_PORT, BUTTON_PIN);
				/*	Button Is Being Pressed Now	*/
					if (loc_ButtonCurrentStatus == PIN_IS_HIGH)
							{
								/*	Evaluate Pressing Time	*/
								loc_TimeCounter++;
							}
					else
						/*	Button May be Released Or Not Pressed	*/
							{
								/*	Button Is Released	*/
									if (loc_ButtonPreviousStatus == PIN_IS_HIGH)
											{
													/*	If Button Pressed For Time Between 2 and 4 Seconds	*/
														if (((loc_TimeCounter*BUTTON_SCAN_PERIODICITY) >= TIME_2000MS)&&
																((loc_TimeCounter*BUTTON_SCAN_PERIODICITY) < TIME_4000MS))
																{
																	ledTogglStatus = LED_TOGGLE_100MS;
																}
																	/*	If Button Pressed For Time Greater Than 4 Seconds	*/
											 else if ((loc_TimeCounter*BUTTON_SCAN_PERIODICITY) >= TIME_4000MS)
																{
																	ledTogglStatus = LED_TOGGLE_400MS;
																}				
																/*	If Button Pressed For Time Lower Than 2 Seconds	*/
													else
																{
																	ledTogglStatus = LED_TOGGLE_OFF;
																}						
																	/*	Reset Time Evaluation Counter	*/
													loc_TimeCounter = ZERO_VALUE;
											}		
									else
											{
													/*	Do Nothing	*/
											}
							}		
				loc_ButtonPreviousStatus = loc_ButtonCurrentStatus;
				vTaskDelay(BUTTON_SCAN_PERIODICITY);			
			}
		}
	
/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
    /* Create Tasks here */
	  /* Create the task LED, storing the handle. */
	  (void)xTaskCreate(
                    vidLED_Task,       	/* Function that implements the task. */
                    "LED_TASK",          /* Text name for the task. */
                    100,      								/* Stack size in words, not bytes. */
                    ( void * ) 1,    				/* Parameter passed into the task. */
                    1,											/* Priority at which the task is created. */
                    &xHandleLED);
		/* Create the task BUTTON, storing the handle. */
		(void)xTaskCreate(
                    vidBUTTON_Task,       	/* Function that implements the task. */
                    "BUTTON_TASK",          /* Text name for the task. */
                    100,      								/* Stack size in words, not bytes. */
                    ( void * ) 1,    				/* Parameter passed into the task. */
                    1,											/* Priority at which the task is created. */
                    &xHandleBUTTON);
	
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


