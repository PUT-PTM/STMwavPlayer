/*	Features:
 	 - FatFS by Chan supported, version RO. 11
 	 "tm_stm32f4_delay.h"
 	 "tm_stm32f4_disco.h"	Read SD card with FatFs by Tilen Majerle
 	 "tm_stm32f4_fatfs.h"

 	 "codec.h" and "codec.c"
*/

#include "defines.h"
#include "stm32f4xx.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_disco.h"
#include "tm_stm32f4_fatfs.h"
#include <stdio.h>
#include <string.h>
#include <main.h>
#include <codec.h>
#include <codec.c>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_rcc.h>

int main(void)
{

	//Fatfs object
    FATFS FatFs;
    //File object
    FIL fil;
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
    int tab[45]={0};
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
    if (f_mount(&FatFs, "", 1) == FR_OK)
    {
        //Mounted OK, turn on RED LED
        TM_DISCO_LedOn(LED_RED);
        //Try to open file
        if (f_open(&fil, "file.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK)
        {
            //File opened, turn off RED and turn on GREEN led
            TM_DISCO_LedOn(LED_GREEN);
            TM_DISCO_LedOff(LED_RED);
            while(1)
            {
            	for(i=0; i<ByteToRead; i++)
            	{
            		f_read(&fil,sample1,3,&ByteRead);
            		s = atoi(sample1);
            		tab[i] = s;
            	}
            	if (SPI_I2S_GetFlagStatus(CODEC_I2S, SPI_I2S_FLAG_TXE))
            	{
                   for(i=0; i<45; i++)
                   {
                         SPI_I2S_SendData(CODEC_I2S, sample);
                         sample = tab[i];
                   }
            	}
            }
            }
            f_close(&fil);

        }
        f_mount(0, "", 1);
    }

