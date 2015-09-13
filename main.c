/* Chicken coop garage door opener */

#include <string.h>
#include "stm8s.h"
#include "stdio.h"
#include "time.h"

// #define SPEEDUP_MODE  /* enable to speed things up by x3600 */
// #define SERIAL_DEBUG

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

#ifdef SPEEDUP_MODE
#define AWU_TBR_TIMEBASE AWU_TBR_TIMEBASE_32MS
#define AWU_APR_TIMEBASE AWU_APR_TIMEBASE_32MS
#else
#define AWU_TBR_TIMEBASE AWU_TBR_TIMEBASE_30S
#define AWU_APR_TIMEBASE AWU_APR_TIMEBASE_30S
#endif
#define DELAY_LOOP_ONE_SEC 1216613
/* Private variables ---------------------------------------------------------*/

#define LED_PIN GPIO_PIN_5
#define LED_PORT GPIOB

#define DOOR_CONTROL_PIN GPIO_PIN_3
#define DOOR_CONTROL_PORT GPIOD

#define DOOR_SENSE_PIN GPIO_PIN_2
#define DOOR_SENSE_PORT GPIOD

#define INT_60HZ_PIN GPIO_PIN_6
#define INT_60HZ_PORT GPIOC

#define HSICLOCKFREQ 16000000
#define BAUDRATE  115200
#define BAUD_DIV ((((HSICLOCKFREQ * 2) / BAUDRATE) + 1) / 2)  /* round up */

struct MAIN_DATA
	{
	short min_of_day;
	char minute;
#define MINS_PER_HR 60
#define SECS_PER_MIN 60
	char hour;
#define HRS_PER_DAY 24
	volatile char door_on_time;
#define DOOR_ON_TIME 60  /* in 60 Hz ticks */

	volatile char mins_int;
	short ticks_60Hz;
	volatile unsigned char im_ok_cntr;  /* in 60 Hz ticks */
	unsigned char im_ok_cntr_set;  /* acts as a state also */
#define IM_OK_TIME_OK 60  /* everything AOK */
#define IM_OK_TIME_SENSE_ERR 30  /* sense error for period */
#define IM_OK_TIME_RESET_ERR 15  /* bad news, if we get here, we stay */
	char sec_30_int;
	volatile char got_60Hz_int;
	short day_of_year;
	short days_per_year;
#define DAYS_PER_YEAR 365
#define DAYS_PER_LEAP_YEAR 366
	char year;
	short sunset_mins;
	short sunrise_mins;
	char sunrise_delay;  /* 0: nothing 1:open scheduled N:counting down to 1 */
#define SUNRISE_DELAY 30  /* in min's */
	char sunset_delay;  /* 0: nothing 1:close scheduled N:counting down to 1 */
#define SUNSET_DELAY 30  /* in min's */

#ifdef SERIAL_DEBUG
#define TX_BUFFER_SIZE 100
#define RX_BUFFER_SIZE 10
	uint8_t TxBuffer[TX_BUFFER_SIZE];
	uint8_t RxBuffer[RX_BUFFER_SIZE];
	uint8_t *tx_hd;
	uint8_t *tx_tl;
#endif
	} main_data = {
	.min_of_day = HR_INIT * MINS_PER_HR + MIN_INIT,
	.minute = MIN_INIT,
	.hour = HR_INIT,
	.mins_int = MIN_INIT,
	.day_of_year = DOY_INIT,
	.days_per_year = DAYS_PER_YEAR,
	.year = YR_INIT,
	.sunset_mins = SUNSET_MINS_INIT,
	.sunrise_mins = SUNRISE_MINS_INIT,
	.im_ok_cntr_set = IM_OK_TIME_OK,
	};

#ifdef SERIAL_DEBUG
void uart_write(struct MAIN_DATA *p_data, const char *str)
	{
	int cntr, length = strlen(str);

	for(cntr = 0; cntr < length; cntr++)
		{
		*p_data->tx_hd++ = *str++;
		if (p_data->tx_hd > &p_data->TxBuffer[TX_BUFFER_SIZE - 1])
			{
			p_data->tx_hd = p_data->TxBuffer;
			}
		}
	UART1->CR2 |= (uint8_t) UART1_CR2_TCIEN;
	return;
}

static void default_handler(int interrupt)
	{
	struct MAIN_DATA *p_data = &main_data;
	unsigned char buf[20];

	sprintf(buf, "default_handler %d\r\n", interrupt);
	uart_write(p_data, buf);
	}

INTERRUPT_HANDLER(TLI_IRQHandler, 0)	{ default_handler(0); }
// INTERRUPT_HANDLER(AWU_IRQHandler, 1)	{ default_handler(1); }
INTERRUPT_HANDLER(CLK_IRQHandler, 2)	{ default_handler(2); }
INTERRUPT_HANDLER(EXTI_PORTA_IRQHandler, 3)	{ default_handler(3); }
INTERRUPT_HANDLER(EXTI_PORTB_IRQHandler, 4)	{ default_handler(4); }
//INTERRUPT_HANDLER(EXTI_PORTC_IRQHandler, 5)	{ default_handler(5); }
INTERRUPT_HANDLER(EXTI_PORTD_IRQHandler, 6)	{ default_handler(6); }
INTERRUPT_HANDLER(EXTI_PORTE_IRQHandler, 7)	{ default_handler(7); }
INTERRUPT_HANDLER(EXTI_PORTF_IRQHandler, 8)	{ default_handler(8); }
INTERRUPT_HANDLER(CAN_TX_IRQHandler, 9)	{ default_handler(9); }
INTERRUPT_HANDLER(SPI_IRQHandler, 10)	{ default_handler(10); }
INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_BRK_IRQHandler, 11)	{ default_handler(11); }
INTERRUPT_HANDLER(TIM1_CAP_COM_IRQHandler, 12)	{ default_handler(12); }
INTERRUPT_HANDLER(TIM2_UPD_OVF_BRK_IRQHandler, 13)	{ default_handler(13); }
INTERRUPT_HANDLER(TIM2_CAP_COM_IRQHandler, 14)	{ default_handler(14); }
INTERRUPT_HANDLER(TIM3_UPD_OVF_BRK_IRQHandler, 15)	{ default_handler(15); }
INTERRUPT_HANDLER(TIM3_CAP_COM_IRQHandler, 16)	{ default_handler(16); }
// INTERRUPT_HANDLER(UART1_TX_IRQHandler, 17)	{ default_handler(17); }
INTERRUPT_HANDLER(UART1_RX_IRQHandler, 18)	{ default_handler(18); }
INTERRUPT_HANDLER(I2C_IRQHandler, 19)	{ default_handler(19); }
INTERRUPT_HANDLER(ADC1_IRQHandler, 22)	{ default_handler(22); }
INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)	{ default_handler(23); }
INTERRUPT_HANDLER(EEPROM_EEC_IRQHandler, 24)	{ default_handler(24); }


static void uart_init(struct MAIN_DATA *p_data)
	{

	p_data->tx_hd = p_data->tx_tl = p_data->TxBuffer;

	GPIOD->DDR |= GPIO_PIN_5; /* Put TX line on as output */
	GPIOD->CR1 |= GPIO_PIN_5;

	UART1->CR2 = UART1_CR2_TEN; /* Allow TX */
	UART1->CR3 &= ~(UART1_CR3_STOP); /* 1 stop bit */

	UART1->BRR2 = (BAUD_DIV & 0x0f) | ((BAUD_DIV & 0xf000) >> 8);
	UART1->BRR1 = (BAUD_DIV & 0x0ff0) >> 4;
	}

INTERRUPT_HANDLER(UART1_TX_IRQHandler, 17)
 {
	struct MAIN_DATA *p_data = &main_data;
	uint8_t temp;

	temp = UART1->SR;  /* must read to clear interrupt */
  if (p_data->tx_tl != p_data->tx_hd) {
		UART1->DR = *p_data->tx_tl++;
		if (p_data->tx_tl > &p_data->TxBuffer[TX_BUFFER_SIZE - 1]) {
			p_data->tx_tl = p_data->TxBuffer;
			}
	}	else {
    /* Disable the USART Transmit Complete interrupt */
		UART1->CR2 &= ~UART1_CR2_TCIEN;
		}
  return;
 }
#endif

/* 60 Hz line frequency interrupt */
INTERRUPT_HANDLER(EXTI_PORTC_IRQHandler, 5)
{
	struct MAIN_DATA *p_data = &main_data;

	p_data->got_60Hz_int = 1;
#ifndef SPEEDUP_MODE
	if (++p_data->ticks_60Hz >= SECS_PER_MIN * 60)  /* 1 min? */
#endif
		{
		p_data->ticks_60Hz = 0;
		if (++p_data->mins_int >= MINS_PER_HR)  /* 1 min? */
			p_data->mins_int = 0;
		}
	if (p_data->door_on_time && --p_data->door_on_time <= 0)
			{
			DOOR_CONTROL_PORT->ODR &= ~DOOR_CONTROL_PIN;  /* turn DOOR_CONTROL off */
			}
	if (p_data->im_ok_cntr && --p_data->im_ok_cntr <= 0)
		LED_PORT->ODR ^= LED_PIN;  /* toggle LED */
}

/**
  * @brief  Auto Wake Up Interrupt routine
  * @param  None
  * @retval None
  */
#define CLOCK_ADJUST (55 * 2)  /* add one min every 55 min's */
INTERRUPT_HANDLER(AWU_IRQHandler, 1)
	{
	struct MAIN_DATA *p_data = &main_data;
	uint8_t temp;

  /* Clear AWU peripheral pending bit */
	temp = AWU->CSR;  /* read to clear */
	if (p_data->got_60Hz_int)
		p_data->got_60Hz_int = 0;
	else
		{
		DOOR_SENSE_PORT->CR1 &= ~DOOR_SENSE_PIN;  /* remove pull up to save power */
		LED_PORT->ODR |= LED_PIN;  /* turn LED off to save power */
		if (++p_data->sec_30_int >= CLOCK_ADJUST)
			{  /* tweak clock a bit when power failure since we run slow */
			p_data->sec_30_int = 0;
			if (++p_data->mins_int >= MINS_PER_HR)  /* Hour? */
				p_data->mins_int = 0;
			}
		if (!(p_data->sec_30_int & 1))  /* only do every other one */
			{
			if (++p_data->mins_int >= MINS_PER_HR)  /* Hour? */
				p_data->mins_int = 0;
			}
		}
	}

void awu_init(void)
	{
  
  /* Set the AWUTB TimeBase */
  AWU->TBR = AWU_TBR_TIMEBASE;
  /* Set the APR prescaler divider */
  AWU->APR = AWU_APR_TIMEBASE;
  /* Enable the AWU peripheral */
  AWU->CSR = AWU_CSR_AWUEN;

  /* before going into Active HALT, do:
		 AHALT bit in Flash control register 1 (FLASH_CR1). This will switch
		 the Flash to power-down state when entering Active-halt mode */

  FLASH->CR1 |= FLASH_CR1_AHALT;
	CLK->ICKR |= CLK_ICKR_SWUAH;  /* really REGAH bit in the CLK_ICKR */
	}

void gpio_init(void)
	{
  /* Initialize LED pins in Output Mode */
  
	LED_PORT->DDR |= LED_PIN;  /* Configure LED pins */
	LED_PORT->CR1 |= LED_PIN;
	LED_PORT->ODR &= ~LED_PIN;  /* turn LED on so we can tell we did a reset */
	
	DOOR_CONTROL_PORT->DDR |= DOOR_CONTROL_PIN;  /* Configure DOOR_CONTROL pins */
	DOOR_CONTROL_PORT->CR1 |= DOOR_CONTROL_PIN;
	DOOR_CONTROL_PORT->ODR &= ~DOOR_CONTROL_PIN;  /* turn DOOR_CONTROL off */

	DOOR_SENSE_PORT->DDR &= ~DOOR_SENSE_PIN;  /* Configure DOOR_SENSE pins */

  EXTI->CR1 &= ~EXTI_CR1_PCIS;
	EXTI->CR1 |= 0x02 << 4;  /* 10: Falling edge only */

	INT_60HZ_PORT->DDR &= ~INT_60HZ_PIN;  /* Configure INT_60HZ input */
	INT_60HZ_PORT->CR1 &= ~INT_60HZ_PIN;  /* no pull up */
	INT_60HZ_PORT->CR2 |= INT_60HZ_PIN;  /* interrupt on */

	}

int main()
	{
	struct MAIN_DATA *p_data = &main_data;
	long cntr;
	unsigned char tmp;

#ifdef SERIAL_DEBUG
	unsigned char buf[80];
#endif

	CLK->CKDIVR = 0; /* Set the frequency to 16 MHz */
#ifdef SERIAL_DEBUG
	CLK->PCKENR1 = CLK_PCKENR1_UART1; /* Enable peripherals */
#endif
	CLK->PCKENR2 = CLK_PCKENR2_AWU; /* Enable peripherals */

	gpio_init();
#ifdef SERIAL_DEBUG
  uart_init(p_data);
#endif
	awu_init();

	enableInterrupts();
	for(cntr = 0; cntr < DELAY_LOOP_ONE_SEC / 8; cntr++) // Sleep
		;  /* wait to see if we get 60 Hz int's */
	tmp = RST->SR;
#ifdef SERIAL_DEBUG
	sprintf(buf, "start test RST=0x%x, pwr=%d\r\n", tmp, p_data->got_60Hz_int);
	uart_write(p_data, buf);
#endif

	if (!(tmp & RST_SR_SWIMF))  /* did we reset from something other than programming? */
		p_data->im_ok_cntr_set = IM_OK_TIME_RESET_ERR;  /* fatal error */
	RST->SR = tmp;  /* clear reset reg */
	if (p_data->got_60Hz_int)  /* if we did then set LED on for a bit */
		p_data->im_ok_cntr = IM_OK_TIME_OK * 4;
	do
		{
		if (!p_data->im_ok_cntr)
			p_data->im_ok_cntr = p_data->im_ok_cntr_set;
#ifdef SERIAL_DEBUG
		while (UART1->CR2 & UART1_CR2_TCIEN)  /* stay here until sent before going into halt */
			;
#endif
		halt();  /* wait for interrupt */
		while (p_data->minute != p_data->mins_int)
			{  /* a minute has passed */
			if (++p_data->min_of_day >= MINS_PER_HR * HRS_PER_DAY)
				p_data->min_of_day = 0;
			if (p_data->sunrise_mins == p_data->min_of_day)
				{  /* sunrise time */
				p_data->sunrise_delay = SUNRISE_DELAY;
#ifdef SERIAL_DEBUG
				sprintf(buf, "sunrise %02d:%03d:%02d:%02d rise mins %d\r\n", p_data->year, p_data->day_of_year, p_data->hour, p_data->minute, p_data->sunrise_mins );
				uart_write(p_data, buf);
#endif
				}
			if (p_data->sunset_mins == p_data->min_of_day)
				{  /* sunset time */
				p_data->sunset_delay = SUNSET_DELAY;
#ifdef SERIAL_DEBUG
				sprintf(buf, "sunset %02d:%03d:%02d:%02d set mins %d\r\n", p_data->year, p_data->day_of_year, p_data->hour, p_data->minute, p_data->sunset_mins );
				uart_write(p_data, buf);
#endif
				}
			if (++p_data->minute >= MINS_PER_HR)
				{  /* an hour has passed */
				p_data->minute = 0;
#if defined(SPEEDUP_MODE) && defined(SERIAL_DEBUG)
				sprintf(buf, "Hour %02d rise %d set %d door %d\r\n", p_data->hour, p_data->sunrise_mins, p_data->sunset_mins, DOOR_SENSE_PORT->IDR & DOOR_SENSE_PIN);
				uart_write(p_data, buf);
#endif
				if (++p_data->hour >= HRS_PER_DAY)
					{  /* a day has passed */
					p_data->hour = 0;

					p_data->sunrise_mins += sunrise_lut[p_data->day_of_year];
					p_data->sunset_mins += sunset_lut[p_data->day_of_year];
					if (++p_data->day_of_year >= p_data->days_per_year)
						{  /* a new year */
						p_data->day_of_year = 0;
						p_data->sunrise_mins = SUNRISE_JAN1_MINS_INIT;
						p_data->sunset_mins = SUNSET_JAN1_MINS_INIT;
						p_data->year++;
						if ((p_data->year & 3) == 0)  /* leap? */
							p_data->days_per_year = DAYS_PER_LEAP_YEAR;
						else
							p_data->days_per_year = DAYS_PER_YEAR;
						}
					}
				}
			/* count down but don't turn on door unless we are powered up (via 60Hz int's) */
			if (p_data->sunrise_delay > 1)
				p_data->sunrise_delay--;
			if (p_data->sunset_delay > 1)
				p_data->sunset_delay--;
			if (p_data->got_60Hz_int)  /* line powered? */
				{
				if (!(DOOR_SENSE_PORT->CR1 & DOOR_SENSE_PIN))  /* is pull up enabled? */
					{
					DOOR_SENSE_PORT->CR1 |= DOOR_SENSE_PIN;  /* apply pull up, then wait until next cycle */
					}
				else
					{
#ifndef SPEEDUP_MODE
#ifdef SERIAL_DEBUG
					sprintf(buf, "time %02d:%02d rise %d set %d door %d\r\n", p_data->hour, p_data->minute, p_data->sunrise_mins, p_data->sunset_mins, DOOR_SENSE_PORT->IDR & DOOR_SENSE_PIN);
					uart_write(p_data, buf);
#endif
#endif
					if (p_data->sunrise_delay == 1)  /* ready to be opened? */
						{
						p_data->sunrise_delay = 0;
						if (!(DOOR_SENSE_PORT->IDR & DOOR_SENSE_PIN))
							{
							DOOR_CONTROL_PORT->ODR |= DOOR_CONTROL_PIN;  /* turn DOOR_CONTROL on */
							p_data->door_on_time = DOOR_ON_TIME;
							if (p_data->im_ok_cntr_set != IM_OK_TIME_RESET_ERR)
								p_data->im_ok_cntr_set = IM_OK_TIME_OK;
							}
						else
							{
#ifdef SERIAL_DEBUG
							sprintf(buf, "Error: door was supposed to be closed\r\n" );
							uart_write(p_data, buf);
#endif
							if (p_data->im_ok_cntr_set != IM_OK_TIME_RESET_ERR)
								p_data->im_ok_cntr_set = IM_OK_TIME_SENSE_ERR;
							}							
						}
					if (p_data->sunset_delay == 1)  /* ready to be closed? */
						{
						p_data->sunset_delay = 0;
						if (DOOR_SENSE_PORT->IDR & DOOR_SENSE_PIN)
							{
							DOOR_CONTROL_PORT->ODR |= DOOR_CONTROL_PIN;  /* turn DOOR_CONTROL on */
							p_data->door_on_time = DOOR_ON_TIME;
							if (p_data->im_ok_cntr_set != IM_OK_TIME_RESET_ERR)
								p_data->im_ok_cntr_set = IM_OK_TIME_OK;
							}
						else
							{
#ifdef SERIAL_DEBUG
							sprintf(buf, "Error: door was supposed to be open\r\n" );
							uart_write(p_data, buf);
#endif
							if (p_data->im_ok_cntr_set != IM_OK_TIME_RESET_ERR)
								p_data->im_ok_cntr_set = IM_OK_TIME_SENSE_ERR;
							}							
						}							
					}
				}
			}
		}
	while(1);
	}
