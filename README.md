# PhoneDTMF
**Arduino library to detect DTMF without any external devices.**
When you use a MCU to decode a DTMF, you don't need an external device like the MT8870. You just need the ADC input and you will be able to detect the tones with simple algorithms (see Goertzel Algorithm)

## Use library
~~~~cpp
#include <PhoneDTMF.h>
PhoneDTMF dtmf = PhoneDTMF();
void setup() {
	Serial.begin(9600);
	dtmf.begin(A0);
}
void loop() {
	uint8_t tones = dtmf.detect();
    char button = dtmf.tone2char(tones);
    if(button > 0) {
    	Serial.print(button); Serial.println(" pressed");
    }
    delay(1000);
}
~~~~

## Circuit
Just remember that you have to filter out the DC compoment and that the signal peaks should stay between 0V and VCC. You can break the ADC input of your microcontroller.
So, to remove the DC voltage, add a capacitor (2.2uF) before you connect the line to your input.
2 Zener  diodes and an opamp would are also good components to protect it. But if you are sure to have a right signal, you can measure directly on the bus.
If you want to see some circuits, for example how to connect your Arduino directly to your analog telephone, check the <a href='circuits/'>**Circuits manual**</a>.


## Goertzel Algorithm
The Goertzel algorithm is a technique in digital signal processing (DSP) for efficient evaluation of the individual terms of the discrete Fourier transform (DFT).
https://en.wikipedia.org/wiki/Goertzel_algorithm

Since the DTMF frequences are well known and are only 8 (for the phone), the algorithm can be shortened a little bit. This is the case in this library.

## Links
Original code from this library: https://forum.arduino.cc/index.php?topic=121540.0
DTMF on Wikipedia: https://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling
Nice documentation about DTMF: https://www.mdpi.com/2076-3417/9/3/422/pdf
Frequence filtering: http://dataandsignal.com/frequency-detection/


## Credits
This library is based on the code written by Jacob Rosenthal (June, 2012) that is based on the Kevin Banks code found at http://www.eetimes.com/design/embedded/4024443/The-Goertzel-Algorithm

It was then modified by by El_Supremo and optimized for DTMF and Arduino UNO (December, 2016), see https://forum.arduino.cc/index.php?topic=121540.0

Now, this library was modified to work with other processors too and with the possibility to detect the tones without find out the magnitude.