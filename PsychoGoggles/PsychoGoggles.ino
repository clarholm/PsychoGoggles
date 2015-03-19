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



void setup() {
  if (displayType == "MONO") {displaySize = 32;} else {displaySize = 16;}
  //TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe7; // set the adc to free running mode
  ADMUX = 0x45; // use adc5
  DIDR0 = 0x20; // turn off the digital input for adc5
  Serial.begin(115200);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  
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
  
  // Some example procedures showing how to display to the pixels:
  ///colorWipe(strip.Color(0, 255, 0), 100000); // Red
  //colorWipe(strip.Color(0, 255, 0), 50); // Green
  //colorWipe(strip.Color(0, 0, 255), 50); // Blue
  // Send a theater pixel chase in...
  //theaterChase(strip.Color(127, 127, 127), 50); // White
  //theaterChase(strip.Color(127,   0,   0), 50); // Red
  //theaterChase(strip.Color(  0,   0, 127), 50); // Blue
  
  //rainbow(9000);
  //rainbowCycle(2000);
  //theaterChaseRainbow(50);
}


void drawSpectrum () {

  if (displaySize == 16) {
    //Serial.println("Start ");
    int disX = 0;
    for (disX; disX < 16; disX++) {
      
   //bin 0   
      if (disX == 0 && sampleL[disX+1] > 3 && sampleL[disX+1] > currentLmaxBin0){
        
        currentLmaxBin0 = sampleL[disX+1];
        //Serial.print("increse");
        //Serial.println(currentLmaxBin0);
        brightness = currentLmaxBin0 * 5;
        strip.setPixelColor(0, (brightness*255/255) , (brightness*0/255), (brightness*100/255));
        strip.show();
      }
      if (disX == 0 && sampleL[disX+1] < currentLmaxBin0){
       
        currentLmaxBin0 = currentLmaxBin0 - 1;
        //Serial.print("decrease");
        //Serial.println(currentLmaxBin0);
       brightness = currentLmaxBin0 * 5;
       strip.setPixelColor(0, (brightness*255/255) , (brightness*0/255), (brightness*100/255));
       strip.show();
      }
      
      if (disX == 0 && sampleR[disX+1] > 3 && sampleR[disX+1] > currentRmaxBin0){
        
        currentRmaxBin0 = sampleR[disX+1];
        //Serial.print("increse");
        //Serial.println(currentLmaxBin0);
        brightness = currentRmaxBin0 * 5;
        strip.setPixelColor(1, (brightness*255/255) , (brightness*0/255), (brightness*100/255));
        strip.show();
      }
      if (disX == 0 && sampleR[disX+1] < currentRmaxBin0){
       
        currentRmaxBin0 = currentRmaxBin0 - 1;
        //Serial.print("decrease");
        //Serial.println(currentLmaxBin0);
       brightness = currentRmaxBin0 * 5;
       strip.setPixelColor(1, (brightness*255/255) , (brightness*0/255), (brightness*100/255));
       strip.show();
      }
      //bin 2   
      if (disX == 2 && sampleL[disX+1] > 3 && sampleL[disX+1] > currentLmaxBin2){
        
        currentLmaxBin2 = sampleL[disX+1];
        //Serial.print("increse");
        //Serial.println(currentLmaxBin0);
        brightness = currentLmaxBin2 * 5;
        strip.setPixelColor(2, (brightness*255/255) , (brightness*0/255), (brightness*100/255));
        strip.show();
      }
      if (disX == 2 && sampleL[disX+1] < currentLmaxBin2){
       
        currentLmaxBin2 = currentLmaxBin2 - 1;
        //Serial.print("decrease");
        //Serial.println(currentLmaxBin0);
       brightness = currentLmaxBin2 * 5;
       strip.setPixelColor(2, (brightness*255/255) , (brightness*0/255), (brightness*100/255));
       strip.show();
      }
      
      if (disX == 2 && sampleR[disX+1] > 3 && sampleR[disX+1] > currentRmaxBin2){
        
        currentRmaxBin2 = sampleR[disX+1];
        //Serial.print("increse");
        //Serial.println(currentLmaxBin0);
        brightness = currentRmaxBin2 * 5;
        strip.setPixelColor(3, (brightness*255/255) , (brightness*0/255), (brightness*100/255));
        strip.show();
      }
      if (disX == 2 && sampleR[disX+1] < currentRmaxBin2){
       
        currentRmaxBin2 = currentRmaxBin2 - 1;
        //Serial.print("decrease");
        //Serial.println(currentLmaxBin0);
       brightness = currentRmaxBin2 * 5;
       strip.setPixelColor(3, (brightness*255/255) , (brightness*0/255), (brightness*100/255));
       strip.show();
      }
         
      }
      
    /*DEBUG
      Serial.print(sampleL[disX+1]);
      Serial.print(" ");
      Serial.print(sampleR[disX+1]);
      Serial.print(" ");
      Serial.print(" | ");
    */
    }
  
  else {
    //dmd.clearScreen(true);
    for (int disX; disX < 33; disX++) {
      //dmd.drawLine (disX, 16, disX, 16-sampleL[disX+1], GRAPHICS_NORMAL );
    }
  }

}


void sampleInput() {
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int x=0; x<FHT_N; x++) {
    while(!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    if (sampleSet == "L") {
      ADMUX = 0x41; // use adc5
    } else {
      ADMUX = 0x42; // use adc4
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

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

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

