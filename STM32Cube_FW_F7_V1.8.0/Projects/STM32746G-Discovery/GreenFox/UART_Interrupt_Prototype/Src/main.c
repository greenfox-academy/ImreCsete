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
#include "string.h"
#include "stdio.h"

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

UART_HandleTypeDef UartHandle;
DMA_HandleTypeDef DMAHandle;
GPIO_InitTypeDef UartTransmit;
GPIO_InitTypeDef UartRecieve;
TIM_HandleTypeDef InterruptTimHandle;

char g_code_buffer[100];
volatile uint8_t event_counter = 0;

/* Private function prototypes -----------------------------------------------*/

void UART_Config(UART_HandleTypeDef *huart);
void DMA_Init();
void USART1_IRQHandler();
void UART_Buffer_Reset();
void Button_Interrupt_Init();
void EXTI15_10_IRQHandler();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void Test_Timer_Init();
void TIM2_IRQHandler();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void DMA1_Stream1_IRQHandler();
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

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


int main(void) {

	MPU_Config();
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();

	UART_Config(&UartHandle);

	HAL_UART_Init(&UartHandle);

	__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE);
	//__HAL_DMA_ENABLE_IT(&DMAHandle, )

	printf("UART interrupt test\n");

	Button_Interrupt_Init();
	//DMA_Init();
	HAL_UART_Receive_IT(&UartHandle, (uint8_t*)g_code_buffer, 80);
	Test_Timer_Init();

	while (1) {
		printf("Contents of g_code_buffer: ");
			for (int i = 0; i < (sizeof(g_code_buffer)/sizeof(g_code_buffer[0])); i++) {
					printf("%c", g_code_buffer[i]);
			}
		printf("\n");
		HAL_Delay(1000);
	}
}

void UART_Config(UART_HandleTypeDef *huart) {

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	__HAL_RCC_USART1_CLK_ENABLE();

	UartTransmit.Pin 		= GPIO_PIN_9;
	UartTransmit.Mode       = GPIO_MODE_AF_PP;
	UartTransmit.Pull		= GPIO_PULLUP;
	UartTransmit.Speed 		= GPIO_SPEED_FAST;
	UartTransmit.Alternate  = GPIO_AF7_USART1;

	HAL_GPIO_Init(GPIOA, &UartTransmit);

	UartRecieve.Pin 		= GPIO_PIN_7;
	UartRecieve.Mode        = GPIO_MODE_AF_PP;
	UartRecieve.Speed 		= GPIO_SPEED_FAST;
	UartRecieve.Alternate   = GPIO_AF7_USART1;

	HAL_GPIO_Init(GPIOB, &UartRecieve);

	huart->Instance = USART1;
	huart->Init.BaudRate = 115200;
	huart->Init.WordLength = UART_WORDLENGTH_8B;
	huart->Init.StopBits = UART_STOPBITS_1;
	huart->Init.Parity = UART_PARITY_NONE;
	huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart->Init.Mode = UART_MODE_TX_RX;

	HAL_NVIC_SetPriority(USART1_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

}

void DMA_Init()
{
	__HAL_RCC_DMA1_CLK_ENABLE();

	DMAHandle.Instance = DMA1;
	DMAHandle.Init.Channel = DMA1_Stream0;
	DMAHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
	DMAHandle.Init.PeriphInc = DMA_PINC_DISABLE;
	DMAHandle.Init.MemInc = DMA_MINC_ENABLE;
	DMAHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	DMAHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	DMAHandle.Init.Mode = DMA_CIRCULAR;
	DMAHandle.Init.Priority = DMA_PRIORITY_VERY_HIGH;

    HAL_DMA_Init(&DMAHandle);

	__HAL_LINKDMA(&UartHandle, hdmarx, DMAHandle);

	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}

void USART1_IRQHandler()
{
	HAL_UART_IRQHandler(&UartHandle);
}

void UART_Buffer_Reset()
{
	memset(&g_code_buffer[0], '\0', sizeof(g_code_buffer));
	UartHandle.RxXferCount = 80;
}

void Button_Interrupt_Init()
{
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
}

void EXTI15_10_IRQHandler()
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	event_counter++;
	UART_Buffer_Reset();
	printf("Buffer reset interrupt event: %d\n", event_counter);
}

void Test_Timer_Init()
{
	__HAL_RCC_TIM2_CLK_ENABLE();

	InterruptTimHandle.Instance               = TIM2;
	InterruptTimHandle.Init.Period            = 16460;
	InterruptTimHandle.Init.Prescaler         = (0xFFFF / 5);
	InterruptTimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	InterruptTimHandle.Init.CounterMode 	  = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&InterruptTimHandle);
	HAL_TIM_Base_Start_IT(&InterruptTimHandle);

	HAL_NVIC_SetPriority(TIM2_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler()
{
	HAL_TIM_IRQHandler(&InterruptTimHandle);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	printf("UART Receive Limit Counter: %d\n", UartHandle.RxXferCount);

	if (HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_READY) {
		printf("UART: Peripheral Initialized and ready for use.\n");
	}

	if (HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_BUSY_RX) {
			printf("UART: Data Transmission process is ongoing.\n");
		}

	if (HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_BUSY_TX_RX) {
			printf("UART: Data Transmission and Reception process is ongoing.\n");
		}

	if (HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_TIMEOUT) {
			printf("UART: Timeout state\n");
		}

	if (HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_ERROR) {
			printf("UART: Error.\n");
		}
}

void DMA1_Stream1_IRQHandler()
{
  HAL_DMA_IRQHandler(&DMAHandle);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	printf("HAL_UART_RxCpltCallback Interrupt event\n");
}

/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
PUTCHAR_PROTOTYPE {
	/* Place your implementation of fputc here */
	/* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
	HAL_UART_Transmit(&UartHandle, (uint8_t *) &ch, 1, 0xFFFF);

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
