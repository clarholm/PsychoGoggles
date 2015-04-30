#define LIN_OUT 1
#define FHT_N 128 // set to 256 point fht

#include <FHT.h> // include the library
#include <Adafruit_NeoPixel.h>

#define PIN 2

//NEOPIXEL SETUP
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

//FHT setup
//-------------------------------------------------------------------------------
// Change this to MONO to for a one-channel full-display or STEREO for two-channel
//
String displayType = "STEREO";
//
// Change this to MONO to for a one-channel full-display or STEREO for two-channel
//-------------------------------------------------------------------------------

byte sampleR[64];
byte sampleL[64];
unsigned long startTime, endTime, oldTime;
String sampleSet;
int displaySize;
uint16_t brightness;
byte currentLmaxBin0 = 0;
byte currentRmaxBin0 = 0;
byte currentLmaxBin2 = 0;
byte currentRmaxBin2 = 0;
int GBin0=0;
int RBin0=0;
int BBin0=0;
int GBin2=0;
int RBin2=130;
int BBin2=100;

int colornumberBin0 = 0;
int colornumberBin2 = 0;
int iterationsWithColorBin0 = 0;
int iterationsWithColorBin2 = 0;
int colorBin0 = 0;
int colorBin2 = 0;
int bottomLedBrightnessScaler = 3; //this value will be read from potentiometers on the front panel
int topLedBrightnessScaler = 3;//this value will be read from potentiometers on the front panel
int Bin0currentFadeRate = 6;
int Bin2currentFadeRate = 3;
int Bin0colorChangeRate = 2;
int Bin2colorChangeRate = 2;
int Bin0Binscaler = 0;
int Bin2Binscaler = 0;
int maxBrightnessAllowed = 50;

int lowerLedBin1 = 0; //this value will be read from potentiometers on the front panel
int upperLedBin = 0; //this value will be read from potentiometers on the front panel

int topLedBinPot = A6;
int topLedBrightnessPot = A7;
int bottomLedBinPot1 = A0;
int bottomLedBrightnessPot = A5;

void setup() {
  if (displayType == "MONO") {displaySize = 32;} else {displaySize = 16;}
  //TIMSK0 = 0; // turn off timer0 for lower jitter
  pinMode(topLedBinPot, INPUT);
  pinMode(topLedBrightnessPot, INPUT);
  pinMode(bottomLedBinPot1, INPUT);
  pinMode(bottomLedBrightnessPot, INPUT);
  //ADCSRA = 0xe7; // set the adc to free running mode
  //ADCSRA = 0xC7; // turn off free running mode.
  //ADMUX = 0x45; // use adc5
  //DIDR0 = 0x20; // turn off the digital input for adc5
  Serial.begin(115200);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  //Serial.println("Setup finished");

}

void loop() {

  readPotentiometers();
  setBinBasedFadeRate();
  startTime = millis();
  sampleSet = "L";
  sampleInput();
  sampleFix();
  if (displaySize == 16) {
    sampleSet = "R";
    sampleInput();
    sampleFix();
  }
  drawSpectrum();
  endTime = millis();
}


void setBinBasedFadeRate(){
Bin0currentFadeRate = 10 - lowerLedBin1;
Bin2currentFadeRate = 10 - upperLedBin;
}


void readPotentiometers() {
  //ADCSRA = 0xf5; // restart adc   
  ADCSRA = 0xC7; // turn off free running mode.
  delay(10);
  
  //Serial.println("Enter read potentiometers.");

  
  bottomLedBrightnessScaler = analogRead(bottomLedBrightnessPot);            // reads the value of the potentiometer (value between 0 and 1023) 
  bottomLedBrightnessScaler = map(bottomLedBrightnessScaler, 1023, 0, 0, 40);     // scale it to have better values (value between 0 and 16)
  Serial.print("After map bottomLedBrightnessScaler: ");
  Serial.println(bottomLedBrightnessScaler);
  
    lowerLedBin1 = analogRead(bottomLedBinPot1);            // reads the value of the potentiometer (value between 0 and 1023)
  //Serial.print("Before map lowerLedBin: ");
  //Serial.println(lowerLedBin);
  lowerLedBin1 = map(lowerLedBin1, 1023, 0, 0, 6);     // scale it to use it to select a frequency bin (value between 0 and 16)
  Serial.print("After map lowerLedBin: ");
  Serial.println(lowerLedBin1);
  
  upperLedBin = analogRead(topLedBinPot);            // reads the value of the potentiometer (value between 0 and 1023) 
  upperLedBin = map(upperLedBin, 1023, 0, 0, 6);     // scale it to use it to select a frequency bin (value between 0 and 16)
  Serial.print("After map upperLedBin: ");
  Serial.println(upperLedBin);
  topLedBrightnessScaler = analogRead(topLedBrightnessPot);            // reads the value of the potentiometer (value between 0 and 1023) 
  topLedBrightnessScaler = map(topLedBrightnessScaler, 1023, 0, 0, 40);     // scale it to have better values (value between 0 and 16)
   Serial.print("After map topLedBrightnessScaler: ");
  Serial.println(topLedBrightnessScaler);
  //delay(1000);
  ADCSRA = 0xe7; // set the adc to free running mode
}

boolean checkCurrentMaxToLimitBrightness(int currentMaxValue){
if (currentMaxValue > maxBrightnessAllowed)
{
  return true;
}
else return false;
}

void drawSpectrum () {
  int color;
  if (displaySize == 16) {
    int disX = 0;
    for (disX; disX < 16; disX++) {
 

   // -------------------------------  Bottom Left led -------------------------------
      if (disX == lowerLedBin1 && sampleL[disX] > 3 && sampleL[disX] > currentLmaxBin0){ 
        //if the amplitude of frequency component for the left channel in (bin 0 + Bin0Scaler) is greater then 3 and greater then current max. Bin0Scaler is added to allow for easy change
       // of what frequency the Base leds are triggered by.
        
        currentLmaxBin0 = sampleL[disX];
        //set new currentLmaxBin0.
        if (checkCurrentMaxToLimitBrightness(currentLmaxBin0)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
        else brightness = currentLmaxBin0 * (bottomLedBrightnessScaler/6);
        //Scale brightness
        
        if(iterationsWithColorBin0 > 10){
        colornumberBin0 = colornumberBin0+(currentLmaxBin0/Bin0colorChangeRate);
        //Every 10th sample change the color by adding current (currentLmaxBin0/Bin0colorChangeRate) This causes the colors to change quickly if the amplitude of the frequency components are high.
        if (colornumberBin0 > 255){
          //Start over if with first color if the value is above 255.
          colornumberBin0 = 0;
        }

        WheelBin0(colornumberBin0);
        iterationsWithColorBin0 = 0;
        }
        
        //If the current color value has not been used for 10 samples only change brightness.
        iterationsWithColorBin0 = iterationsWithColorBin0 + 1;
        //Set the light up the pixel with desired brightness, if it is base or treble leds that should light up and what pixel number.
        /*
        Serial.print("Bottom Left disx: ");
        Serial.println(disX);
        Serial.print("Bottom Left new current LmaxBin0: ");
        Serial.println(currentLmaxBin0);
        Serial.print("Bottom Left new brightness");
        Serial.println(brightness);
        setPixelColorAndBrightness(brightness, 0, 3);
        */

      }
      //If no new max was detected then fade the pixels with a constant called fade rate
      if (disX == lowerLedBin1 && sampleL[disX] < currentLmaxBin0 && currentLmaxBin0-Bin0currentFadeRate > 0){
       
        currentLmaxBin0 = currentLmaxBin0 - Bin0currentFadeRate;
        if (checkCurrentMaxToLimitBrightness(currentLmaxBin0)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
        else brightness = currentLmaxBin0 * (bottomLedBrightnessScaler/6);
        setPixelColorAndBrightness(brightness, 0, 3);

      }
      //If the current Lmax goes below 0 set turn the pixel off.
        if (disX == lowerLedBin1 && sampleL[disX] < currentLmaxBin0 && currentLmaxBin0-Bin0currentFadeRate <= 0){
        currentLmaxBin0 = 0;
               if (checkCurrentMaxToLimitBrightness(currentLmaxBin0)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
       else brightness = currentLmaxBin0 * (bottomLedBrightnessScaler/6);
       setPixelColorAndBrightness(brightness, 0, 3);

      }
      
   // -------------------------------  Bottom Right led -------------------------------
      if (disX == lowerLedBin1 && sampleR[disX] > 3 && sampleR[disX] > currentRmaxBin0){
        
        currentRmaxBin0 = sampleR[disX];
        if (checkCurrentMaxToLimitBrightness(currentLmaxBin0)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
        else brightness = currentRmaxBin0 * (bottomLedBrightnessScaler/6);
        setPixelColorAndBrightness(brightness, 0, 2);
      }
      if (disX == lowerLedBin1 && sampleR[disX] < currentRmaxBin0 && currentRmaxBin0-Bin0currentFadeRate > 0){
        currentRmaxBin0 = currentRmaxBin0 - Bin0currentFadeRate;
        if (checkCurrentMaxToLimitBrightness(currentLmaxBin0)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
       
       else brightness = currentRmaxBin0 * (bottomLedBrightnessScaler/6);
       setPixelColorAndBrightness(brightness, 0, 2);
      }
       if (disX == lowerLedBin1 && sampleR[disX] < currentRmaxBin0 && currentRmaxBin0-Bin0currentFadeRate <= 0){
        currentRmaxBin0 = 0;
        if (checkCurrentMaxToLimitBrightness(currentLmaxBin0)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
       else brightness = currentRmaxBin0 * (bottomLedBrightnessScaler/6);
       setPixelColorAndBrightness(brightness, 0, 2);

      }
      
   // -------------------------------  Top Left led -------------------------------  
      if (disX == upperLedBin && sampleL[disX] > 3 && sampleL[disX] > currentLmaxBin2){
        
        currentLmaxBin2 = sampleL[disX];
         brightness = currentLmaxBin2 * (topLedBrightnessScaler/6);
        
        if(iterationsWithColorBin2 > 10){
        colornumberBin2 = colornumberBin2+(currentLmaxBin2/Bin2colorChangeRate);
        if (colornumberBin2 > 255){
          colornumberBin2 = 0;
        }

        WheelBin2(colornumberBin2);
        iterationsWithColorBin2 = 0;
        }
        iterationsWithColorBin2 = iterationsWithColorBin2 + 1;
        /*
        Serial.print("Top Left disx: ");
        Serial.println(disX);
        Serial.print("Top Left new current LmaxBin0: ");
        Serial.println(currentLmaxBin2);
        Serial.print("Top Left new brightness");
        Serial.println(brightness);
        setPixelColorAndBrightness(brightness, 2, 0);
        */
      
      }
      if (disX == upperLedBin && sampleL[disX] < currentLmaxBin2  && currentLmaxBin2-Bin2currentFadeRate > 0){
       
        currentLmaxBin2 = currentLmaxBin2 - Bin2currentFadeRate;
         if (checkCurrentMaxToLimitBrightness(currentLmaxBin2)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
       else brightness = currentLmaxBin2 * (topLedBrightnessScaler/6);
       setPixelColorAndBrightness(brightness, 2, 0);
      }
      
      if (disX == upperLedBin && sampleL[disX] < currentLmaxBin2 && currentLmaxBin2-Bin2currentFadeRate <= 0){
        currentLmaxBin2 = 0;
          if (checkCurrentMaxToLimitBrightness(currentLmaxBin2)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
       else brightness = currentLmaxBin2 * bottomLedBrightnessScaler;
       setPixelColorAndBrightness(brightness, 2, 0);
      }
      
       // -------------------------------  Top Right led -------------------------------  
       
      if (disX == upperLedBin && sampleR[disX] > 3 && sampleR[disX] > currentRmaxBin2){
        
        currentRmaxBin2 = sampleR[disX];
          if (checkCurrentMaxToLimitBrightness(currentLmaxBin2)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
        else brightness = currentRmaxBin2 * (topLedBrightnessScaler/6);
         setPixelColorAndBrightness(brightness, 2, 1);
      }
      if (disX == upperLedBin && sampleR[disX] < currentRmaxBin2  && currentRmaxBin2-Bin2currentFadeRate > 0){
       
        currentRmaxBin2 = currentRmaxBin2 - Bin2currentFadeRate;
          if (checkCurrentMaxToLimitBrightness(currentLmaxBin2)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
       else brightness =currentRmaxBin2 * (topLedBrightnessScaler/6);
       setPixelColorAndBrightness(brightness, 2, 1);
      }
      if (disX == upperLedBin && sampleR[disX] < currentRmaxBin2 && currentRmaxBin2-Bin2currentFadeRate <= 0){
        currentRmaxBin2 = 0;
          if (checkCurrentMaxToLimitBrightness(currentLmaxBin2)== true){
        brightness = maxBrightnessAllowed * (bottomLedBrightnessScaler/6);
        }
       else brightness = currentRmaxBin2 * (topLedBrightnessScaler/6);
       setPixelColorAndBrightness(brightness, 2, 1);
      }
      
         
      }
}
  
  else {
    //dmd.clearScreen(true);
    for (int disX; disX < 33; disX++) {
      //dmd.drawLine (disX, 16, disX, 16-sampleL[disX+1], GRAPHICS_NORMAL );
    }
  }

}

void setPixelColorAndBrightness(int brightness, int bin, int pixelnumber){
  if (bin == 0){
strip.setPixelColor(pixelnumber,  ((brightness*GBin0)/255) , ((brightness*RBin0)/255) , ((brightness*BBin0)/255));
strip.show();
Serial.print("Brightness bin 0");
Serial.println(brightness);
  }
  else if (bin == 2){
strip.setPixelColor(pixelnumber,  ((brightness*GBin2)/255) , ((brightness*RBin2)/255) , ((brightness*BBin2)/255));
strip.show();
  }
}
/*
int readPotsADC(){
    //ADCSRA = 0xe7; // set the adc to free running mode
    ADCSRA = 0xC7; // turn off free running mode.
    
    //lowerLedBin = analogRead(topLedBrightnessPot); 
    /*
    while(!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    ADMUX = 0x46; // use adc2
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    
    ADCSRA = 0xe7; // set the adc to free running mode
    return lowerLedBin;

}
*/
void sampleInput() {
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int x=0; x<FHT_N; x++) {
    while(!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    if (sampleSet == "L") {
      ADMUX = 0x41; // use adc1
      //ADMUX = 0x1; // use adc1
    } else {
      ADMUX = 0x42; // use adc2
      //ADMUX = 0x2; // use adc2
    }
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fht_input[x] = k; // put real data into bins
  }
  sei();
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run(); // process the data in the fht
  fht_mag_lin();

}

void sampleFix() {

  int newPos; 
  float fhtCount, tempY;
  for (int x=0; x < displaySize; x++) {
    fhtCount = FHT_N/2;
    newPos = x * (fhtCount / displaySize); // single channel half-display 15-LED wide
    tempY = fht_lin_out[newPos]; 
    if (sampleSet == "L") {
    sampleL[x] = ((tempY/256)*16); // single channel full 16 LED high
    } else {
    sampleR[x] = ((tempY/256)*16); // single channel full 16 LED high
    }
  }  

} 


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
/*
void makeColor() {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      color[i] = Wheel((i+j) & 255);
     
    }
  }
}
*/
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

void WheelBin0(byte WheelPos) {
  if(WheelPos < 85) {
    GBin0 = WheelPos * 3;
    RBin0 = 255 - WheelPos * 3;
    BBin0 = 0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   GBin0 = 255 - WheelPos * 3;
   RBin0 = 0;
   BBin0 = WheelPos * 3;
  } else {
   WheelPos -= 170;
   GBin0 = 0;
   RBin0 = WheelPos * 3;
   BBin0 = 255 - WheelPos * 3;
  }
}

void WheelBin2(byte WheelPos) {
  if(WheelPos < 85) {
    GBin2 = WheelPos * 3;
    RBin2 = 255 - WheelPos * 3;
    BBin2 = 0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   GBin2 = 255 - WheelPos * 3;
   RBin2 = 0;
   BBin2 = WheelPos * 3;
  } else {
   WheelPos -= 170;
   GBin2 = 0;
   RBin2 = WheelPos * 3;
   BBin2 = 255 - WheelPos * 3;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

