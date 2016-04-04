#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"

uint8_t spi_sendrecv(uint8_t byte)
{
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI3, byte);

	while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI3);
}


int main(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_SPI3, ENABLE);
	RCC_PLLI2SCmd(ENABLE);

	GPIO_InitTypeDef PinInitStruct;
	PinInitStruct.GPIO_Pin = GPIO_Pin_4;
	PinInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	PinInitStruct.GPIO_OType = GPIO_OType_PP;
	PinInitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	PinInitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOD, &PinInitStruct);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI3);

	I2S_InitTypeDef I2S_InitType;
	I2S_InitType.I2S_AudioFreq = I2S_AudioFreq_48k;
	I2S_InitType.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitType.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitType.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitType.I2S_Standard = I2S_Standard_Phillips;
	I2S_InitType.I2S_CPOL = I2S_CPOL_Low;

	I2S_Init(SPI3, &I2S_InitType);
	I2S_Cmd(SPI3, ENABLE);

	I2C_InitTypeDef I2C_InitType;
	I2C_InitType.I2C_ClockSpeed = 100000;
	I2C_InitType.I2C_Mode = I2C_Mode_I2C;
	I2C_InitType.I2C_OwnAddress1 = 99;
	I2C_InitType.I2C_Ack = I2C_Ack_Enable;
	I2C_InitType.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitType.I2C_DutyCycle = I2C_DutyCycle_2;

	I2C_Init(I2C1, &I2C_InitType);
	I2C_Cmd(I2C1, ENABLE);


	SystemInit();

		fir_8 filt;

		//enables GPIO clock for PortD
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOD, &GPIO_InitStructure);

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

		codec_init();
		codec_ctrl_init();

		I2S_Cmd(CODEC_I2S, ENABLE);

		initFilter(&filt);

}
