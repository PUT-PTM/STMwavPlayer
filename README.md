 <center> <h1> <b><i> # STMwavPlayer </i></b> </h1> </center>
  <h3> Overview </h3>
   #STMwavPalyer plays songs from SD card using a microcontroler (STM32F407VG).
  <h3> Description </h3>
  <h3> Tools </h3>
   CooCox CoIDE 1.7.8
  <h3> What is needed </h3>
   To run the project correctly you have to have: <br> 
   &nbsp;&nbsp;&nbsp; &#9830; Microcontroler STM32F4xxx-Discovery <br>
   &nbsp;&nbsp;&nbsp; &#9830; microSD Adapter with microSD card <br>
   &nbsp;&nbsp;&nbsp; &#9830; Potentiometer <br>
   &nbsp;&nbsp;&nbsp; &#9830; a few cables <br>
  <h3> How to run </h3>
   Connect STM board with microSD Adapter by using cables: <br>
   &nbsp;&nbsp;&nbsp; &#9830; STM32 <---> SD Card Module <br>
   &nbsp;&nbsp;&nbsp; &#9830; GND <---> GND <br>
   &nbsp;&nbsp;&nbsp; &#9830; 3V <---> 3V3 <br>
   &nbsp;&nbsp;&nbsp; &#9830; PB11 <---> CS <br>
   &nbsp;&nbsp;&nbsp; &#9830; PB13 <---> SCK <br>
   &nbsp;&nbsp;&nbsp; &#9830; PB14 <---> MISO <br>
   &nbsp;&nbsp;&nbsp; &#9830; PB15 <---> MOSI <br>
   &nbsp;&nbsp;&nbsp; &#9830; GND <---> GND <br>
   Connect potentiometer to STM board (GND,PA1,VDD) <br>
   Upload WAV files on microSD card <br>
   Plug microSD card into microSD Adapter <br>
  <h3> How to compile </h3>
   Just download the project from github.com/PUT-PTM/STMwavPlayer ,compile it with Coocox CoIDE 1.7.8 and download it to flash <br>
  <h3> Attributions </h3>
   &#9830; http://elm-chan.org/fsw/ff/00index_e.html FatFs R0.12 29 april 2016 <br>
   &#9830; http://stm32f4-discovery.net/2014/07/library-21-read-sd-card-fatfs-stm32f4xx-devices/ <br>
   &#9830; https://www.youtube.com/watch?v=EYs3f4uwYTo <br>
   &#9830; http://www.mind-dump.net/configuring-the-stm32f4-discovery-for-audio <br>
   &#9830; http://forbot.pl/blog/artykuly/programowanie/kurs-stm32-6-pomiar-napiecia-przetwornik-adc-id8462 <br>
   
   
  <h3> License </h3>
  <h3> Credits </h3>
   Marcin Maćkowiak <br>
   Kamil Szulc <br> <br>
   The project was conducted during the Microprocessor Lab course held by the Institute of Control and Information Engineering, Poznan University of Technology.
  <br>
  <br>
  Supervisor: Tomasz Mańkowski
 
 
 
