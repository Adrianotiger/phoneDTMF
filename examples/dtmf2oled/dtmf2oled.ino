/*
  Example, using the PhoneDTMF library.
  https://github.com/Adrianotiger/phoneDTMF
  
  This example will measure the DTMF once per second.
  The result will be printed on a display (128x64) as spectrum with some debug information:
  - MCU MAX FREQ: is the maximal frequence your microcontroller can reach (but maybe the ADC frequence could be less than this value)
  - MEASURED FREQ: is the frequence measured to read the samples (should be equal or less than MAX_FREQ=6000)
  - BUTTON PRESSED: the detected phone button
  - TONES: the tones as HEX number from the last measurement
  - MAGNIUTUDE: base magnitude on startup, when no signal is detected
  - MAX_MAGNITUDE: maximal magnitude on the last measurement
  - SPECTRUM: 8 frequences with their magnitude (magnitude scale will change on each measurement)
  - ADC MEASURED: ADC center measured on startup (should be neutral, without any signal)
  - MAX_MAGNITUDE: same as described before 
*/
#include <PhoneDTMF.h>

 // oled forward declaration
void oled_setup();
void oled_updateScreen(uint16_t audioCenter, uint16_t freq, uint16_t realFreq, uint8_t tones, char button, float magnitude, float *magnitudes);

 // New dtmf, with 128 samples
PhoneDTMF dtmf = PhoneDTMF(128);

#if defined(GPIO_NUM_36)
  uint8_t analogPin = GPIO_NUM_36; // ESP32 ADC1, CHANNEL_0 = GPIO36 on Wroom32
#else
  uint8_t analogPin = A0;
#endif

// OLED DISPLAY:
//  ***************************************************
//  * [MCU MAX FREQ]  [BUTTON PRESSED] [MAGNITUDE]    *
//  * [MEASURED FREQ] [TONES (HEX)]    [MAX MAGNITUDE]*
//  *                                 |               *
//  *               | [SPRECTRUM]     |               *
//  *   |           |                 |           |   *
//  *   |     |     |     |     |     |     |     |   *
//  * ----------------------------------------------- *
//  * [ADC MEASURED]       [MAX MAGNITUDE]            *
//  ***************************************************

void setup()
{
  Serial.begin(115200);
  
  oled_setup();
  Serial.println("OLED started");
  
    // try to increase resolution to 12bit (0-4096)
    // un-comment this line if you are on an Arduino Due or ESP or Teensy
  //analogReadResolution(12);
    
    // wait until ADC is ready
  delay(1000);
  
  dtmf.begin(analogPin);
}

void loop()
{
  uint8_t   tones;
  char      button;
  float     afMag[8];
  
    // measure dtmf and save results to afMag. -1 means Automagnitude (do not set it manually).
  tones = dtmf.detect(afMag, -1.0f);
  
    // get the pressed button or 0 if an invalid button was detected
  button = dtmf.tone2char(tones);
  if (button > 0)
	{
	  Serial.print("BUTTON PRESSED: "); Serial.println(button);
  }
   // update oled display
  oled_updateScreen(
    dtmf.getAnalogCenter(), 
    dtmf.getSampleFrequence(), 
    dtmf.getRealFrequence(), 
    tones, 
    button > 0 ? button : '-', 
    dtmf.getBaseMagnitude(), 
    afMag
  );
    
  delay(1000);
}
