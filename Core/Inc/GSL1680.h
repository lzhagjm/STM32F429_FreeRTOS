/*
 * GSL1680.h
 *
 *  Created on: 2023年4月4日
 *      Author: LZH-PC
 */

#ifndef INC_GSL1680_H_
#define INC_GSL1680_H_

#include "i2c.h"
#include "cmsis_os2.h"
extern osSemaphoreId_t Touchsemaphore;

/* 					port 					*/
#define GSL_I2C_DEVICE (&hi2c2)
#define GSL_RST(x)	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, x);
#define GSL_ADDR		(0x40 << 1)




/* 			  GSL REGs 				*/
#define GSL_REG_RESET	0xE0
#define GSL_REG_DATA		0x80
#define GSL_REG_TOUCH_NR	0x80
#define GSL_REG_POWER	0xBC
#define GSL_REG_CLOCK	0xE4
#define GSL_REG_STATUS	0xB0
#define GSL_REG_ID		0xFC
#define GSL_REG_MEM_CHECK	0xB0
#define GSL_STATUS_REG		0xe0
#define GSL_PAGE_REG		0xf0


#define GSL_STATUS_OK	0x5A5A5A5A
#define GSL_TS_DATA_LEN	44
#define GSL_CLOCK		0x04

#define GSL_CMD_RESET	0x88
#define GSL_CMD_START	0x00

#define GSL_POINT_DATA_LEN	0x04
#define GSL_POINT_Y_OFF      0x00
#define GSL_POINT_Y_MSB_OFF	0x01
#define GSL_POINT_X_OFF	0x02
#define GSL_POINT_X_MSB_OFF	0x03
#define GSL_EXTRA_DATA_MASK	0xF0

#define GSL_CMD_SLEEP_MIN	10000
#define GSL_CMD_SLEEP_MAX	20000
#define GSL_POWER_SLEEP	20
#define GSL_STARTUP_SLEEP	30

#define GSL_MAX_FINGERS	10
#define DMA_TRANS_LEN	0x20

struct gsl_touch_info
{
	int x[10];
	int y[10];
	int id[10];
	int finger_num;
};


/* 				GSL FUNs 					*/

#define GSL_Transmit(reg, pData, Size, Timeout) HAL_I2C_Mem_Write(GSL_I2C_DEVICE, GSL_ADDR, reg, I2C_MEMADD_SIZE_8BIT, pData, Size, Timeout)

#define GSL_Receive(reg, pData, Size, Timeout) HAL_I2C_Mem_Read(GSL_I2C_DEVICE, GSL_ADDR, reg, I2C_MEMADD_SIZE_8BIT, pData, Size, Timeout)


uint8_t GSL_Init(void);
void GSL_INT_Callback(void);

#endif /* INC_GSL1680_H_ */
