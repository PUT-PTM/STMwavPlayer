#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "delay.h"
#include "codec.h"
#include "ff.h"
#include <stdbool.h>


FATFS fatfs;
FIL file;
u16 DMA_buffer[2048];
const char* songs[4] = {"Beat.wav","Song.wav","wav1.wav","wav2.wav"};
int i=0;
int result_of_conversion=0;
volatile s8 swich=0, change=0;





int main( void )
{
    SystemInit();
    SD_init();// init SPI pod SD
    FRESULT fresult;
    disk_initialize(0);// inicjalizacja karty
    fresult = f_mount( &fatfs, 1,1 );// zarejestrowanie dysku logicznego w systemie

    codec_init();
    codec_ctrl_init();

    I2S_Cmd(CODEC_I2S, ENABLE);// Integrated Interchip Sound to connect digital devices
    dma_init();

    przycisk_start();
    ADC_init();
    //WAV_play(songs[i]);
    for(;;)
    {
        WAV_play(songs[i]);
        if(change == 1)
            i++;
        if(i == 4) i =0;
    }
    for(;;)
    {

    }

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
    change=0; // zmienna do zmiany utworow

    res = f_open(&file, FileName, FA_READ); // otworzenie pliku
    f_lseek(&file,44); // ominiecie pierwszych 44 bajtow pliku WAV
    f_read(&file, &DMA_buffer[0], 2048, &cnt); // przeczytanie jednej probki (2B)
    volatile ITStatus it_st; // sprawdzenie flagi DMA

    while(1)
    {
        it_st = RESET;
        while(it_st == RESET)
        {
            it_st = DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG_HTIF5); // pobieramy flage DMA HTIF ze streamu 5 (w polowie)
        }
        f_read(&file, &DMA_buffer[0], 2048, &cnt); // czytaj kolejna probke (2B)
        DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_HTIF5); //
        if(cnt<1024)break; // jezeli koniec utworu to wyjdz

        it_st = RESET;
        while(it_st == RESET)
        {
             it_st = DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG_TCIF5);
        }
        f_read (&file,&DMA_buffer[1024],2048,&cnt);
        DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5 );
        if(cnt<1024 || change == 1)break; // jezeli koniec utworu lub wcisnieto przycisk to wyjdz i zmien utwor
    }
    f_close(&file);
}

void przycisk_start(void)
{
    IniPrzycisku();
    IniTimerPrzerwanie2();

}

void IniPrzycisku(void)
{
//przycisk - PA0

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef y;
    EXTI_InitTypeDef ex;

    y.GPIO_Pin = GPIO_Pin_0;
    y.GPIO_Mode = GPIO_Mode_IN;
    y.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &y);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitTypeDef NVIC_InitStructure;
    // numer przerwania
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    // priorytet główny
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    // subpriorytet
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    // uruchom dany kanał
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // zapisz wypełnioną strukturę do rejestrów
    NVIC_Init(&NVIC_InitStructure);


    ///* Configure EXTI Line0 */
    ex.EXTI_Line=EXTI_Line0;
    ex.EXTI_Mode=EXTI_Mode_Interrupt;
    ex.EXTI_Trigger=EXTI_Trigger_Rising;
    ex.EXTI_LineCmd=ENABLE;
    EXTI_Init(&ex);

    ///* Connect EXTI Line0 to PA0 pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0) ;
}

void IniTimerPrzerwanie2()//wykorzystywany do przycisku
{
    //ustawienie trybu pracy priorytetow przerwan
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitTypeDef p;
    //numer przerwania
    p.NVIC_IRQChannel = TIM5_IRQn;
    // priorytet główny
    p.NVIC_IRQChannelPreemptionPriority = 0x00;
    // subpriorytet
    p.NVIC_IRQChannelSubPriority = 0x00;
    // uruchom dany kanał
    p.NVIC_IRQChannelCmd = ENABLE;
    // zapisz wypełnioną strukturę do rejestrów
    NVIC_Init(&p);

    //doprowadzenie sygnalu zerowego
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    TIM_TimeBaseInitTypeDef a;
    a.TIM_Prescaler=8400-1;
    a.TIM_CounterMode=TIM_CounterMode_Up;
    a.TIM_Period=3000-1;
    a.TIM_ClockDivision=TIM_CKD_DIV1;
    a.TIM_RepetitionCounter=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &a);
    TIM_Cmd(TIM5,DISABLE);


    // wyczyszczenie przerwania od timera (wystąpiło przy konfiguracji timera)
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    // zezwolenie na przerwania od przepełnienia dla timera
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    //wlaczenie timera
    //TIM_Cmd(TIM5, ENABLE);

}


void EXTI0_IRQHandler ( void )
{
    if ( EXTI_GetITStatus ( EXTI_Line0 ) != RESET ) // sprawdzenie źródł a przerwania
    {
        swich = 0;
        TIM_Cmd(TIM5, ENABLE);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    EXTI_ClearITPendingBit ( EXTI_Line0 ); //pamiętamy o zerowaniu bitu przerwania
}

void TIM5_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
{
        if(swich == 0)
        {
            change = 1; // zmiana utworu
        }
        TIM_Cmd(TIM5, DISABLE);
        TIM_SetCounter(TIM5, 0);
        // wyzerowanie flagi wyzwolonego przerwania
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    }
}

////potencjometer
void ADC_conversion()
{
    // Odczyt wartosci przez odpytnie flagi zakonczenia konwersji
    // Wielorazowe sprawdzenie wartosci wyniku konwersji
    ADC_SoftwareStartConv(ADC1);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    result_of_conversion = ((ADC_GetConversionValue(ADC1))/16);
}
void TIM2_ADC_init()
{

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIMER_2;
    TIMER_2.TIM_Period = 2100-1;// okres zliczania
    TIMER_2.TIM_Prescaler = 2000-1;// wartosc preskalera
    TIMER_2.TIM_ClockDivision = TIM_CKD_DIV1;// dzielnik zegara
    TIMER_2.TIM_CounterMode = TIM_CounterMode_Up;// kierunek zliczania
    TIM_TimeBaseInit(TIM2, &TIMER_2);
    TIM_Cmd(TIM2, ENABLE);// Uruchomienie Timera

    // KONFIGURACJA PRZERWAN - TIMER/COUNTER
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;// numer przerwania
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;// priorytet glowny
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;// subpriorytet
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;// uruchom dany kanal
    NVIC_Init(&NVIC_InitStructure);// zapisz wypelniona strukture do rejestrow
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);// wyczyszczenie przerwania od timera 2 (wystapilo przy konfiguracji timera)
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);// zezwolenie na przerwania
}
void ADC_init()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);// zegar dla portu GPIO z ktorego wykorzystany zostanie pin
    // jako wejscie ADC (PA1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);// zegar dla modulu ADC1

    // inicjalizacja wejscia ADC
    GPIO_InitTypeDef  GPIO_InitStructureADC;
    GPIO_InitStructureADC.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructureADC.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructureADC.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructureADC);

    ADC_CommonInitTypeDef ADC_CommonInitStructure;// Konfiguracja dla wszystkich ukladow ADC
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;// niezalezny tryb pracy przetwornikow
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;// zegar glowny podzielony przez 2
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;// opcja istotna tylko dla tryby multi ADC
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;// czas przerwy pomiedzy kolejnymi konwersjami
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_InitTypeDef ADC_InitStructure;// Konfiguracja danego przetwornika
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;// ustawienie rozdzielczosci przetwornika na maksymalna (12 bitow)
    // wylaczenie trybu skanowania (odczytywac bedziemy jedno wejscie ADC
    // w trybie skanowania automatycznie wykonywana jest konwersja na wielu
    // wejsciach/kanalach)
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;// wlaczenie ciaglego trybu pracy wylaczenie zewnetrznego wyzwalania
    // konwersja moze byc wyzwalana timerem, stanem wejscia itd. (szczegoly w dokumentacji)
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;// liczba konwersji rowna 1, bo 1 kanal
    ADC_Init(ADC1, &ADC_InitStructure);// zapisz wypelniona strukture do rejestrow przetwornika numer 1

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_84Cycles);// Konfiguracja kanalu pierwszego ADC
    ADC_Cmd(ADC1, ENABLE);// Uruchomienie przetwornika ADC

    TIM2_ADC_init();
}
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        ADC_conversion();
        Codec_VolumeCtrl(result_of_conversion);
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
