/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FRAM.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c4;

IWDG_HandleTypeDef hiwdg;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);
static void MX_I2C4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define METADATA_ADDRESS FRAM_SWEEP_TABLE_SECTION_START
#define METADATA_SIZE 7
#define GOLDEN_IMAGE_ADDRESS 0x08010000 // Application start address
#define SECOND_IMAGE_ADDRESS 0x08020000
#define THIRD_IMAGE_ADDRESS 0x08040000

typedef void (*pFunction)(void); // Function pointer type for application entry


void JumpToApp(void)
{
	uint32_t *app_vector_table;
	uint32_t app_sp, app_start;

	uint16_t Received_CRC;
	uint16_t Computed_CRC;
	uint8_t metadata_raw[7] = {0};
    Metadata_Struct metadata;

    readFRAM(METADATA_ADDRESS, (uint8_t *)&metadata_raw, METADATA_SIZE);

    Received_CRC = ((uint16_t)metadata_raw[0] << 8) | metadata_raw[1];
    Computed_CRC = Calc_CRC16(metadata_raw + 2, METADATA_SIZE - 2);

    if(Computed_CRC != Received_CRC)
    {
    	metadata.crc = 0xFFFF;
    	metadata.new_metadata = NO;
    	metadata.boot_feedback = BOOT_NEW_IMAGE;
    	metadata.image_index = 1;
    	metadata.boot_counter = 3;
    	metadata.error_code = CORRUPTED_METADATA_ERROR;

    	goto Compute_CRC;
    }

    unpack_metadata(&metadata, metadata_raw);

    if(metadata.new_metadata == YES)
    {
    	metadata.new_metadata = NO;
    	metadata.error_code = NO_BOOT_ERROR;
    	goto Compute_CRC;
    }
    else if(metadata.new_metadata == NO)
    {
    	if(metadata.boot_feedback == BOOTED_OK)
    	{
    	    // --- Handle IWDG reset: force booting golden image on watchdog reset ---
    	    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    	    {
    	        __HAL_RCC_CLEAR_RESET_FLAGS();
    	        metadata.new_metadata = YES;
    	        metadata.boot_feedback = BOOT_NEW_IMAGE;
    	        metadata.image_index = 1;
    	    	metadata.boot_counter = 3;
    	        metadata.error_code = HARDWARE_RESET_ERROR;
    	        goto Compute_CRC;
    	    }
    	    else
    	    {
    	    	metadata.error_code = NO_BOOT_ERROR;
    	    	goto Compute_CRC;
    	    }
    	}
    	else if(metadata.boot_feedback == BOOT_NEW_IMAGE)
    	{
    		if(metadata.boot_counter > 0 && metadata.boot_counter < 4)
    		{
    			metadata.boot_counter --;
    			metadata.error_code = BOOT_FAILURE_CURRENT_IMAGE_ERROR;
    			goto Compute_CRC;
    		}
    		else
    		{
    	        metadata.new_metadata = YES;
    	        metadata.image_index = 1;
    	    	metadata.boot_counter = 3;
    	        metadata.error_code = BOOT_FAILURE_PREVIOUS_IMAGE_ERROR;
    	        goto Compute_CRC;
    		}
    	}
    	else
    	{
    		goto Wrong_Metadata;
    	}
    }
    else
    {
    	goto Wrong_Metadata;
    }

Wrong_Metadata:

	metadata.new_metadata = YES;
	metadata.boot_feedback = BOOT_NEW_IMAGE;
	metadata.image_index = 1;
	metadata.boot_counter = 3;
	metadata.error_code = HARDWARE_RESET_ERROR;
	goto Compute_CRC;

Compute_CRC:
	pack_metadata(&metadata, metadata_raw);
	Computed_CRC = Calc_CRC16(metadata_raw + 2, METADATA_SIZE - 2);
    metadata_raw[0] = (Computed_CRC >> 8) & 0xFF;  // MSB
    metadata_raw[1] = Computed_CRC & 0xFF;         // LSB

    writeFRAM(METADATA_ADDRESS, (uint8_t *)&metadata_raw, METADATA_SIZE);

    // --- Select application vector table based on image index ---
    switch (metadata.image_index)
    {
        case 1: app_vector_table = (uint32_t*)GOLDEN_IMAGE_ADDRESS; break;
        case 2: app_vector_table = (uint32_t*)SECOND_IMAGE_ADDRESS; break;
        case 3: app_vector_table = (uint32_t*)THIRD_IMAGE_ADDRESS; break;
        default: app_vector_table = (uint32_t*)GOLDEN_IMAGE_ADDRESS; break;
    }

    app_sp    = app_vector_table[0]; // Initial Stack Pointer
    app_start = app_vector_table[1]; // Reset Handler (entry point)

    // --- Disable interrupts and system tick ---
    __disable_irq();
    SysTick->CTRL = 0;

    // --- Set vector table offset to application ---
    SCB->VTOR = (uint32_t)app_vector_table;

    // --- Set main stack pointer ---
    __set_MSP(app_sp);

    // --- Jump to application ---
    typedef void (*pFunction)(void);
    pFunction app_entry = (pFunction)app_start;
    app_entry();

    // Should never return
    while (1);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

	// MAKE SURE THAT THE MPU_Config() part is commented out
	// IF YOU REGENERATE THE CODE, it will be included

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
//  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_IWDG_Init();
  MX_I2C4_Init();
  /* USER CODE BEGIN 2 */
  __HAL_DBGMCU_FREEZE_IWDG();

  HAL_Delay(2000);
  JumpToApp();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x00303D5B;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
