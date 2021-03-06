/**
 ******************************************************************************
 * @file    Templates/Src/main.c
 * @author  MCD Application Team
 * @version V1.0.3
 * @date    22-April-2016
 * @brief   STM32F7xx HAL API Template project
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>

/** @addtogroup STM32F7xx_HAL_Examples
 * @{
 */

/** @addtogroup Templates
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef uart_handle;
GPIO_InitTypeDef fan0;
GPIO_InitTypeDef button0;
GPIO_InitTypeDef button1;
GPIO_InitTypeDef sensor;
TIM_HandleTypeDef TimHandle;
TIM_HandleTypeDef Tim2Handle;
TIM_HandleTypeDef Tim3Handle;
TIM_OC_InitTypeDef sConfig;
TIM_IC_InitTypeDef ICConfig;

/* Private function prototypes -----------------------------------------------*/

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

static void SystemClock_Config(void);
static void Error_Handler(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */

void EXTI9_5_IRQHandler();
void EXTI0_IRQHandler();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void TIM2_IRQHandler();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
uint32_t event = 0;

int main(void) {
	/* This project template calls firstly two functions in order to configure MPU feature
	 and to enable the CPU Cache, respectively MPU_Config() and CPU_CACHE_Enable().
	 These functions are provided as template implementation that User may integrate
	 in his application, to enhance the performance in case of use of AXI interface
	 with several masters. */

	/* Configure the MPU attributes as Write Through */
	MPU_Config();

	/* Enable the CPU Cache */
	CPU_CACHE_Enable();

	/* STM32F7xx HAL library initialization:
	 - Configure the Flash ART accelerator on ITCM interface
	 - Configure the Systick to generate an interrupt each 1 msec
	 - Set NVIC Group Priority to 4
	 - Low Level Initialization
	 */
	HAL_Init();

	/* Configure the System clock to have a frequency of 216 MHz */
	SystemClock_Config();

	BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_EXTI);

	/* Add your application code here
	 */
	BSP_LED_Init(LED_GREEN);

	uart_handle.Init.BaudRate = 115200;
	uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
	uart_handle.Init.StopBits = UART_STOPBITS_1;
	uart_handle.Init.Parity = UART_PARITY_NONE;
	uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart_handle.Init.Mode = UART_MODE_TX_RX;

	BSP_COM_Init(COM1, &uart_handle);

	__HAL_RCC_TIM1_CLK_ENABLE();

	TimHandle.Instance               = TIM1;
	TimHandle.Init.Period            = 5000;
	TimHandle.Init.Prescaler         = 5000;
	TimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	TimHandle.Init.CounterMode 		 = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TimHandle);
	HAL_TIM_Base_Start_IT(&TimHandle);

	HAL_TIM_PWM_Init(&TimHandle);

	sConfig.OCMode = TIM_OCMODE_PWM1;
	sConfig.Pulse = 2500;

	HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start_IT(&TimHandle, TIM_CHANNEL_1);

	__HAL_RCC_TIM2_CLK_ENABLE();

	Tim2Handle.Instance               = TIM2;
	Tim2Handle.Init.Period            = 1646;
	Tim2Handle.Init.Prescaler         = (0xFFFF / 10);
	Tim2Handle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	Tim2Handle.Init.CounterMode 	  = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&Tim2Handle);
	HAL_TIM_Base_Start_IT(&Tim2Handle);

	__HAL_RCC_TIM3_CLK_ENABLE();

	Tim3Handle.Instance               = TIM3;
	Tim3Handle.Init.Period            = 1646;
	Tim3Handle.Init.Prescaler         = (0xFFFF / 10);
	Tim3Handle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	Tim3Handle.Init.CounterMode 	  = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&Tim3Handle);
	HAL_TIM_Base_Start_IT(&Tim3Handle);

	ICConfig.ICPolarity = TIM_ICPOLARITY_RISING;
	ICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
	ICConfig.ICPrescaler = TIM_ICPSC_DIV1;
	ICConfig.ICFilter = 0;

	HAL_TIM_IC_ConfigChannel(&Tim3Handle, &ICConfig, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&Tim3Handle, TIM_CHANNEL_1);

	__HAL_RCC_GPIOA_CLK_ENABLE();

	fan0.Pin = GPIO_PIN_8;
	fan0.Mode = GPIO_MODE_AF_PP;
	fan0.Pull = GPIO_NOPULL;
	fan0.Speed = GPIO_SPEED_HIGH;
	fan0.Alternate = GPIO_AF1_TIM1;

	HAL_GPIO_Init(GPIOA, &fan0);

	__HAL_RCC_GPIOF_CLK_ENABLE();

	button0.Pin = GPIO_PIN_6;
	button0.Mode = GPIO_MODE_IT_RISING;
	button0.Pull = GPIO_PULLUP;
	button0.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOF, &button0);

	button1.Pin = GPIO_PIN_0;
	button1.Mode = GPIO_MODE_IT_RISING;
	button1.Pull = GPIO_PULLUP;
	button1.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOA, &button1);

	__HAL_RCC_GPIOB_CLK_ENABLE();

	sensor.Pin = GPIO_PIN_4;
	sensor.Mode = GPIO_MODE_IT_RISING;
	sensor.Pull = GPIO_PULLUP;
	sensor.Speed = GPIO_SPEED_HIGH;
	sensor.Alternate = GPIO_AF2_TIM3;

	HAL_GPIO_Init(GPIOB, &sensor);

	printf("\n-----------------WELCOME-----------------\r\n");
	printf("**********in STATIC interrupts WS**********\r\n\n");

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI0_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	HAL_NVIC_SetPriority(TIM2_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	HAL_NVIC_SetPriority(TIM3_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	while (1) {
	}
}

void EXTI0_IRQHandler()
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI9_5_IRQHandler()
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
}

void TIM2_IRQHandler()
{
	HAL_TIM_IRQHandler(&Tim2Handle);
}

void TIM3_IRQHandler()
{
	HAL_TIM_IRQHandler(&Tim3Handle);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_6) {
		if (TIM1->CCR1 > 25) {
			TIM1->CCR1 = TIM1->CCR1 - 25;
		} else {
			TIM1->CCR1 = 0;
		}
	} else {
		if (TIM1->CCR1 < 4975) {
			TIM1->CCR1 = TIM1->CCR1 + 25;
		} else {
			TIM1->CCR1 = 5000;
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	event++;
	printf("Interrupt event: %d\nPulse rate percent: %d\n", event, ((TIM1->CCR1) / 50), TIM3->CCR1);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

}



/*void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	event++;
	if (TIM1->CCR1 > 1) {
		TIM1->CCR1 = TIM1->CCR1 - 1;
	} else {
		TIM1->CCR1 = 0;
	}
	printf("Interrupt event: %d.\nPulse rate set to: %d.\n", event, (TIM1->CCR1));
}*/

/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
PUTCHAR_PROTOTYPE {
	/* Place your implementation of fputc here */
	/* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
	HAL_UART_Transmit(&uart_handle, (uint8_t *) &ch, 1, 0xFFFF);

	return ch;
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 216000000
 *            HCLK(Hz)                       = 216000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 25000000
 *            PLL_M                          = 25
 *            PLL_N                          = 432
 *            PLL_P                          = 2
 *            PLL_Q                          = 9
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 7
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/* activate the OverDrive to reach the 216 Mhz Frequency */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static void Error_Handler(void) {
	/* User may add here some code to deal with this error */
	while (1) {
	}
}

/**
 * @brief  Configure the MPU attributes as Write Through for SRAM1/2.
 * @note   The Base Address is 0x20010000 since this memory interface is the AXI.
 *         The Region Size is 256KB, it is related to SRAM1 and SRAM2  memory size.
 * @param  None
 * @retval None
 */
static void MPU_Config(void) {
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

	/* Configure the MPU attributes as WT for SRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x20010000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void) {
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
