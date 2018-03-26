
#include "arduinoFFT.h"          // Standard Arduino FFT library
arduinoFFT FFT = arduinoFFT();
#include <M5Stack.h>

#define SCALE 512
#define SAMPLES 1024              // Must be a power of 2
#define SAMPLING_FREQUENCY 20000
//// Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.

struct eqBand {
  const char *freqname;
  uint16_t amplitude;
  int peak;
  int lastpeak;
  uint16_t lastval;
  unsigned long lastmeasured;
};


eqBand audiospectrum[8] = {
  //Adjust the amplitude values to fit your microphone
  // Lower values increase sensetivity to the freq
  { "63Hz", 50, 0, 0, 0, 0},
  { "160Hz", 40, 0, 0, 0, 0},
  { "400Hz", 40, 0, 0, 0, 0},
  { "1KHz",  40, 0, 0, 0, 0},
  { "2.5KHz",  25, 0, 0, 0, 0},
  { "6.2KHz",  25, 0, 0, 0, 0},
  { "10KHz",  25, 0, 0, 0, 0},
  { "20KHz", 25,  0, 0, 0, 0}
};

unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;
uint16_t tft_width  = 320; // ILI9341_TFTWIDTH;
uint16_t tft_height = 240; // ILI9341_TFTHEIGHT;
uint8_t bands = 8;
uint8_t bands_width = floor( tft_width / bands );
uint8_t bands_pad = bands_width - 10;
uint16_t colormap[255]; // color palette for the band meter (pre-fill in setup)

void setup() {
  Serial.begin(115200);
  M5.begin();
  dacWrite(25, 0); // Speaker OFF
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(BLUE, BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setRotation(0);

// ADC setup// ADC Setup
  pinMode(35, INPUT);
  analogReadResolution(9);
  analogSetWidth(9);
  analogSetCycles(4);
  analogSetSamples(4);
  analogSetClockDiv(1);
  analogSetAttenuation(ADC_0db);
  adcAttachPin(35);





  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  //delay(50);
  for(uint8_t i=0;i<tft_height;i++) {
    colormap[i] = M5.Lcd.color565(tft_height-i, tft_height+i, i);
    //colormap[i] = M5.Lcd.color565(tft_height-i*.5, i*1.1, 0);
  }
  for (byte band = 0; band <= 7; band++) {
    M5.Lcd.setCursor(bands_width*band + 2, 0);
    M5.Lcd.print(audiospectrum[band].freqname);
  }
}


void displayBand(int band, int dsize){
  uint16_t hpos = bands_width*band;
  int dmax = 200;
  if(dsize>tft_height-10) {
    dsize = tft_height-10; // leave some hspace for text
  }
  if(dsize < audiospectrum[band].lastval) {
    // lower value, delete some lines
    M5.Lcd.fillRect(hpos, tft_height-audiospectrum[band].lastval,
                    bands_pad, audiospectrum[band].lastval - dsize, BLACK);
  }
  if (dsize > dmax) dsize = dmax;
  for (int s = 0; s <= dsize; s=s+4){
    M5.Lcd.drawFastHLine(hpos, tft_height-s, bands_pad, colormap[tft_height-s]);
  }
  if (dsize > audiospectrum[band].peak) {
    audiospectrum[band].peak = dsize;
  }
  audiospectrum[band].lastval = dsize;
  audiospectrum[band].lastmeasured = millis();
}


byte getBand(int i) {
  //Serial.println(i);
  if (i<=4 )             return 0;  // 125Hz
  if (i >6   && i<=10 )   return 1;  // 250Hz
  if (i >10   && i<=14 )   return 2;  // 500Hz
  if (i >14   && i<=30 )  return 3;  // 1000Hz
  if (i >30  && i<=60 )  return 4;  // 2000Hz
  if (i >60  && i<=106 )  return 5;  // 4000Hz
  if (i >106  && i<=400 ) return 6;  // 8000Hz
  if (i >400           ) return 7;  // 16000Hz
  return 8;
}

/*
if (i<=2 )             return 0;  // 125Hz
if (i >3   && i<=5 )   return 1;  // 250Hz
if (i >5   && i<=7 )   return 2;  // 500Hz
if (i >7   && i<=15 )  return 3;  // 1000Hz
if (i >15  && i<=30 )  return 4;  // 2000Hz
if (i >30  && i<=53 )  return 5;  // 4000Hz
if (i >53  && i<=200 ) return 6;  // 8000Hz
if (i >200           ) return 7;  // 16000Hz
return 8;
*/


void loop() {
  for (int i = 0; i < SAMPLES; i++) {
    adcStart(35);
    newTime = micros()-oldTime;
    oldTime = newTime;
    vReal[i] = adcEnd(35);
    //Serial.println(vReal[i]);
    //vReal[i] = analogRead(35); // A conversion takes about 1uS on an ESP32
    vImag[i] = 0;
    //while (micros() < (newTime + sampling_period_us)) {
      // do nothing to wait
    //}
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  for (int i = 2; i < (SAMPLES/2); i++){

    // Each array eleement represents a frequency and its value the amplitude.
    if (vReal[i] > 500) { // Add a crude noise filter, 10 x amplitude or more
      byte bandNum = getBand(i);
      if(bandNum!=8) {
        displayBand(bandNum, (int)vReal[i]/audiospectrum[bandNum].amplitude);
      }
    }
  }

  long vnow = millis();
  for (byte band = 0; band <= 7; band++) {
    // auto decay every 50ms on low activity bands
    if(vnow - audiospectrum[band].lastmeasured > 135) {
      displayBand(band, audiospectrum[band].lastval>4 ? audiospectrum[band].lastval-4 : 0);
    }
    if (audiospectrum[band].peak > 0) {
      audiospectrum[band].peak -= 2;
      if(audiospectrum[band].peak<=0) {
        audiospectrum[band].peak = 0;
      }
    }
    // only draw if peak changed
    if(audiospectrum[band].lastpeak != audiospectrum[band].peak) {
      // delete last peak
     M5.Lcd.drawFastHLine(bands_width*band,tft_height-audiospectrum[band].lastpeak,bands_pad,BLACK);
     audiospectrum[band].lastpeak = audiospectrum[band].peak;
     M5.Lcd.drawFastHLine(bands_width*band, tft_height-audiospectrum[band].peak,
                           bands_pad, colormap[tft_height-audiospectrum[band].peak]);
    }
  }
}
