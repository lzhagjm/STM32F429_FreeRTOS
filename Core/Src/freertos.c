/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs.h"
#include "sdio.h"
#include <string.h>
#include "dma2d.h"
#include <stdio.h>
#include "_1.h"
#include "adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IMGRAM_BASE_ADDR 0xD0000000


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/************Fatfs Task****************/
osThreadId_t FatfsTaskHandle;
const osThreadAttr_t FatfsTask_attributes = {
  .name = "fatfsTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/************LCD Task****************/
osThreadId_t LcdTaskHandle;
const osThreadAttr_t LcdTask_attributes = {
  .name = "lcdTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void FatfsTask(void *argument);
void LcdTask(void *argument);
//static void draw_xy_wh_c(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t c);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  uint32_t val = 0;
	FatfsTaskHandle = osThreadNew(FatfsTask, NULL, &FatfsTask_attributes);
	LcdTaskHandle = osThreadNew(LcdTask, NULL, &LcdTask_attributes);

    for(;;)
    {
    	HAL_ADC_Start(&hadc1);
    	HAL_ADC_PollForConversion(&hadc1, 0xFFFF);
    	val = HAL_ADC_GetValue(&hadc1);
    	HAL_ADC_Stop(&hadc1);
    	printf("core tempsensor:%.2f\r\n",((float)val * 3300/4096 - 760) / 2.5 + 25); // @suppress("Float formatting support")
    	HAL_GPIO_TogglePin(GPIOH, GPIO_PIN_10);
      osDelay(1000);
    }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

static void draw_xy_wh_c(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t c){
	dma2d_put_src_wh_c(IMGRAM_BASE_ADDR + y * 1024 * 3 + x * 3, w, h, c);
}

static void draw_xy_wh_img(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* src){
//	dma2d_put_mem_mem(src, IMGRAM_BASE_ADDR + y * 1024 * 3 + x * 3, w, h);
	uint8_t *p = (uint8_t*)(IMGRAM_BASE_ADDR + y * 1024 * 3 + x * 3);
	uint32_t index = 0;
	int i = 0, j = 0;
	for(i = 0 ; i < h; i++){
		for(j = 0 ; j < w; j++){
				*p = src[index++];
				p++;
				*p = src[index++];
				p++;
				*p = src[index++];
				p++;
		}

		p=p+(1024-w)*3;
	}
}

void FatfsTask(void *argument){
	int ret = 0;
	char path[50];
	const char *write_buff = "hello world! xsdhjkshfksehifuhseuhfuisdhfuise";
	char readbuff[100];
	HAL_SD_CardInfoTypeDef pCardInfo;
	FIL SDFile1, SDFile2;
	UINT bw = 0, br = 0;
	/*1、挂载文件系统*/
	BYTE work[_MAX_SS];
	ret = f_mount (&SDFatFS, (const TCHAR*)SDPath, 1);
	if(ret == FR_NO_FILESYSTEM){
		ret = f_mkfs ((const TCHAR*)SDPath, FM_FAT32, 0, work, sizeof(work));
		if(ret != FR_OK){
				printf("f_mkfs is err:%d\r\n",ret);
				goto loop;
		}
		f_mount (&SDFatFS, SDPath, 1);
	}else if (ret != FR_OK){
			printf("f_mount is err:%d\r\n",ret);
			goto loop;
	}else{
		printf("f_mount is ok\r\n");
	}
	ret = HAL_SD_GetCardInfo(&hsd, &pCardInfo);
	if(ret != HAL_OK){
		printf("HAL_SD_GetCardInfo is err\r\n");
	}else{
		printf("Initialize SD card successfully!\r\n");
		 // 打印SD卡基本信�????????????
		 printf(" SD card information! \r\n");
		 printf(" CardBlockSize : %ld \r\n", pCardInfo.BlockSize);   // 块大�????????????
		 printf(" CardBlockNbr : %ld \r\n", pCardInfo.BlockNbr);   // 块大�????????????
		 printf(" CardCapacity  : %.2f MB \r\n",(double)((unsigned long long)pCardInfo.BlockSize * pCardInfo.BlockNbr/1024)/1024);// 显示容量 // @suppress("Float formatting support")
		 printf(" LogBlockNbr   : %ld \r\n", pCardInfo.LogBlockNbr); // 逻辑块数�????????????
		 printf(" LogBlockSize  : %ld \r\n", pCardInfo.LogBlockSize);// 逻辑块大�????????????
		 printf(" RCA           : %ld \r\n", pCardInfo.RelCardAdd);  // 卡相对地�????????????
		 printf(" CardType      : %ld \r\n", pCardInfo.CardType);    // 卡类�????????????
		 // 读取并打印SD卡的CID信息
		 HAL_SD_CardCIDTypeDef sdcard_cid;
		 HAL_SD_GetCardCID(&hsd,&sdcard_cid);
		 printf(" ManufacturerID: %d \r\n",sdcard_cid.ManufacturerID);
	}
		/*2、新建test.txt文件*/
		sprintf(path, "%stest.txt",SDPath);
		ret = f_open (&SDFile1, path, FA_READ | FA_WRITE);
		if(ret != FR_OK){
			printf("f_open is err:%d\r\n",ret);
			goto f_open_err;
		}
		/*将缓存写入文�?????????????????*/

		ret = f_write (&SDFile1, write_buff, strlen(write_buff), &bw);
		if(ret != FR_OK){
				printf("f_write is err:%d\r\n",ret);
				goto f_write_err;
		}
		printf("write is ok: bw:%d\r\n",bw);
		/*读取文件 0:/xixi.txt*/
		sprintf(path, "%sxixi.txt",SDPath);
		ret = f_open (&SDFile2, path, FA_READ);
		if(ret != FR_OK){
			printf("f_open is err:%d\r\n",ret);
		}
		ret = f_read (&SDFile2, readbuff, 16, &br);
		if(ret != FR_OK){
				printf("f_read is err:%d\r\n",ret);
		}
		readbuff[16] = 0;
		printf("%s\r\n",readbuff);
		f_close (&SDFile1);
		f_close (&SDFile2);
		f_mount(NULL, SDPath, 1);
		goto loop;
		/* Infinite loop */
	f_write_err:
		f_close (&SDFile1);
	f_open_err:
		f_mount(NULL, SDPath, 1);
		loop:
	while(1){
		osDelay(1000);
	};
}


void LcdTask(void *argument){

	while(1){
		draw_xy_wh_c(0, 0, 1024, 600, 0);
		draw_xy_wh_c(255, 149, 512, 300, 0xFF0000);
		osDelay(1000);
		draw_xy_wh_c(255, 149, 512, 300, 0x00FF00);
		osDelay(1000);
		draw_xy_wh_c(255, 149, 512, 300, 0x0000FF);
		osDelay(1000);
		draw_xy_wh_c(255, 149, 512, 300, 0xFFFFFF);
		osDelay(1000);
		draw_xy_wh_c(0, 0, 1024, 600, 0);
		draw_xy_wh_img(411, 199, 200, 200, _1_IMAGE);
		osDelay(1000);
	}
}



/* USER CODE END Application */

