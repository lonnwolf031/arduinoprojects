#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
/*
   Written by: Lonn  https://github.com/lonnwolf031 2020

   Using Adafruit NeoPixel 16x16 matrix and DS1307 RTC Module on Arduino Uno
   Using "DS1307RTC" library by Michael Margolis, including "Time" library also by Michael Margolis
   Using "Adafruit NeoPixel" library by Adafruit

   I used byte data for the colors and byte arrays. This because it minimizes memory and rgb has r, g and b values [0,255] like a byte
   Also the matrix consists of 256 LEDs which because zero-indexed, falls exactly in the byte range [0,255]

   The LED numbers that correspond with the text are hardcoded in arrays. Because the letters are on top of the
   matrix there is no "smart" way of doing that.

   The most efficient way of making a sort of "multidimensional jagged array" is with arrays and an array of pointers
   A real multidimensional array would not be the best way because the arrays have no fixed width
   So a static jagged array is used with an array and a pointer. For instance: the LED numbers of each minute are hardcode ("one" is the min1 array, so the sequence is logical and the numbers easier to find)
   After the arrays for each minute up until twenty (because in text only those are needed) a 1D array of pointers
   of size 21 is made with addresses of each minute array. Because it is zero indexed, the minutes later can be found by number (zero --> o'clock, 1 --> one ...)

   There also is an array with array sizes of each min for the for loops to show each of those LEDs


*/
#define DEBUG

// time of delay in millis
#define DELAYVAL 500
// number of LEDs in NeoPixel matrix
#define numpixels 256

const byte pin = 6;
const byte vclock = 3;

tmElements_t tm;
tmElements_t td;

Adafruit_NeoPixel matrix(numpixels, pin, NEO_RGB + NEO_KHZ800);

byte timehours = 0;
byte timemins = 0;
byte timeminsold = 99;
byte rgb_r;
byte rgb_g;
byte rgb_b;
byte hoursindex;

const byte itis[4] = {0, 31, 63, 64};
const byte past[4] = {8, 23, 40, 55};
const byte to[2] = {231, 248};
const byte half[4] = {7, 24, 39, 56};
const byte oclock[6] = {171, 180, 203, 212, 235, 244};

const byte min0[6] = {171, 180, 203, 212, 235, 244 };  // = o'clock
const byte min1[3] = {223, 224, 255};
const byte min2[3] = {1, 30, 33};
const byte min3[5] = {3, 28, 35, 60, 67};
const byte min4[4] = {194, 221, 226, 253};
const byte min5[4] = {2, 29, 34, 61};
const byte min6[3] = {5, 26, 37};
const byte min7[5] = {6, 25, 38, 57, 70};
const byte min8[5] = {133, 154, 165, 186, 197};
const byte min9[4] = {99, 124, 131, 156};
const byte min10[3] = {65, 94, 97};
const byte min11[6] = {93, 98, 125, 130, 157, 162};
const byte min12[5] = {166, 185, 196, 217, 230};
const byte min13[8] = {129, 158, 161, 190, 193, 222, 225, 254};
const byte min14[8] = {4, 27, 36, 59, 68, 91, 100, 123};
const byte min15[7] = {103, 120, 135, 152, 167, 184, 199};  // quarter
const byte min16[7] = {5, 26, 37, 58, 69, 90, 101};
const byte min17[9] = {6, 25, 38, 57, 70, 89, 102, 121, 134};
const byte min18[8] = {133, 154, 165, 186, 197, 218, 229, 250};
const byte min19[8] = {99, 124, 131, 156, 163, 188, 195, 220};
const byte min20[6] = {96, 127, 128, 159, 160, 191};

const byte* mins[21] = {min0, min1, min2, min3, min4, min5, min6, min7, min8, min9, min10, min11, min12, min13, min14, min15, min16, min17, min18, min19, min20};

const byte minsArraySize[21] = {6, 3, 3, 5, 4, 4, 3, 5, 5, 4, 3, 6, 5, 8, 8, 7, 7, 9, 8, 8, 6}, k = 0;  // Array to hold the size of each row

const byte hour1[3] = {87, 104, 119};
const byte hour2[3] = {151, 168, 183};
const byte hour3[5] = {75, 84, 107, 116, 139};
const byte hour4[4] = {118, 137, 150, 169};
const byte hour5[4] = {201, 214, 233, 246};
const byte hour6[3] = {215, 232, 247};
const byte hour7[5] = {10, 21, 42, 53, 74};
const byte hour8[5] = {106, 117, 138, 149, 170};
const byte hour9[4] = {202, 213, 234, 245};
const byte hour10[3] = {11, 20, 43};
const byte hour11[6] = {41, 54, 21, 42, 53, 74};
const byte hour12[6] = {9, 22, 41, 54, 73, 86};

const byte* hrs[12] = {hour1, hour2, hour3, hour4, hour5, hour6, hour7, hour8, hour9, hour10, hour11, hour12};

const byte hrsArraySize[12] = {3, 3, 5, 4, 4, 3, 5, 5, 4, 3, 6, 6};

const byte at[2] = {141, 146};
const byte inthe[5] = {12, 19, 51, 76, 83};

const byte morning[7] = {13, 18, 45, 50, 77, 82, 109};
const byte afternoon[9] = {115, 140, 147, 172, 179, 204, 211, 236, 243};
const byte evening[7] = {14, 17, 46, 49, 78, 81, 110};
const byte night[5] = {178, 205, 210, 237, 242};

void setup() {
  bool parse = false;
  bool config = false;
  
  // using digital pin for powering RTC
  pinMode(vclock, OUTPUT);
  digitalWrite(vclock, HIGH);

  #ifdef DEBUG
  // init Serial: just for debugging
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor
  delay(200);
  #endif

  /* get the date and time the compiler was run with: if (getTime(__TIME__))
   *  On my other setup it worked. Currently it does not, however if(initTime()) does work
   *  getTime is defined at the bottom and gets compiler time. 
   *  initTime is hardcoded setup time. 
   *  Don't know yet whether the problem comes from, but hardcoded time does work, although it is not the preferred way 
   *  (if you hardcode a time you cannot avoid time passing by before the RTC takes that as starting point, resulting in an inevitable delay 
   *  
   *  Also, when it didn't work I changed my Arduino IDE setup to 'Arduino Nano' with 'ATmega328P'  processor (although using an Uno the processor is the same)
   *  Don't know why it makes a difference but it does.
   *  
   *  Now it kinda works, but not well. The time updates seemingly randomly. 
   *  
   */
  //if (getTime(__TIME__)) {
  if(initTime()){
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      config = true;
    }
  }
  
  // start command for matrix init
  matrix.begin();
  matrix.setBrightness(150); // Set brightness(max = 255)

 #ifdef DEBUG
  if (parse && config) {
    Serial.print("DS1307 configured Time=");
    Serial.print(__TIME__);
  } else if (parse) {
    Serial.println("DS1307 Communication Error :-{");
    Serial.println("Please check your circuitry");
  } else {
    Serial.print("Could not parse info from the compiler, Time=\"");
    Serial.print(__TIME__);
    Serial.println("\"");
  }
  #endif
}

void loop() {
  if (RTC.read(td)) {
    //read current hour and minutes
    timehours = td.Hour;
    timemins = td.Minute;
  }
  // check if time in mins has changed

  if (timemins != timeminsold) {
#ifdef DEBUG
    Serial.println("inloop");
    Serial.println(timemins);
    Serial.println(td.Minute);
#endif
    // update old
    timeminsold = timemins;

    // make random colors by mixing random r, g, b values
    // this is in 'if' condition because we don't want to change colors every DELAYVAL, but only when time has changed
    rgb_r = random(0, 255);
    rgb_g = random(0, 255);
    rgb_b = random(0, 255);
    timeminsold = timemins;
    // clear previous text from matrix
    matrix.clear();

    // show 'it is'
    for (int i = 0; i < 4; i++) {
      matrix.setPixelColor(itis[i], matrix.Color(rgb_r, rgb_g, rgb_b));
    }
    // if between o'clock and .30 show 'past'
    if (timemins >= 0 && timemins < 30) {
      // check if minutes are lower than half an hour and show
      showNumLowerThirty();
      // show 'past'
      for (int i = 0; i < 4; i++) {
        matrix.setPixelColor(past[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
    }
    // at half an hour
    else if (timemins == 30) {
      for (int i = 0; i < 4; i++) {
        matrix.setPixelColor(half[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
      for (int i = 0; i < 4; i++) {
        matrix.setPixelColor(past[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
    }
    // if between half and o'clock
    else if (timemins > 30) {
      // adjust minutes to match written time
      timemins = 60 - timemins;
      showNumLowerThirty();
      // show 'to'
      for (int i = 0; i < 2; i++) {
        matrix.setPixelColor(to[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
      // add hour because of ... 'to' ....
      timehours = timehours + 1;
    }

    // show at night
    if (timehours >= 0 && timehours < 6) {
      for (int i = 0; i < 2; i++) {
        matrix.setPixelColor(at[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
      for (int i = 0; i < 5; i++) {
        matrix.setPixelColor(night[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
    }
    // in the morning
    else if (timehours >= 6 && timehours < 12) {
      for (int i = 0; i < 5; i++) {
        matrix.setPixelColor(inthe[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
      for (int i = 0; i < 7; i++) {
        matrix.setPixelColor(morning[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
    }
    // in the afternoon
    else if (timehours >= 12 && timehours < 18) {
      for (int i = 0; i < 5; i++) {
        matrix.setPixelColor(inthe[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
      for (int i = 0; i < 9; i++) {
        matrix.setPixelColor(afternoon[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
    }
    else if (timehours >= 18 && timehours <= 24) {
      for (int i = 0; i < 5; i++) {
        matrix.setPixelColor(inthe[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
      for (int i = 0; i < 7; i++) {
        matrix.setPixelColor(evening[i], matrix.Color(rgb_r, rgb_g, rgb_b));
      }
    }

    // zero-indexed hours, adjust in 0 - 12 instead of 24h notation
    if (timehours > 12) {
      timehours = timehours - 12;
    }
    hoursindex = timehours - 1;
    // show hours
    for (int i = 0; i < hrsArraySize[hoursindex]; i++) {
      matrix.setPixelColor(hrs[hoursindex][i], matrix.Color(rgb_r, rgb_g, rgb_b));
    }
#ifdef DEBUG
    Serial.print("Time is " );
    Serial.print(timehours);
    Serial.print(".");
    Serial.print(timemins);
    Serial.println("\"");
#endif
    
    // show text on matrix
    matrix.show();
    // do a delay, it would be unnecessary to  update continuously
    delay(DELAYVAL);
  }

}

void showNumLowerThirty() {
  // show time in minutes until, including twenty
  if (timemins >= 0 && timemins < 21) {
    for (int i = 0; i < minsArraySize[timemins]; i++) {
      matrix.setPixelColor(mins[timemins][i], matrix.Color(rgb_r, rgb_g, rgb_b));
    }
  }
  else if (timemins >= 21 && timemins < 30) {
    // this shows 20
    for (int i = 0; i < minsArraySize[20]; i++) {
      matrix.setPixelColor(mins[20][i], matrix.Color(rgb_r, rgb_g, rgb_b));
    }
    // print numbers after 20
    for (int i = 0; i < minsArraySize[timemins - 20]; i++) {
      matrix.setPixelColor(mins[timemins - 20][i], matrix.Color(rgb_r, rgb_g, rgb_b));
    }
  }
}

// this gets the current time from the RTC
bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool initTime()
{
  tm.Hour = 12;
  tm.Minute = 20;
  tm.Second = 0;
  return true;
}
