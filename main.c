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

int main(void)
{
    //Fatfs object
    FATFS FatFs;
    //File object
    FIL fil;
    //Free and total space
    uint32_t total, free;
    char buff[30];
    unsigned int ByteToRead = sizeof(buff);
    unsigned int ByteRead;
    //Initialize system
    SystemInit();
    //Initialize delays
    TM_DELAY_Init();
    //Initialize LEDs
    TM_DISCO_LedInit();
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

    //Mount drive
    if (f_mount(&FatFs, "", 1) == FR_OK)
    {
        //Mounted OK, turn on RED LED
        TM_DISCO_LedOn(LED_RED);
        //Try to open file
        if (f_open(&fil, "1stfile.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK)
        {
            //File opened, turn off RED and turn on GREEN led
            TM_DISCO_LedOn(LED_GREEN);
            TM_DISCO_LedOff(LED_RED);
            f_read(&fil,buff,ByteToRead,&ByteRead);

            f_close(&fil);
        }
        f_mount(0, "", 1);
    }

    while(1)
        {

        	if (SPI_I2S_GetFlagStatus(CODEC_I2S, SPI_I2S_FLAG_TXE))
        	{
        		SPI_I2S_SendData(CODEC_I2S, sample);

        		//only update on every second sample to insure that L & R ch. have the same sample value
        		if (sampleCounter & 0x00000001)
        		{
        			sawWave += NOTEFREQUENCY;
        			if (sawWave > 1.0)
        				sawWave -= 2.0;

        			filteredSaw = updateFilter(&filt, sawWave);

        			sample = (int16_t)(NOTEAMPLITUDE*filteredSaw);
        		}
        		sampleCounter++;
        	}

        	if (sampleCounter==48000)
        	{
        		LED_BLUE_OFF;

        	}
        	else if (sampleCounter == 96000)
        	{
        		LED_BLUE_ON;
        		sampleCounter = 0;
        	}

        }
    }

    // a very crude FIR lowpass filter
float updateFilter(fir_8* filt, float val)
{
    uint16_t valIndex=0;
    uint16_t paramIndex;
    float outval = 0.0;

    valIndex = filt->currIndex;
    filt->tabs[valIndex] = val;

    for (paramIndex=0; paramIndex<10; paramIndex++)
    {
    	outval += (filt->params[paramIndex]) * (filt->tabs[(valIndex+paramIndex)&0x07]); //??
    }

    valIndex++;
    //valIndex &= 0x07; ??

    filt->currIndex = valIndex;

    return outval;
}

void initFilter(fir_8* theFilter)
{
    uint8_t i,k=0.1;

    theFilter->currIndex = 0;

    for (i=0; i<10; i++)
    	theFilter->tabs[i] += k;

}
