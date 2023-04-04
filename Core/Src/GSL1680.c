#include "gsl1680_fw.h"
#include "GSL1680.h"
#include <stdio.h>
#include "cmsis_os2.h"

osSemaphoreId_t Touchsemaphore;

static HAL_StatusTypeDef GSL_Transmit_Byte(uint16_t DevAddress, uint8_t Data, uint32_t Timeout){
	uint8_t val = Data;
	return GSL_Transmit(DevAddress, &val, 1, Timeout);
}


static uint8_t GSL_Receive_Byte(uint16_t DevAddress, uint32_t Timeout){
	uint8_t val;
	uint8_t err;
	err = GSL_Receive(DevAddress, &val, 1, Timeout);
	if(err != HAL_OK){
		printf("GSL_Receive is err: %d\r\n", err);
	}
	return val;
}

void GSL_INT_Callback(void){
	printf("this is int\r\n");
}


uint8_t GSL_Init(void){
	int err = 0;
	uint8_t id[4];
	/* reset */
	GSL_RST(0);
	HAL_Delay(20);
	GSL_RST(1);
	HAL_Delay(300);
	/* resd GSL id */
	err = GSL_Receive(GSL_ADDR, id, 4, 0xFFFF);
	if(err != HAL_OK){
		printf("GSL_Receive ID is err: %d\r\n", err);
	}else{
		printf("GSL_Receive ID is : %2X%2X%2X%2X\r\n", id[3], id[2], id[1], id[0]);
	}
	Touchsemaphore = osSemaphoreNew (1, 0, NULL);
	return err;
}



