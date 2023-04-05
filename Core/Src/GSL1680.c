#include "gsl1680_fw.h"
#include "GSL1680.h"
#include <stdio.h>


osSemaphoreId_t Touchsemaphore;

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static HAL_StatusTypeDef GSL_Transmit_Byte(uint8_t reg, uint8_t Data, uint32_t Timeout){
	uint8_t val = Data;
	return GSL_Transmit(reg, &val, 1, Timeout);
}


static uint8_t GSL_Receive_Byte(uint8_t reg, uint32_t Timeout){
	uint8_t val;
	uint8_t err;
	err = GSL_Receive(reg, &val, 1, Timeout);
	if(err != HAL_OK){
		printf("GSL_Receive is err: %d\r\n", err);
	}
	return val;
}

void GSL_INT_Callback(void){
	osSemaphoreRelease(Touchsemaphore);
}



static __inline__ void fw2buf(uint8_t *buf, const uint32_t *fw)
{
	uint32_t *u32_buf = (int *)buf;
	*u32_buf = *fw;
}

static void gsl_load_fw(void)
{
	uint8_t buf[DMA_TRANS_LEN*4 + 1] = {0};
	uint8_t send_flag = 1;
	uint8_t *cur = buf + 1;
	uint32_t source_line = 0;
	uint32_t source_len;
	HAL_StatusTypeDef err;
	const struct fw_data *ptr_fw;

	printf("=============gsl_load_fw start==============\r\n");

	ptr_fw = GSLX680_FW;
	source_len = ARRAY_SIZE(GSLX680_FW);

	for (source_line = 0; source_line < source_len; source_line++)
	{
		/* init page trans, set the page val */
		if (GSL_PAGE_REG == ptr_fw[source_line].offset)
		{
			fw2buf(cur, &ptr_fw[source_line].val);
			err = GSL_Transmit(GSL_PAGE_REG, buf + 1, 4, 0xFFFFFF);
			if(err != HAL_OK){
				printf("GSL_Transmit error, source_line: %d\r\n", source_line);
				goto error;
			}
			send_flag = 1;
		}
		else
		{
			if (1 == send_flag % (DMA_TRANS_LEN < 0x20 ? DMA_TRANS_LEN : 0x20))
	    			buf[0] = (uint8_t)ptr_fw[source_line].offset;

			fw2buf(cur, &ptr_fw[source_line].val);
			cur += 4;

			if (0 == send_flag % (DMA_TRANS_LEN < 0x20 ? DMA_TRANS_LEN : 0x20))
			{
				err = GSL_Transmit(buf[0], buf + 1, cur - buf - 1, 0xFFFFFF);
				if(err != HAL_OK){
					printf("GSL_Transmit error, source_line: %d\r\n", source_line);
					goto error;
				}
	    	cur = buf + 1;
			}
			send_flag++;
		}
	}

	printf("=============gsl_load_fw end==============\r\n");
error:
	return;
}


static void test_i2c(void)
{
	uint8_t read_buf = 0;
	uint8_t write_buf = 0x12;
	int ret;
	ret = GSL_Receive(0xf0, &read_buf, sizeof(read_buf), 0xFFFFFF);
	if  (ret != HAL_OK)
	{
		printf("I2C transfer error!\r\n");
	}
	else
	{
		printf("I read reg 0xf0 is %x\r\n", read_buf);
	}
	HAL_Delay(10);

	ret = GSL_Transmit(0xf0, &write_buf, sizeof(write_buf), 0xFFFFFF);
	if  (ret != HAL_OK)
	{
		printf("I2C transfer error!\r\n");
	}
	else
	{
		printf("I write reg 0xf0 0x12\r\n");
	}
	HAL_Delay(10);

	ret = GSL_Receive(0xf0, &read_buf, sizeof(read_buf), 0xFFFFFF);
	if  (ret != HAL_OK)
	{
		printf("I2C transfer error!\r\n");
	}
	else
	{
		printf("I read reg 0xf0 is 0x%x\r\n", read_buf);
	}
	HAL_Delay(10);

}

static void startup_chip(void)
{
	uint8_t tmp = 0x00;
//	uint8_t buf[4] = {0x00};
//	buf[3] = 0x01;
//	buf[2] = 0xfe;
//	buf[1] = 0x10;
//	buf[0] = 0x00;
//	GSL_Transmit(0xf0, buf, 4, 0xFFFFFF);
//	buf[3] = 0x00;
//	buf[2] = 0x00;
//	buf[1] = 0x00;
//	buf[0] = 0x0f;
//	GSL_Transmit(0x04, buf, 4, 0xFFFFFF);
//	HAL_Delay(20);
	GSL_Transmit(0xe0, &tmp, 1, 0xFFFFFF);
	HAL_Delay(10);
}

static void reset_chip(void)
{
	uint8_t tmp = 0x88;
	uint8_t buf[4] = {0x00};
	GSL_Transmit(0xe0, &tmp, sizeof(tmp), 0xFFFFFF);
	HAL_Delay(20);
	tmp = 0x04;
	GSL_Transmit(0xe4, &tmp, sizeof(tmp), 0xFFFFFF);
	HAL_Delay(10);
	GSL_Transmit(0xbc, buf, 4, 0xFFFFFF);
	HAL_Delay(10);
}

static void clr_reg(void)
{
	uint8_t write_buf[4]	= {0};

	write_buf[0] = 0x88;
	GSL_Transmit(0xe0, &write_buf[0], 1, 0xFFFFFF);
	HAL_Delay(20);
	write_buf[0] = 0x03;
	GSL_Transmit(0x80, &write_buf[0], 1, 0xFFFFFF);
	HAL_Delay(5);
	write_buf[0] = 0x04;
	GSL_Transmit(0xe4, &write_buf[0], 1, 0xFFFFFF);
	HAL_Delay(5);
	write_buf[0] = 0x00;
	GSL_Transmit(0xe0, &write_buf[0], 1, 0xFFFFFF);
	HAL_Delay(20);
}

static void init_chip(void)
{
	GSL_RST(0);
	HAL_Delay(20);
	GSL_RST(1);
	HAL_Delay(20);
	test_i2c();
	clr_reg();
	reset_chip();
	gsl_load_fw();
	startup_chip();
	reset_chip();
	startup_chip();
}

static void check_mem_data(void)
{
	uint8_t read_buf[4]  = {0};

	HAL_Delay(1000);
	GSL_Receive(0xb0, read_buf, 4, 0xFFFFFF);
	if (read_buf[3] != 0x5a || read_buf[2] != 0x5a || read_buf[1] != 0x5a || read_buf[0] != 0x5a)
	{
	 printf("#########check mem read 0xb0 = %x %x %x %x #########\r\n", read_buf[3], read_buf[2], read_buf[1], read_buf[0]);
	 init_chip();
	}
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
	err = GSL_Receive(GSL_REG_ID, id, 4, 0xFFFF);
	if(err != HAL_OK){
		printf("GSL_Receive ID is err: %d\r\n", err);
	}else{
		printf("GSL_Receive ID is : %2X%2X%2X%2X\r\n", id[3], id[2], id[1], id[0]);
	}
	Touchsemaphore = osSemaphoreNew (0xFFFFFFFF, 0, NULL);
	init_chip();
	check_mem_data();
	return err;
}



