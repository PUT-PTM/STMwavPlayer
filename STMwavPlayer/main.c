#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "delay.h"
#include "codec.h"
#include "ff.h"
#include <stdbool.h>


FATFS fatfs;
FIL file;
u16 DMA_buffer[2048];
const char* songs[4] = {"s0.wav","s1.wav","s2.wav","s3.wav"};



int main( void )
{
	SystemInit();
	SD_init();// init SPI pod SD
	FRESULT fresult;
	disk_initialize(0);// inicjalizacja karty
	fresult = f_mount( &fatfs, 1,1 );// zarejestrowanie karty w systemie

	codec_init();
	codec_ctrl_init();

	I2S_Cmd(CODEC_I2S, ENABLE); // CODEC_I2S to SPI3
	dma_init();


	WAV_play("Beat.wav");

	for(;;)
	{ }

	return 0;
}
void dma_init()
{
	DMA_InitTypeDef  DMA_init;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Stream5);

	DMA_init.DMA_Channel = DMA_Channel_0; // kanal
	DMA_init.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI3->DR));// adres docelowy (SPI3 to kodek)
	DMA_init.DMA_Memory0BaseAddr = (uint32_t)&DMA_buffer;// adres zrodlowy
	DMA_init.DMA_DIR = DMA_DIR_MemoryToPeripheral;// memory to peripherial
	DMA_init.DMA_BufferSize = 2048;// liczba danych do przeslania (Probka 2B)
	DMA_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_init.DMA_Mode = DMA_Mode_Circular;
	DMA_init.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;// pol-slowo (flaga htif)
	DMA_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_init.DMA_FIFOMode = DMA_FIFOMode_Disable; //bez kolejki
	DMA_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;

	DMA_Init(DMA1_Stream5, &DMA_init);
	DMA_Cmd(DMA1_Stream5, ENABLE);

	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE);
	SPI_Cmd(SPI3,ENABLE);
}

void WAV_play(const char *FileName)
{
	FRESULT res;
	UINT cnt;

	res = f_open(&file, FileName, FA_READ);
	f_lseek(&file,44);
	f_read(&file, &DMA_buffer[0], 2048, &cnt);
	volatile ITStatus it_st;

	while(1)
	{
		it_st = RESET;
		while(it_st == RESET)
		{
			it_st = DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG_HTIF5);
		}
		f_read(&file, &DMA_buffer[0], 2048, &cnt);
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_HTIF5);
		if(cnt<1024)break;

		it_st = RESET;
		while(it_st == RESET)
		{
		     it_st = DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG_TCIF5);
		}
		f_read (&file,&DMA_buffer[1024],2048,&cnt);
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5 );
		if(cnt<1024)break;
	}
	f_close(&file);
}
