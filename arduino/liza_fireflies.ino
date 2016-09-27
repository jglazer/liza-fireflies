/*
 * Liza Fireflies 
 * by John Keefe
 * http://johnkeefe.net
 * September 2016
 * Released under the MIT License
 *
 * A program for an ATtiny microcontroller that cycles through
 * patterns and colors. Designed for 50+ fireflies.
 * More at: 
 *
 * ------
 * Watchdog Sleep Example 
 * Demonstrate the Watchdog and Sleep Functions
 * LED on digital pin 0
 * 
 * KHM 2008 / Lab3/  Martin Nawrath nawrath@khm.de
 * Kunsthochschule fuer Medien Koeln
 * Academy of Media Arts Cologne
 *
 * Modified on 5 Feb 2011 by InsideGadgets (www.insidegadgets.com)
 * to suit the ATtiny85 and removed the cbi( MCUCR,SE ) section 
 * in setup() to match the Atmel datasheet recommendations
 * https://www.insidegadgets.com/2011/02/05/reduce-attiny-power-consumption-by-sleeping-with-the-watchdog-timer/
 *
 * -----
 * Colors code from
 * Adafruit Arduino - Lesson 3. RGB LED
 * https://learn.adafruit.com/adafruit-arduino-lesson-3-rgb-leds/parts
 * 
 * ------
 * Arduino IDE Settings ...
 *
 * Board: ATtiny
 * Processor: ATtiny85
 * Clock: 1 Mhz (Internal)
 * 
 * Programmer: Arduino as ISP
 */

#include <avr/sleep.h>
#include <avr/wdt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

volatile boolean f_wdt = 1;
int seconds_asleep = 0;
int frame = 0;

int seconds_max = 10;

int redPin = 2;
int greenPin = 0;
int bluePin = 1;
int watchdog_counter = 0;

void setup(){

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // cycle through colors on startup
  setColor(255,0,0);
  delay(2000);
  setColor(0,255,0);
  delay(2000);
  setColor(0,0,255);
  delay(2000);
  setColor(0,0,0);

  setup_watchdog(6); // 6 is about a second
  
  // Other settings:
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

  // generate a random base from the value of an unused analog pin
  randomSeed(analogRead(A3));
  
}


void loop(){

  //// MAIN PLAN
  
  // Different functions are called depending on the "frame" count
  
  // Each frame lasts 10 seconds
  // 6 frames a minute
  // 360 frames an hour

  if (frame < 360) firefly(1); // 1 = green
  if (frame >= 360 && frame < 540) firefly(0);  // 0  = blue 
  if (frame >= 540 && frame < 720) blueSolid();
  if (frame >= 720 && frame < 900) dapple(0);   // 0 just blues; 1 add orange
  if (frame >= 900 && frame < 1080) dapple (1);  // 0 just blues; 1 add orange
  if (frame >= 1080 && frame < 1260) halfOn(false); // false = orange
  if (frame >= 1260 && frame < 1440) halfOn(true);  // true = pink
  if (frame >= 1440 && frame < 1800) twinkle();
  if (frame >= 1800 && frame < 2160) softGlow();

  if (frame >= 2160) frame = 0;

   //// END MAIN PLAN

}

//// SLEEP FUNCTIONS

// set system into the sleep state 
// system wakes up when watchdog is timed out
void system_sleep() {

  // turn pinmodes to input to save power
  pinMode(redPin, INPUT);
  pinMode(greenPin, INPUT);
  pinMode(bluePin, INPUT);  
  
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out 
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON

  // restore pinmodes to outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  
  
}

// This sets up the watchdog timer ... called earlier in the program
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}
  
// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
  seconds_asleep++;  // increase counter by one
}

// COLOR AND PATTERN FUNCTIONS

void setColor(int red, int green, int blue)
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
} 

void frameShow(int blinks) {
  
  // During debugging, I used this function to quickly flash
  // the LED red with each frame.

  for (uint8_t i=0; i < blinks; i++) {
    setColor(255, 0, 0);
    delay(50);
    setColor(0,0,0);
    delay(50);
  }
  
}

void advanceFrame() {

    //// Uncomment the next line flashes the LED red each frame for debugging
    // frameShow(1);
    
    // advance a frame
    frame ++;
  
}

void firefly(bool useGreen) {
    
  // This function makes the LED flash like a 
  // firefly, sometimes with 1 flash and sometimes with 2
  // and either green or blue depending on the number in the firefly()
  // function.

  int blue_val;
  int green_val;
  
  // This whole function should consume 10secs
  // and advance one frame

  if (useGreen) {
    blue_val = 0;
    green_val = 255;
  } else {
    blue_val = 255;
    green_val = 0;
  }
  
  if (seconds_asleep >= 10) {            
           
    seconds_asleep = 0; // reset counter
  
    /// -- begin action stuff  --- ///

    // Each frame there's a little less than 
    // 1 in 3 chance of either a 1x firefly or a 2x firefly
    int glowPick = random(1,6);
    
    if (glowPick == 1) {
      setColor(0, green_val, blue_val);
      delay(2000);
      setColor(0,0,0); 
    } else  
    
    if (glowPick == 2) {
      if (useGreen) {
        setColor(0, green_val, blue_val);
        delay(500);
        setColor(0, 0, 0);
        delay(500);
        setColor(0, green_val, blue_val);
        delay(500);
        setColor(0,0,0);
        delay(500);
      }
         
     } else {
      
      delay(2000);

    }
    
    advanceFrame();
  
  }  

  system_sleep();
  
}

void blueSolid() {

  // This glows the led blue, occasionally
  // "undulating" by fading out and fading back in 

  // This whole function should consume 10secs
  // and advance one frame

  setColor(0, 0, 205); // dark blue

  // each frame there's a 1 in 4 chance it'll undulate
  int yesUndulate = random(1, 5);
  if (yesUndulate == 1) {

    // fade out from max to min in increments of 5 points:
    for (int fadeValue = 205 ; fadeValue >= 0; fadeValue -= 5) {
      // sets the value (range from 0 to 204):
      analogWrite(bluePin, fadeValue);
      // wait for 30 milliseconds to see the dimming effect
      delay(30);
    }
    
    // fade in from min to max in increments of 5 points:
    for (int fadeValue = 0 ; fadeValue <= 205; fadeValue += 5) {
      // sets the value (range from 0 to 205):
      analogWrite(bluePin, fadeValue);
      // wait for 30 milliseconds to see the dimming effect
      delay(30);
    }

    delay(7500);

  } else {
    // just leave it steady
    delay(10000);
  }

  advanceFrame();
  
}

void dapple(bool addOrange) {
    
  // When called with dapple(0), this function picks between
  // three blues. 
  // When called with dapple(1), orange is added into the mix

  // each frame, pick a blue to show
  int whichBlue = random (1,4);

  switch (whichBlue) {
    case 1: 
      if (addOrange) {
        setColor(220, 60, 0);  // orange
      } else {
        setColor(0,20,255); // royal blue
      }
      break;
    case 2:
      setColor(0, 102, 102);  // aqua blue
      break;
    case 3:
      setColor(51,51,255);  // sky blue
      break;
  }

  delay(10000);

  advanceFrame();
  
}

void halfOn(bool usePink) {
    
    // This function glows the LED for 10 seconds
    // and then douses it for 10 seconds.
    
    // If called with halfOn(0), it's orange
    // If called with halfOn(1), it's pink

    if (seconds_asleep >= 10) {

      advanceFrame();

      // glow for 10 seconds

      if (usePink) {
        setColor(210, 0, 3);  // warm pink
      } else {
        setColor(220, 60, 0);  // orange
      }
      
      delay(10000);
      advanceFrame();

      // douse the light for another 10 seconds
      setColor(0, 0, 0);
      seconds_asleep = 0;
     
     } else {
      
      system_sleep();
      
     }
  
}

void twinkle() {
    
  // Twinkle LED every fourth second 
  // and sleep in between

  if(seconds_asleep >= 10) {

    advanceFrame();
    seconds_asleep = 0;
    
  }

  // every fourth second, twinkle
  if (seconds_asleep % 4 == 0) {

    setColor(225, 0, 10);  // warm pink
    delay(50);
    setColor(0, 0, 0);
    
  }

  system_sleep();
}

void softGlow(){
    
  // This just glows the LED a soft green ... so I could collect the fireflies!
  
  setColor(0, 5, 0);  // quiet green
  delay(10000);
  advanceFrame();
  
}
