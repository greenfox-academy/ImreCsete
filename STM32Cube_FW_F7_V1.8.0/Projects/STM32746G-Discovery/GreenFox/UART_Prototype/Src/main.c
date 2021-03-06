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
#include <stdint.h>
#include <stddef.h>

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
GPIO_InitTypeDef UartTransmit;
GPIO_InitTypeDef UartRecieve;

volatile uint32_t timIntPeriod;

// typedef ring buffer

typedef unsigned int rb_descriptor; // rbd_t

typedef struct {
    size_t element_size;
    size_t element_number;
    void *buffer;
} rb_attributes; // rb_attr_t

#define RING_BUFFER_MAX 1

//ring buffer initialization

struct ring_buffer
{
    size_t element_size;
    size_t element_number;
    uint8_t *buffer_array; // *buf
    volatile size_t head;
    volatile size_t tail;
};

static struct ring_buffer _rb[RING_BUFFER_MAX];

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

void UART_Config(UART_HandleTypeDef *huart);
int Ring_Buffer_Init(rb_descriptor *rbd, rb_attributes *attr); // init a ring buffer, pointer to a ring buffer descriptor, attr - ring buffer attributes
int Ring_Buffer_Put(rb_descriptor rbd, const void *data); // Add an element to the ring buffer, the ring buffer descriptor, data - the data to add
int Ring_Buffer_Get(rb_descriptor rbd, void *data); // Get and remove an element from the ring buffer, rb - the ring buffer descriptor, data - pointer to store the data
static int Ring_Buffer_Full(struct ring_buffer *rb);
static int Ring_Buffer_Empty(struct ring_buffer *rb);

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
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

	/* Add your application code here
	 */

	UART_Config(&UartHandle);
	HAL_UART_Init(&UartHandle);

	static rb_descriptor ring_buffer_test;
	static char ring_buffer_array[16];

	char *uart_test = "UART test line";

	rb_attributes ring_buffer_test_attributes = {sizeof(ring_buffer_array[0]), (sizeof(ring_buffer_array) / sizeof(ring_buffer_array[0])), ring_buffer_array};

	Ring_Buffer_Init(&ring_buffer_test, &ring_buffer_test_attributes);

	Ring_Buffer_Put(ring_buffer_test, &uart_test);

	while (1) {

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
}

int Ring_Buffer_Init(rb_descriptor *rbd, rb_attributes *attr)
{
    static int index = 0;
    int error = -1;

    if ((index < RING_BUFFER_MAX) && (rbd != NULL) && (attr != NULL)) {
        if ((attr->buffer != NULL) && (attr->element_size > 0)) { // check size if 2^x
            if (((attr->element_number - 1) & attr->element_number) == 0) {
                _rb[index].head = 0;
                _rb[index].tail = 0;
                _rb[index].buffer_array = attr->buffer;
                _rb[index].element_size = attr->element_size;
                _rb[index].element_number = attr->element_number; // init rb variables

                *rbd = index++;
                error= 0;
            }
        }
    }

    return error;
}

int Ring_Buffer_Put(rb_descriptor rbd, const void *data)
{
    int error = 0;

    if ((rbd < RING_BUFFER_MAX) && (Ring_Buffer_Full(&_rb[rbd]) == 0)) {
        const size_t offset = (_rb[rbd].head & (_rb[rbd].element_number - 1)) * _rb[rbd].element_size;
        memcpy(&(_rb[rbd].buffer_array[offset]), data, _rb[rbd].element_size);
        _rb[rbd].head++;
    } else {
    	error = -1;
    }

    return error;
}

int Ring_Buffer_Get(rb_descriptor rbd, void *data)
{
    int error = 0;

    if ((rbd < RING_BUFFER_MAX) && (Ring_Buffer_Empty(&_rb[rbd]) == 0)) {
        const size_t offset = (_rb[rbd].tail & (_rb[rbd].element_number - 1)) * _rb[rbd].element_size;
        memcpy(data, &(_rb[rbd].buffer_array[offset]), _rb[rbd].element_size);
        _rb[rbd].tail++;
    } else {
    	error = -1;
    }

    return error;
}

static int Ring_Buffer_Full(struct ring_buffer *rb)
{
    return ((rb->head - rb->tail) == rb->element_number) ? 1 : 0;
}

static int Ring_Buffer_Empty(struct ring_buffer *rb)
{
    return ((rb->head - rb->tail) == 0U) ? 1 : 0;
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
