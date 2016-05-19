///*	Features:
// 	 - FatFS by Chan supported, version RO. 11
// 	 "tm_stm32f4_delay.h"
// 	 "tm_stm32f4_disco.h"	Read SD card with FatFs by Tilen Majerle
// 	 "tm_stm32f4_fatfs.h"
//
// 	 "codec.h" and "codec.c"
//*/
//
#include "defines.h"
#include "stm32f4xx.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_disco.h"
#include "tm_stm32f4_fatfs.h"
#include "stdbool.h"
#include <stdio.h>
#include <string.h>
#include <main.h>
#include <codec.h>
#include <codec.c>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_rcc.h>

int tab1[20000]; //bufor dzwiêku
    int petla;
    int wym_tab;
    u32 sample_buffer;
    //Fatfs object
        FATFS FatFs;
        //File object
        FIL fil;
int main(void)
{


    char sample1[3];
    unsigned int ByteToRead = 45;
    unsigned int ByteRead, i;
    //Initialize system
    SystemInit();
    //Initialize delays
    TM_DELAY_Init();
    //Initialize LEDs
    TM_DISCO_LedInit();
    int s;
    //int tab[2000]={0};
    FRESULT fresult;

    //u16 probka;
    WORD ile_bajtow;

    	u8 temp[44];		//pomijane dane z pliku .Wav
    	u16 rozmiar_probki,probka;
    	u32 rozmiar_danych,czestotliwosc;
    	int j,ile;//,i;;
    	char liczba[5];



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

    //Mount drive
    fresult = (f_mount(&FatFs, "", 1));
        //Mounted OK, turn on RED LED
        TM_DISCO_LedOn(LED_RED);
        //Try to open file
        if (f_open(&fil, "0.wav", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK)
        {
            //File opened, turn off RED and turn on GREEN led
            TM_DISCO_LedOn(LED_GREEN);
            TM_DISCO_LedOff(LED_RED);
-            f_read(&fil, &temp[0], 32, &ile_bajtow);
            f_read(&fil, &rozmiar_probki, 2, &ile_bajtow);
            f_read(&fil, &temp[0], 6, &ile_bajtow);
            f_read(&fil, &rozmiar_danych, 4, &ile_bajtow);
            ile=rozmiar_danych/rozmiar_probki;
            //for(i=0; i<(rozmiar_danych/rozmiar_probki); i++) ile++;
            	wym_tab=20000;
            	//for(i=0; i<20000; i++) wym_tab++;

            	while(ile>0)
            	{
            		if(wym_tab > ile) petla = ile; else petla = wym_tab;

            		for(i=0; i<petla; i++)
            		{
            			f_read(&fil, &probka,rozmiar_probki, &ile_bajtow);
            			tab1[i] = probka;
            		}
            		for(i=0; i<petla; i++)
            		{
            			wyslij_dzwiek(tab1[i]);
            		}
            		ile = ile-petla;
            	}
//            	if (SPI_I2S_GetFlagStatus(CODEC_I2S, SPI_I2S_FLAG_TXE))
//            	{
//                   for(i=0; i<45; i++)
//                   {
//                         SPI_I2S_SendData(CODEC_I2S, sample);
//                         sample = tab[i];
//                   }
//            	}

            }
//        ile=rozmiar_danych/rozmiar_probki;
//
//                    	wym_tab=20000;
//
//                    	while(ile>0)
//                    	{
//                    		if(wym_tab > ile) petla = ile; else petla = wym_tab;
//
//                    		for(i=0; i<petla; i++)
//                    		{
//                    			f_read(&fil, &probka,rozmiar_probki, &ile_bajtow);
//                    			tab1[i] = probka;
//                    		}
//                    		for(i=0;i<petla;i++)
//                    				{
//                    					wyslij_dzwiek(tab1[i]);
//                    					wyslij_dzwiek(tab1[i]);
//                    				}
//                    		ile = ile-petla;
//                    	}
        f_close(&fil);
        f_mount(0, "", 1);
    }

void wyslij_dzwiek(int dzwiek)
{
	int b;
	b=1;

	do
	{
		if (SPI_I2S_GetFlagStatus(CODEC_I2S, SPI_I2S_FLAG_TXE))
		{
			SPI_I2S_SendData(CODEC_I2S,dzwiek);
			b=0;
		}
	}while(b);
}


void MY_DMA_initM2P()
{
	DMA_InitTypeDef  DMA_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Stream5);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;// wybor kanalu DMA
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;// ustalenie rodzaju transferu (memory2memory / peripheral2memory /memory2peripheral)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// tryb pracy - pojedynczy transfer badz powtarzany
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;// ustalenie priorytetu danego kanalu DMA
	DMA_InitStructure.DMA_BufferSize = 512;// liczba danych do przeslania
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&sample_buffer;// adres zrodlowy
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI3->DR));// adres docelowy
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;// zezwolenie na inkrementacje adresu po kazdej przeslanej paczce danych
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;// ustalenie rozmiaru przesylanych danych
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;// ustalenie trybu pracy - jednorazwe przeslanie danych
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;// wylaczenie kolejki FIFO (nie uzywana w tym przykadzie
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;

	DMA_Init(DMA1_Stream5, &DMA_InitStructure);// zapisanie wypelnionej struktury do rejestrow wybranego polaczenia DMA
	DMA_Cmd(DMA1_Stream5, ENABLE);// uruchomienie odpowiedniego polaczenia DMA

	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE);
	SPI_Cmd(SPI3,ENABLE);
}




bool read_and_send(int position, volatile ITStatus it_status, UINT read_bytes, uint32_t DMA_FLAG)
{
	it_status = RESET;
	while(it_status == RESET)
	{
		it_status = DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG);
	}
	f_read(&fil,&sample_buffer,256*4,&read_bytes);
	DMA_ClearFlag(DMA1_Stream5, DMA_FLAG);

	if(read_bytes<256*41)
	{
		return 0;
	}
	return 1;
}
void play_wav(FRESULT fresult)
{
	// byla lista xd pomocniczo, by nie dzialac na oryginale
	UINT read_bytes;// uzyta w f_read
	f_open( &fil, "0.wav", FA_READ );
		f_lseek(&fil,44);// pominiecie 44 B naglowka pliku .wav
		volatile ITStatus it_status;// sprawdza flage DMA

		while(1)
		{
			if (read_and_send(0, it_status, read_bytes, DMA_FLAG_HTIF5)==0)
			{
				break;
			}
			if (read_and_send(256, it_status, read_bytes, DMA_FLAG_TCIF5)==0)
			{
				break;
			}
		}
		f_close(&fil);
}
