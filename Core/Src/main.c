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

IWDG_HandleTypeDef hiwdg;

/* USER CODE BEGIN PV */

typedef enum {
	BOOT_NEXT_FROM_HERE = 0,
	TRYING_TO_BOOT = 1,
	BOOTED_SUCCESFULLY = 2
} Boot_Feedback;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define METADATA_ADDRESS 0x08008000
#define GOLDEN_IMAGE_ADDRESS 0x08010000 // Application start address
#define SECOND_IMAGE_ADDRESS 0x08020000
#define THIRD_IMAGE_ADDRESS 0x08040000

typedef void (*pFunction)(void); // Function pointer type for application entry

//void configure_metadata(uint8_t magic_number, uint8_t image_index, uint8_t boot_feedback)
//{
//  if(magic_number != 22 || boot_feedback != BOOTED_SUCCESFULLY)
//  {
//    HAL_FLASH_Unlock();
//
//    // the 3rd byte of the metadata represents the following things:
//    // -> 0 = the system has to boot from this new image for the first time (set when a JUMP_TO_IMAGE command was received
//    // -> 1 = the bootloader jumped to this new image, and waits to see if it can successfully boot
//    // -> 2 = the system booted successfully
//
//    FLASH_EraseInitTypeDef eraseInitStruct;
//    uint32_t pageError = 0;
//
//    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;     // or FLASH_TYPEERASE_PAGES (depends on family)
//    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;    // depends on your supply voltage
//    eraseInitStruct.Sector = FLASH_SECTOR_1;                 // sector you want to erase
//    eraseInitStruct.NbSectors = 1;
//
//    HAL_FLASHEx_Erase(&eraseInitStruct, &pageError);
//
//    HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, METADATA_ADDRESS, 22);
//    HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, METADATA_ADDRESS+1, image_index);
//    HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, METADATA_ADDRESS+2, TRYING_TO_BOOT);
//
//    HAL_FLASH_Lock();
//  }
//}

void JumpToApp(void)
{
    uint8_t magic_number;
    uint8_t image_index;
    uint8_t boot_feedback;
    uint32_t *app_vector_table;
    uint32_t app_sp, app_start;

    // --- Read metadata ---
    magic_number   = *(volatile uint8_t*)(METADATA_ADDRESS);
    image_index    = *(volatile uint8_t*)(METADATA_ADDRESS + 1);
    boot_feedback  = *(volatile uint8_t*)(METADATA_ADDRESS + 2);

    // --- Handle IWDG reset: force booting golden image on watchdog reset ---
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        __HAL_RCC_CLEAR_RESET_FLAGS();
        image_index = 1;
    }

    // --- Check metadata integrity ---
    if (magic_number != 22)
    {
        image_index = 1; // Fallback to golden image
    }

    // --- Select application vector table based on image index ---
    switch (image_index)
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

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
//  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
//  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  __HAL_DBGMCU_FREEZE_IWDG();

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

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
