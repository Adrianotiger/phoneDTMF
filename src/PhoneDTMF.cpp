/*
Based on: https://github.com/jacobrosenthal/Goertzel
Heavily modified by Pete (El_Supremo on Arduino forums) to decode DTMF tones
It is also public domain and provided on an AS-IS basis. There's no warranty
or guarantee of ANY kind whatsoever.
This uses Digital Pin 4 to allow measurement of the sampling frequency

The Goertzel algorithm is long standing so see
http://en.wikipedia.org/wiki/Goertzel_algorithm for a full description.
It is often used in DTMF tone detection as an alternative to the Fast
Fourier Transform because it is quick with low overheard because it
is only searching for a single frequency rather than showing the
occurrence of all frequencies.
This work is entirely based on the Kevin Banks code found at
http://www.eetimes.com/design/embedded/4024443/The-Goertzel-Algorithm
so full credit to him for his generic implementation and breakdown. I've
simply massaged it into an Arduino library. I recommend reading his article
for a full description of whats going on behind the scenes.

Created by Jacob Rosenthal, June 20, 2012.
Released into the public domain.

*/

// include this library's description file
#include "DTMF.h"

#define MAX_FREQ 6000.0f

  /// <summary>Constructor, you can set 2 params, if you don't like the standard behaviour</summary>
  /// <param name="sampleCount">number of measurements, default is 128 (with a frequence of 6000Hz it means 128 * 1/6000 = 21.3ms for each measurement). Normal values are from 50 to 200.</param>
  /// <param name="amplifier">use amplifier if your signal does cover only 1/5 or 1/10 of the maximal amplitude (an opamp would be better)</param>
PhoneDTMF::PhoneDTMF(int16_t sampleCount, float amplifier)
{               
  _iSamplesCount = sampleCount; 
  _fAmplifier = amplifier;
  _iCompensation = 0;
  _fBaseMagnitude = 0.0f;
  _iSamplesFrequence = MAX_FREQ;
}

  /// <summary>Init the module (calculate frequence, ADC center, coefficients)</summary>
  /// <remarks>even if the frequence could be 25-50kHz, many microcontroller have a limited ADC reading frequence. 
  /// For example the ESP32 is able to execute readAnalogInput 25.000 times each second but internally it read the inputs only 6000 times.</remarks>
  /// <param name="sensorPin">the pin used to detect the DTMF</param>
  /// <returns>maximal frequence available on this processor</returns>
uint16_t PhoneDTMF::begin(uint8_t sensorPin)
{
  _Pin = sensorPin;
  pinMode(_Pin, INPUT);
  delay(10);
  float magnitudes[TONES];
  uint32_t t1 = millis();
  for(uint16_t j=0;j<1000;j++)
  {
    singleDetect();
  }
  t1 = millis() - t1;
  float sampleFrequence = (1000.0f / t1) * 1000.0f; 
  //sampleFrequence *= 0.5;
  if (sampleFrequence > MAX_FREQ) _iCompensation = ((1000.0f / MAX_FREQ) - (1000.0f / sampleFrequence)) * 1000.0f;
  
  _iAdcCentre = analogRead(_Pin);
  _iSamplesFrequence = (uint16_t)sampleFrequence;

  calculateMeasurement(magnitudes);
  for (uint8_t j = 0; j < TONES; j++)
	  _fBaseMagnitude += magnitudes[j];
  _fBaseMagnitude /= TONES;
  
  float	omega;
  // Calculate the coefficient for each DTMF tone
  for(uint8_t i=0; i<TONES; i++) 
  {
    omega = (2.0f * PI * DTMF_TONES[i]) / MAX_FREQ;
    
    // DTMF detection doesn't need the phase.
    // Computation of the magnitudes (which DTMF does need) does not
    // require the value of the sin.
    // not needed    sine = sin(omega);
	_afToneCoeff[i] = 2.0f * cos(omega);
  }
  ResetDTMF();

  return _iSamplesFrequence;
}

  /// <summary>Detect the frequences</summary>
  /// <param name="pMagnitudes">a pointer to an array of 8 floats. It will return the values of all frequences. NULL or leave empty if you don't need the values.</param>
  /// <param name="magnitude">set the magnitude to detect the tones. &lt;0 or leave empty if it should detect them automatically</param>
  /// <returns>tones detected as 8-bit number (1000 0010 means that the second and last frequence where detected)</returns>
uint8_t PhoneDTMF::detect(float* pMagnitudes, float magnitude)
{
  uint32_t fTemp = millis();
  for (uint16_t index=0; index< _iSamplesCount; index++)
  {
    singleDetect();
    delayMicroseconds(_iCompensation);
  }
  fTemp = millis() - fTemp;
  _iRealFrequence = 1.0f / ((fTemp / 1000.0f) / _iSamplesCount);
  if (_iRealFrequence > MAX_FREQ + 50 && _iCompensation < 200) _iCompensation++;
  if (_iRealFrequence < MAX_FREQ - 150 && _iCompensation > 100) _iCompensation--;
  return calculateMeasurement(pMagnitudes, magnitude);
}

  /// <summary>utility function to retrieve the pressed button with the current dtmf. See <see cref="detect(float* pMagnitudes, float magnitude)"/> to detect dtmf tones.</summary>
  /// <param name="dtmf">detected tones (8 bit flags, 0=not detected and 1=detected)</param>
  /// <returns>character of the button pressed (example '1' or '#')</returns>
char PhoneDTMF::tone2char(uint8_t dtmf)
{
  for(uint8_t j=0; j<TONES*2; j++) 
  {
    if(dtmf == DTMF_MAP[j]) return(DTMF_CHAR[j]);
  }
  return((char) 0);
}

  /// <summary>returns the maximal microcontroller sample frequence</summary>
uint32_t PhoneDTMF::getSampleFrequence()
{
	return _iSamplesFrequence;
}

  /// <summary>returns the analog middle when nothing is detected (used to detect tones)</summary>
uint16_t PhoneDTMF::getAnalogCenter()
{
	return _iAdcCentre;
}

  /// <summary>returns the real frequence by measuring the tones (should be MAX_FREQ=6000)</summary>
uint32_t PhoneDTMF::getRealFrequence()
{
	return _iRealFrequence;
}

  /// <summary>returns magnitude when nothing is detected. Used to detect the tones automatically</summary>
uint16_t PhoneDTMF::getBaseMagnitude()
{
	return (uint16_t)_fBaseMagnitude;
}

  /// <summary>returns the time for a single measurement</summary>
uint16_t PhoneDTMF::getMeasurementTime()
{
	return (uint16_t)((1000.0f / _iRealFrequence) * _iSamplesCount);
}

/*
  Private functions:
*/

uint8_t PhoneDTMF::calculateMeasurement(float* pRet, float magnitude)
{
  float dtmf_mag[TONES];
  float maxMag = _fBaseMagnitude * 3.0f;
  for (uint8_t i = 0; i < TONES; i++)
  {
    dtmf_mag[i] = sqrt(_afQ1[i] * _afQ1[i] + _afQ2[i] * _afQ2[i] - _afToneCoeff[i] * _afQ1[i] * _afQ2[i]);
    if (maxMag < dtmf_mag[i]) maxMag = dtmf_mag[i];
  }
  if (pRet != NULL)
  {
    memcpy(pRet, dtmf_mag, sizeof(dtmf_mag));
  }
  ResetDTMF();

  uint8_t dtmf = 0;
  if (magnitude < 0.0f) magnitude = maxMag - (maxMag - _fBaseMagnitude) * 0.3f;

  for (uint8_t i = 0; i < TONES; i++)
  {
    if (dtmf_mag[i] > magnitude)
    {
      dtmf |= (0x01 << i);
    }
  }

  return dtmf;
}

void PhoneDTMF::singleDetect()
{
  ProcessSample(analogRead(_Pin));
}


/* Call this routine for every sample. */
//El_Supremo - change to int (WHY was it byte??)
  /// <summary>
void PhoneDTMF::ProcessSample(int16_t sample)
{
  float Q0;
  //EL_Supremo subtract adc_centre to offset the sample correctly
  for (uint8_t i = 0; i < TONES; i++)
  {
    Q0 = _afToneCoeff[i] * _afQ1[i] - _afQ2[i] + (float)(sample - _iAdcCentre) * _fAmplifier;
    _afQ2[i] = _afQ1[i];
    _afQ1[i] = Q0;
  }
}

/* Call this routine before every "block" (size=N) of samples. */
void PhoneDTMF::ResetDTMF(void)
{
  for (uint8_t i = 0; i < TONES; i++)
  {
    _afQ2[i] = 0;
    _afQ1[i] = 0;
  }
}