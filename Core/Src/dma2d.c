/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dma2d.c
  * @brief   This file provides code for the configuration
  *          of the DMA2D instances.
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
#include "dma2d.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

DMA2D_HandleTypeDef hdma2d;

/* DMA2D init function */
void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspInit 0 */

  /* USER CODE END DMA2D_MspInit 0 */
    /* DMA2D clock enable */
    __HAL_RCC_DMA2D_CLK_ENABLE();
  /* USER CODE BEGIN DMA2D_MspInit 1 */

  /* USER CODE END DMA2D_MspInit 1 */
  }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspDeInit 0 */

  /* USER CODE END DMA2D_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DMA2D_CLK_DISABLE();
  /* USER CODE BEGIN DMA2D_MspDeInit 1 */

  /* USER CODE END DMA2D_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void dma2d_put_src_wh_c(uint32_t p, uint32_t w, uint32_t h, uint32_t c){
  hdma2d.Init.Mode = DMA2D_R2M;
	hdma2d.Init.OutputOffset = 1024 - w;
  /* Change DMA2D peripheral state */
  hdma2d.State = HAL_DMA2D_STATE_BUSY;
  MODIFY_REG(hdma2d.Instance->CR, DMA2D_CR_MODE, hdma2d.Init.Mode);
  MODIFY_REG(hdma2d.Instance->OOR, DMA2D_OOR_LO, hdma2d.Init.OutputOffset);
  /* Initialize the DMA2D state*/
  hdma2d.State  = HAL_DMA2D_STATE_READY;
	HAL_DMA2D_Start(&hdma2d, c, p, w, h);
	HAL_DMA2D_PollForTransfer(&hdma2d, 0xFFFF);
}


void dma2d_put_mem_mem(uint32_t src, uint32_t dst, uint32_t w, uint32_t h){
  hdma2d.Init.Mode = DMA2D_M2M;
	hdma2d.Init.OutputOffset = (1024 - w);
  /* Change DMA2D peripheral state */
  hdma2d.State = HAL_DMA2D_STATE_BUSY;
  MODIFY_REG(hdma2d.Instance->CR, DMA2D_CR_MODE, hdma2d.Init.Mode);
  MODIFY_REG(hdma2d.Instance->OOR, DMA2D_OOR_LO, hdma2d.Init.OutputOffset);
  /* Initialize the DMA2D state*/
  hdma2d.State  = HAL_DMA2D_STATE_READY;
	HAL_DMA2D_Start(&hdma2d, src, dst, w, h);
	HAL_DMA2D_PollForTransfer(&hdma2d, 0xFFFF);
}

/* USER CODE END 1 */
