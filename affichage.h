/*
 * affichage.h
 *
 *  Created on: Nov. 25, 2020
 *      Author: kameladi
 */

#include "fsl_tpm.h"
#include "fsl_gpio.h"


/* define instance */
#define BOARD_TPM TPM2
/* Interrupt number and interrupt handler for the TPM instance used */
#define BOARD_TPM_IRQ_NUM TPM2_IRQn
#define BOARD_TPM_HANDLER TPM2_IRQHandler
/* Get source clock for TPM driver */
#define TPM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_McgIrc48MClk) / 4)

void init();
void writeDigit(unsigned char, unsigned char);
void BOARD_TPM_HANDLER(void);
void writeNumber(float);

