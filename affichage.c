/*
 * affichage.c
 *
 *  Created on: Nov. 25, 2020
 *      Author: kameladi
 */

#include "affichage.h"

volatile uint32_t milisecondCounts = 0U;
volatile unsigned char value[4] = {0};
volatile unsigned char cnt = 0;

void init()
{
	tpm_config_t tpmInfo;

	gpio_pin_config_t config  = {
		    	        kGPIO_DigitalOutput,
		    	        0,
		    	    };

	/* TPM Initialisation */

	/* Select the clock source for the TPM counter as MCGPLLCLK */
	  CLOCK_SetTpmClock(1U);
	  TPM_GetDefaultConfig(&tpmInfo);

	  #ifndef TPM_PRESCALER
	  #define TPM_PRESCALER kTPM_Prescale_Divide_4
	  #endif

	  /* TPM clock divide by TPM_PRESCALER */
	  tpmInfo.prescale = TPM_PRESCALER;

	  /* Initialize TPM module */
	  TPM_Init(BOARD_TPM, &tpmInfo);

	  /*
	   * Set timer period.
	   */
	   TPM_SetTimerPeriod(BOARD_TPM, USEC_TO_COUNT(1000U, TPM_SOURCE_CLOCK));

	   TPM_EnableInterrupts(BOARD_TPM, kTPM_TimeOverflowInterruptEnable);

	   EnableIRQ(BOARD_TPM_IRQ_NUM);

	   TPM_StartTimer(BOARD_TPM, kTPM_SystemClock);

	   /* GPIO Initialisation */
	   GPIO_PinInit(GPIOA, 5, &config);
	    GPIO_PinInit(GPIOA, 1, &config);
	    GPIO_PinInit(GPIOA, 2, &config);
	    GPIO_PinInit(GPIOA, 12, &config);
	    GPIO_PinInit(GPIOE, 25, &config);
	    GPIO_PinInit(GPIOE, 31, &config);
	   // config.outputLogic =0;
	    GPIO_PinInit(GPIOE, 24, &config);
	    GPIO_PinInit(GPIOC, 9, &config);
	    GPIO_PinInit(GPIOB, 3, &config);
	    GPIO_PinInit(GPIOB, 2, &config);
	    GPIO_PinInit(GPIOC, 10, &config);
	    GPIO_PinInit(GPIOC, 8, &config);
	    GPIO_PinInit(GPIOC, 4, &config);
	    GPIO_PinInit(GPIOC, 5, &config);
	    // Point décimal
	    //GPIO_PinWrite(GPIOC, 10, 1);
}

void writeDigit(unsigned char number, unsigned char digitNumber)
{
int d0, d1, d2, d3;

d0 = number & 1;
d1 = (number >> 1) & 1;
d2 = (number >> 2) & 1;
d3 = (number >> 3) & 1;

GPIO_PinWrite(GPIOD, 4, 0);
GPIO_PinWrite(GPIOD, 5, 0);
GPIO_PinWrite(GPIOB, 3, 0);
GPIO_PinWrite(GPIOB, 2, 0);

//Activate the corresponding digit
switch (digitNumber)
{
case 0 : GPIO_PinWrite(GPIOD, 4, 1); break;
case 1 : GPIO_PinWrite(GPIOD, 5, 1); break;
case 2 : GPIO_PinWrite(GPIOB, 3, 1); break;
case 3 : GPIO_PinWrite(GPIOB, 2, 1); break;
}

GPIO_PinWrite(GPIOD, 0, d0);
GPIO_PinWrite(GPIOD, 1, d1);
GPIO_PinWrite(GPIOD, 2, d2);
GPIO_PinWrite(GPIOD, 3, d3);
}

void BOARD_TPM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    TPM_ClearStatusFlags(BOARD_TPM, kTPM_TimeOverflowFlag);
    milisecondCounts ++;
    if (milisecondCounts > 5)
    {
     milisecondCounts = 0;
     writeDigit(value[cnt], cnt);
     cnt++;
     if (cnt==4) cnt = 0;
    }
    __DSB();
}

void writeNumber(float e)
{

 int intTemp;
 char i=0;

 intTemp = e * 10;
 while (i < 4)
 {
	 value[3-i] = intTemp % 10;
	 intTemp /= 10;
	 i++;
 }
}
