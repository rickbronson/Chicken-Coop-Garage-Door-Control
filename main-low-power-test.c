#include <string.h>
#include "stm8s.h"
#include "stdio.h"
#undef CLK_PCKENR1_UART1
#define CLK_PCKENR1_UART1    ((uint8_t)0x08) /*!< UART1 clock enable */

#define AWU_TBR_TIMEBASE_NO_IT 0	
#define AWU_TBR_TIMEBASE_250US 1	
#define AWU_TBR_TIMEBASE_500US 2	
#define AWU_TBR_TIMEBASE_1MS   3	
#define AWU_TBR_TIMEBASE_2MS   4	
#define AWU_TBR_TIMEBASE_4MS   5	
#define AWU_TBR_TIMEBASE_8MS   6	
#define AWU_TBR_TIMEBASE_16MS  7	
#define AWU_TBR_TIMEBASE_32MS  8	
#define AWU_TBR_TIMEBASE_64MS  9	
#define AWU_TBR_TIMEBASE_128MS 10
#define AWU_TBR_TIMEBASE_256MS 11
#define AWU_TBR_TIMEBASE_512MS 12
#define AWU_TBR_TIMEBASE_1S    12
#define AWU_TBR_TIMEBASE_2S    14
#define AWU_TBR_TIMEBASE_12S   15
#define AWU_TBR_TIMEBASE_30S   15

#define AWU_APR_TIMEBASE_NO_IT 0	
#define AWU_APR_TIMEBASE_250US 30
#define AWU_APR_TIMEBASE_500US 30
#define AWU_APR_TIMEBASE_1MS   30
#define AWU_APR_TIMEBASE_2MS   30
#define AWU_APR_TIMEBASE_4MS   30
#define AWU_APR_TIMEBASE_8MS   30
#define AWU_APR_TIMEBASE_16MS  30
#define AWU_APR_TIMEBASE_32MS  30
#define AWU_APR_TIMEBASE_64MS  30
#define AWU_APR_TIMEBASE_128MS 30
#define AWU_APR_TIMEBASE_256MS 30
#define AWU_APR_TIMEBASE_512MS 30
#define AWU_APR_TIMEBASE_1S    61
#define AWU_APR_TIMEBASE_2S    23
#define AWU_APR_TIMEBASE_12S   23
#define AWU_APR_TIMEBASE_30S   62

/* Private variables ---------------------------------------------------------*/

#define LED (1 << 5)

#define HSICLOCKFREQ 16000000
#define BAUDRATE  115200

struct MAIN_DATA
	{
	int flags;
#define TIM2_INTERRUPT_HIT (1 << 0)
	int loop_cntr;
	volatile int tim2_cntr;
	unsigned long adc_cntr;
	int adc_conv;

#define TX_BUFFER_SIZE 100
#define RX_BUFFER_SIZE 10
	uint8_t TxBuffer[TX_BUFFER_SIZE];
	uint8_t RxBuffer[RX_BUFFER_SIZE];
	uint8_t *tx_hd;
	uint8_t *tx_tl;
	} main_data;

void uart_write(struct MAIN_DATA *p_data, const char *str)
	{
	int cntr, length = strlen(str);

	for(cntr = 0; cntr < length; cntr++) {
		*p_data->tx_hd++ = *str++;
		if (p_data->tx_hd > &p_data->TxBuffer[TX_BUFFER_SIZE - 1]) {
			p_data->tx_hd = p_data->TxBuffer;
			}
		}
	UART1->CR2 |= (uint8_t) UART1_CR2_TCIEN;
			while (UART1->CR2 & UART1_CR2_TCIEN)
				;
//	UART1->CR2 |= UART1_CR2_TIEN;
//		GPIOB->ODR ^= LED;
	return;
}

INTERRUPT_HANDLER(TIM2_UPD_OVF_BRK_IRQHandler, 13)
 {
	struct MAIN_DATA *p_data = &main_data;
  /* Clear the IT pending Bit */
 TIM2->SR1 = (uint8_t) ~TIM2_IT_UPDATE;
 p_data->flags |= TIM2_INTERRUPT_HIT;
 p_data->tim2_cntr++;
// GPIOB->ODR ^= LED;
 return;
 }

INTERRUPT_HANDLER(UART1_TX_IRQHandler, 17)
 {
	struct MAIN_DATA *p_data = &main_data;
	uint8_t temp;

	temp = UART1->SR;  /* must read to clear interrupt */
  if (p_data->tx_tl != p_data->tx_hd) {
		UART1->DR = *p_data->tx_tl++;
		if (p_data->tx_tl > &p_data->TxBuffer[TX_BUFFER_SIZE - 1]) {
//			GPIOB->ODR ^= LED;
			p_data->tx_tl = p_data->TxBuffer;
			}
	}	else {
    /* Disable the USART Transmit Complete interrupt */
		UART1->CR2 &= ~UART1_CR2_TCIEN;
		}
  return;
 }

INTERRUPT_HANDLER(ADC2_IRQHandler, 22)
 {
	struct MAIN_DATA *p_data = &main_data;
	uint8_t temp;
	
	temp = ADC1->DRH;  /* Read MSB first*/
	p_data->adc_conv = (temp << 8) | ADC1->DRL;
	p_data->adc_cntr++;
  ADC1->CSR &= (uint8_t)(~ADC1_CSR_EOC);  /* clear pending bit */
  return;
 }

/**
  * @brief  Auto Wake Up Interrupt routine
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(AWU_IRQHandler, 1)
	{
	uint8_t temp;

  /* Clear AWU peripheral pending bit */
	temp = AWU->CSR;  /* read to clear */
	}

#define TIM2_PERIOD (HSICLOCKFREQ / 32768 / 2)  /* set to 500ms */
static void timer2_init()
	{
  /* Set the Prescaler value */
  TIM2->PSCR = (uint8_t)TIM2_PRESCALER_32768;
  /* Set the Autoreload value */
  TIM2->ARRH = (uint8_t)(TIM2_PERIOD >> 8);
  TIM2->ARRL = (uint8_t)(TIM2_PERIOD);
	
	TIM2->CR1 = (uint8_t)TIM2_CR1_CEN;  /* ~TIM2_CR1_ARPE ~TIM2_CR1_OPM ~TIM2_CR1_UDIS ~TIM2_CR1_URS */
	TIM2->IER |= (uint8_t)TIM2_IT_UPDATE;  /* Enable the Interrupt sources */
	}

static void uart_init(struct MAIN_DATA *p_data)
	{
	unsigned int baud_div;

	p_data->tx_hd = p_data->tx_tl = p_data->TxBuffer;

	GPIOD->DDR = GPIO_PIN_5; // Put TX line on as output
	GPIOD->CR1 = GPIO_PIN_5;

	UART1->CR2 = UART1_CR2_TEN; // Allow TX
	UART1->CR3 &= ~(UART1_CR3_STOP); // 1 stop bit
	baud_div = (HSICLOCKFREQ * 2) / BAUDRATE;
	baud_div++;  /* round up */
	baud_div /= 2;
//	UART1->BRR2 = 0x03; UART1->BRR1 = 0x68; // 9600 baud
	UART1->BRR2 = baud_div & 0x0f;
	UART1->BRR2 |= (baud_div & 0xf000) >> 8;
	UART1->BRR1 = (baud_div & 0x0ff0) >> 4;
	}

void awu_init(void)
	{
  
  /* Set the AWUTB TimeBase */
  AWU->TBR = AWU_TBR_TIMEBASE_12S;
  /* Set the APR prescaler divider */
  AWU->APR = AWU_APR_TIMEBASE_12S;
  /* Enable the AWU peripheral */
  AWU->CSR = AWU_CSR_AWUEN;

  /* before going in to Active HALT, do:
		 AHALT bit in Flash control register 1 (FLASH_CR1). This will switch
		 the Flash to power-down state when entering Active-halt mode */

  FLASH->CR1 |= FLASH_CR1_AHALT;
	CLK->ICKR |= CLK_ICKR_SWUAH;  /* really REGAH bit in the CLK_ICKR */
	}

#define ADC1_CHANNEL 2  /* AIN2 */

void adc_init(void)
	{
	unsigned int cntr;
  /* Clear the align bit */
  ADC1->CR2 = ADC1_CR2_ALIGN;  /* ~ADC1_CR2_EXTSEL */
    /* Enable the selected external Trigger */
//	ADC1->CR2 |= (uint8_t)(ADC1_CR2_EXTTRIG);
  ADC1->CSR = ADC1_CSR_EOCIE | ADC1_CHANNEL;
	ADC1->TDRL = (1 << ADC1_CHANNEL);  /* disable SchmittTriggerConfig */
	/* Set the continuous conversion mode */
  ADC1->CR1 = ADC1_CR1_CONT | ADC1_PRESSEL_FCPU_D18;
  /* start */
  ADC1->CR1 |= ADC1_CR1_ADON;  /* CAUION, can't change any other bits to start */

	for(cntr = 0; cntr < 10000; cntr++)
		; // Sleep
	ADC1->CR1 |= ADC1_CR1_ADON;  /* start 1st conversion */
}

int main()
	{
	struct MAIN_DATA *p_data = &main_data;
	unsigned char buf[30];
	int cntr = 0;

	CLK->CKDIVR = 0; /* Set the frequency to 16 MHz */
	CLK->PCKENR1 = CLK_PCKENR1_UART1 | CLK_PCKENR1_TIM2; /* Enable peripherals */
	CLK->PCKENR2 = CLK_PCKENR2_ADC | CLK_PCKENR2_AWU; /* Enable peripherals */

/* gpio_init:
	 configured as input with internal pull-up/down resistor,
*/
  uart_init(p_data);
	timer2_init();
	adc_init();
	awu_init();

	enableInterrupts();
  /* Initialize LED pins in Output Mode */
  
	GPIOB->DDR = LED;  /* Configure LED pins */
	GPIOB->CR1 = LED;
	GPIOB->ODR |= LED;  /* turn LED off */
	
	sprintf(buf, "start test\r\n");
	uart_write(p_data, buf);
	do {
		sprintf(buf, "test1 %d\r\n", p_data->tim2_cntr);
		uart_write(p_data, buf);
		p_data->flags &= ~TIM2_INTERRUPT_HIT;
		halt();  /* wait for interrupt */
		sprintf(buf, "test2 %d\r\n", (int) p_data->tim2_cntr);
		uart_write(p_data, buf);
		GPIOB->ODR ^= LED;
		for (cntr = p_data->tim2_cntr + (2 * 2); p_data->tim2_cntr < cntr; )
			;
		sprintf(buf, "test3 %d\r\n", (int) p_data->tim2_cntr);
		uart_write(p_data, buf);
		GPIOB->ODR ^= LED;
		for (cntr = p_data->tim2_cntr + (3 * 2); p_data->tim2_cntr < cntr; )
			;
		} while(1);
	}
