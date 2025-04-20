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

#define METADATA_ADDRESS 0x08008000
#define GOLDEN_IMAGE_ADDRESS 0x08010000 // Application start address
#define SECOND_IMAGE_ADDRESS 0x08020000

typedef void (*pFunction)(void); // Function pointer type for application entry

void JumpToApp(void) {

	uint8_t magic_number;
	uint8_t image_index;
	uint32_t *app_vector_table;

	magic_number = *(volatile uint8_t*)(METADATA_ADDRESS);

	if(magic_number == 22)
	{
		image_index = *(volatile uint8_t*)(METADATA_ADDRESS+1);
	}
	else
	{
		// jump to the golden image
		image_index = 1;
	}

	if(image_index == 1)
	{
		app_vector_table = (uint32_t*)GOLDEN_IMAGE_ADDRESS;
	}
	else if(image_index == 2)
	{
		app_vector_table = (uint32_t*)SECOND_IMAGE_ADDRESS;
	}
	else
	{
		app_vector_table = (uint32_t*)GOLDEN_IMAGE_ADDRESS;
	}


    uint32_t app_sp = app_vector_table[0]; // Application's initial stack pointer
    uint32_t app_start = app_vector_table[1]; // Application's reset handler

    // 2. Disable interrupts and peripherals
    __disable_irq();
    SysTick->CTRL = 0; // Disable SysTick timer

    // 3. Set vector table offset register
    SCB->VTOR = app_vector_table;

    // 4. Update stack pointer
    __set_MSP(app_sp); // Set main stack pointer to application's SP

    // 5. Jump to application
    pFunction app_entry = (pFunction)app_start;
    app_entry(); // Call application's entry point
}

int main(void) {

    JumpToApp();

    while(1);
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
