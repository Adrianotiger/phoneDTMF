#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void oled_setup()
{
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setContrast(125);
}

void oled_updateScreen(uint16_t audioCenter, uint16_t freq, uint16_t realFreq, uint8_t tones, char button, float magnitude, float *magnitudes)
{
  float maxM = magnitude * 1.1f;
  float bigM = 0;
  int16_t len;
  for(uint8_t j=0;j<8;j++)
  {
    if (maxM < magnitudes[j]) maxM = magnitudes[j];
	  if (bigM < magnitudes[j]) bigM = magnitudes[j];
  }
    
  u8g2.setFont(u8g2_font_baby_tr);
  
  u8g2.firstPage();
  do {	
    u8g2.drawHLine(0, 54, 128);
    for(uint8_t j=0;j<8;j++)
    {
      if(maxM < 2.0f)
        len = 2;
      else 
  	    len = (int)(magnitudes[j] * 38.0f / maxM);
      u8g2.drawVLine(8 + j * 16, 16 + (38 - len), len);
    }
    
    u8g2.drawStr(1, 8, (String(freq) + String("Hz")).c_str());
    u8g2.drawStr(1, 16, (String(realFreq) + String("Hz")).c_str());
    
    u8g2.drawStr(40, 16, (String("Tns: ") + String(tones, HEX)).c_str());
    u8g2.drawStr(40, 8, (String("Btn: ") + String(button)).c_str());
    
    u8g2.drawStr(76, 8, (String("Mag: ") + String((int)magnitude)).c_str());
    u8g2.drawStr(76, 16, (String("Max: ") + String((int)maxM)).c_str());
  
  	u8g2.drawStr(1, 64, (String("ADC: ") + String(audioCenter)).c_str());
  	u8g2.drawStr(64, 64, (String("MAG: ") + String((int)bigM)).c_str());
    
  } while ( u8g2.nextPage() );
}