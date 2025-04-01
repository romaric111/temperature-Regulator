
/*
 * affichage.c
 *
 *  Nov. 25, 2020
 *  Original file : Bosch
 *  Modified by : kamel adi
 */

#include <stdio.h>
#include <stdint.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL27Z644.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_i2c.h"
#include "affichage.h"

#include "delay/delay.h"
#include "LiquidCrystal_I2C.h"

#include "bmp280.h"
//lcd
#define I2C_MASTER_BASEADDR I2C0
#define	I2C_BUFFER_LEN 25
//global variables for bmp
 int8_t rslt;
	    struct bmp280_dev bmp;
	    struct bmp280_config conf;
	    struct bmp280_uncomp_data ucomp_data;
	    int32_t temp32;
	    double temp;

struct bmp280_dev bmp;

volatile uint32_t g_systickCounter;







#define STANDARD_TEMP (25U)

void delay_ms1(uint32_t period_ms);
double lecteurTemperature(void);
void iniBmp(void);
void affichageEcran(double temperature,char message[]);
void affichageEcranFixe(void);
int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
void print_rslt(const char api_name[], int8_t rslt);
//motor
void initGPIO();
void turnOnMotor();
void turnOffMotor();

int main(void)
{
	    /* Init board hardware. */
	    BOARD_InitBootPins();
 	    BOARD_InitBootClocks();
	    BOARD_InitBootPeripherals();
	    /* Init FSL debug console. */
	    BOARD_InitDebugConsole();
	    BOARD_InitPeripherals();
	   /* Set systick reload value to generate 1ms interrupt */
	    if (SysTick_Config(SystemCoreClock / 1000U))
	    {
	     PRINTF("Echec dans l'initialisation du systick\n");
	   }
        iniBmp();// initialise le capteur
	    affichageEcranFixe(); //initialise l'ecran
	    char message[13];
	    int tour=0; // indique l'etat du ventilatteur
	     double temperature=0; // stocke la temperature actuelle
	 while(1){

		 // Vérification si 3 secondes sont passées
		         if (millis() - last_time >= 3000) {
		             // Mise à jour du temps
		             last_time = millis(); // option pour permettre de donner un timer


	     temperature=lecteurTemperature(); // Lecture de la temperature

	    if(temperature<25) {
	    	 if(tour==1){
	    		     turnOffMotor();
	    		     tour=0;
	    	 }
	      sprintf(message,"Bon_temps", 0);
	      affichageEcran(temperature,message);
	      GPIOA->PCOR|=(1u<<13);//allumage led blue

	      	  }
	    else{
	    	sprintf(message,"Mauvais_temps", 0);
		  affichageEcran(temperature,message);
		  GPIOA->PSOR|=(1u<<13);//etteindre led verte

		  if(tour==0){
	    	 GPIOB->PCOR|=(1u<<18);//allumage  rouge
	    	 delay_ms1(100000U);
	    	 turnOnMotor();
	    	 tour=1;
		  }
	    }
 }

	    	  //delay_ms1(3000U); // suppression du au remarque positive du professeur
	                              //lors de la presentation

	    return 0;
}

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs such as "bmg250_soft_reset",
 *      "bmg250_set_foc", "bmg250_perform_self_test"  and so on.
 *
 *  @param[in] period_ms  : the required wait time in milliseconds.
 *  @return void.
 *
 */
void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
}


void delay_ms1(uint32_t period_ms)
{
	g_systickCounter = period_ms;
	    while (g_systickCounter != 0U)
	    {
	    }
}

/*!
 *  @brief Function for writing the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[in] reg_data : Pointer to the data buffer whose value is to be written.
 *  @param[in] length   : No of bytes to write.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
	status_t status;

		uint32_t iError = 0;
		uint8_t array[I2C_BUFFER_LEN]= {0};
		uint8_t stringpos = 0;
		array[0] = reg_addr;

		i2c_master_transfer_t masterXfer;
	    memset(&masterXfer, 0, sizeof(masterXfer));

		masterXfer.slaveAddress   = i2c_addr;
		masterXfer.direction      = kI2C_Write;
		masterXfer.subaddress     = reg_addr;
		masterXfer.subaddressSize = 1;
		masterXfer.data           = &array[0];
		masterXfer.dataSize       = length;
		masterXfer.flags          = kI2C_TransferDefaultFlag;

		for (stringpos = 0; stringpos < length; stringpos++) {
				array[stringpos] = *(reg_data + stringpos);
			}

		status = I2C_MasterTransferBlocking(I2C_MASTER_BASEADDR, &masterXfer);

		if (status == kStatus_Success)
		{
			iError = 0;
		}
		else
		{
				iError = 2;  // Doit être revu ....
		}

		return (uint8_t)iError;

}

/*!
 *  @brief Function for reading the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : Sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[out] reg_data    : Pointer to the data buffer to store the read data.
 *  @param[in] length   : No of bytes to read.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{

		status_t status;
		uint32_t iError = 0;
		uint8_t array[I2C_BUFFER_LEN] = {0};
		uint8_t stringpos = 0;
		array[0] = reg_addr;
		i2c_master_transfer_t masterXfer;

		memset(&masterXfer, 0, sizeof(masterXfer));
		masterXfer.slaveAddress   = i2c_addr;
		masterXfer.direction      = kI2C_Read;
		masterXfer.subaddress     = reg_addr;
		masterXfer.subaddressSize = 1;
		masterXfer.data           = array;
		masterXfer.dataSize       = length;
		masterXfer.flags          = kI2C_TransferDefaultFlag;

		status = I2C_MasterTransferBlocking(I2C_MASTER_BASEADDR, &masterXfer);

		if (status == kStatus_Success)
			{
				iError = 0;
			}
			else
			{
					iError = 2;  // Doit être revu ....
			}

		for (stringpos = 0; stringpos < length; stringpos++) {
			*(reg_data + stringpos) = array[stringpos];
		}

		return (uint8_t)iError;
}


/*!
 *  @brief Prints the execution status of the APIs.
 *
 *  @param[in] api_name : name of the API whose execution status has to be printed.
 *  @param[in] rslt     : error code returned by the API whose execution status has to be printed.
 *
 *  @return void.
 */
void print_rslt(const char api_name[], int8_t rslt)
{
    if (rslt != BMP280_OK)
    {
        printf("%s\t", api_name);
        if (rslt == BMP280_E_NULL_PTR)
        {
            printf("Error [%d] : Null pointer error\r\n", rslt);
        }
        else if (rslt == BMP280_E_COMM_FAIL)
        {
            printf("Error [%d] : Bus communication failed\r\n", rslt);
        }
        else if (rslt == BMP280_E_IMPLAUS_TEMP)
        {
            printf("Error [%d] : Invalid Temperature\r\n", rslt);
        }
        else if (rslt == BMP280_E_DEV_NOT_FOUND)
        {
            printf("Error [%d] : Device not found\r\n", rslt);
        }
        else
        {
            /* For more error codes refer "*_defs.h" */
            printf("Error [%d] : Unknown error code\r\n", rslt);
        }
    }
}
void iniBmp(void){

// connect bmp with i2c
	    bmp.delay_ms = delay_ms;

	   	    /* Assign device I2C address based on the status of SDO pin (GND for PRIMARY(0x76) & VDD for SECONDARY(0x77)) */
	   	    bmp.dev_id =BMP280_I2C_ADDR_PRIM; //BMP280_I2C_ADDR_PRIM;BMP280_I2C_ADDR_SEC;

	   	    /* Select the interface mode as I2C */
	   	    bmp.intf = BMP280_I2C_INTF;

	   	    /* Map the I2C read & write function pointer with the functions responsible for I2C bus transfer */
	   	    bmp.read = i2c_reg_read;
	   	    bmp.write = i2c_reg_write;

	    rslt = bmp280_init(&bmp);
	       print_rslt(" bmp280_init status", rslt);

	       /* Always read the current settings before writing, especially when
	        * all the configuration is not modified
	        */
	       rslt = bmp280_get_config(&conf, &bmp);
	       print_rslt(" bmp280_get_config status", rslt);

	       /* configuring the temperature oversampling, filter coefficient and output data rate */
	       /* Overwrite the desired settings */
	       conf.filter = BMP280_FILTER_COEFF_2;

	       /* Temperature oversampling set at 4x */
	       conf.os_temp = BMP280_OS_4X;
	       /* Setting the output data rate as 1HZ(1000ms) */
	         conf.odr = BMP280_ODR_1000_MS;
	         rslt = bmp280_set_config(&conf, &bmp);
	         print_rslt(" bmp280_set_config status", rslt);

	         /* Always set the power mode after setting the configuration */
	         rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);
	         print_rslt(" bmp280_set_power_mode status", rslt);

	         init();
}
double lecteurTemperature(void){
	         /* Reading the raw data from sensor */
	               rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);
	               /* Getting the 32 bit compensated temperature */
	               rslt = bmp280_get_comp_temp_32bit(&temp32, ucomp_data.uncomp_temp, &bmp);

	               /* Getting the compensated temperature as floating point value */
	               rslt = bmp280_get_comp_temp_double(&temp, ucomp_data.uncomp_temp, &bmp);
	               printf("UT: %ld, T32: %ld, T: %f \r\n", ucomp_data.uncomp_temp, temp32, temp);
	               //writeNumber((float)temp);
	               /* Sleep time between measurements = BMP280_ODR_1000_MS */
	               bmp.delay_ms(1000);

	return temp;
}


//affichage lcd
void affichageEcranFixe(void) {
    /* Configure le timer système pour des interruptions périodiques (SysTick) */
    if (SysTick_Config(SystemCoreClock / 100000U)) {
        PRINTF("Echec dans l'initialisation du systick\n");
    }

    // Initialisation de l'écran LCD
    begin();  // Démarre l'écran LCD (en mode 4 bits par exemple)

    // Active le rétroéclairage de l'écran
    backlight();

    // Affiche le curseur sur l'écran
    cursor();

    // Efface l'écran pour démarrer avec un affichage propre
    clear();

    // Affiche un message statique sur l'écran
    print("tempCapteur=");  // Affiche "tempCapteur=" sur la première ligne
    setCursor(0U, 1U);  // Déplace le curseur sur la deuxième ligne
    print("welcome");   // Affiche "welcome" sur la deuxième ligne
}

void affichageEcran(double tempcapteur,char message[]){
	 char myStringCap[20];  // Adjust the size based on your needs
	// char myStringCart[20];
		        	    // Use sprintf to convert double to string
		        	    sprintf(myStringCap, "%.2f", tempcapteur);
		        	    printf("\nString: %s\n", myStringCap);
		        	    setCursor(12U,0U);//set cursor line 0
		        	    print(myStringCap);
		        	    setCursor(0U,1U);//set cursor line1
		        	    print(message);
}


void initGPIO() {

}






void turnOnMotor() {
    //Active la LED verte (indiquant que le moteur est actif)
    GPIOB->PSOR |= (1u << 18);

    //Active le moteur en mettant à HIGH la broche correspondante
    GPIOA->PSOR |= (1u << 12);

    // Éteint la LED rouge (si elle était allumée précédemment)
    GPIOB->PCOR |= (1u << 19);
}

void turnOffMotor() {
    //Désactive le moteur en mettant à LOW la broche correspondante
    GPIOA->PCOR |= (1u << 12);

    //Éteint la LED verte (indiquant que le moteur est désactivé)
    GPIOB->PSOR |= (1u << 19);
}
