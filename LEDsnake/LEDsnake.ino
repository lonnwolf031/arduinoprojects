#include "LedControl.h"

#define DEBUG

/*
   Written by: Lonn  https://github.com/lonnwolf031 2017

   Using a 8x8 single color LED matrix and joystick on Arduino Uno.
   Implementation of the well-known snake game.
*/

LedControl lc=LedControl(12,11,10,1);

int joystickX = A1;
int joystickY = A0;
int valueX = 512; 
int valueY = 512;
int fruitX, fruitY, snakeDirection = 0;
unsigned long delaytime=500;
unsigned long flash=200;
enum matrix {
  MINX = 0, 
  MINY = 0,
  MAXX = 7,
  MAXY = 7
};
enum sDirection {
  TOP = 1,
  DOWN = 2,
  LEFT = 3,
  RIGHT = 4
};
const int MAX_SNAKE_LENGTH = 10;

int snakeX[MAX_SNAKE_LENGTH];                      // X-coordinates of snake
int snakeY[MAX_SNAKE_LENGTH];                      // Y-coordinates of snake
int snakeLength = 1;

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  // wake up from power saving mode
  lc.shutdown(0,false);
  // medium brightness values
  lc.setIntensity(0,8);
  // clear the display
  lc.clearDisplay(0);
  // init fruit
  makeFruit();
  setSnake();
}

void drawSnake(){
  for(int i=0; i<snakeLength; i++){
    lc.setLed(0,snakeX[i], snakeY[i],true);
  }
}

void setSnake() {
  snakeLength = 1;
  snakeX[0] = 4;
  snakeY[0] = 7;
  for(int i=1; i<MAX_SNAKE_LENGTH; i++){
    snakeX[i] = snakeY[i] = -1;
  }
}

void drawFruit() {
  lc.setLed(0,fruitX,fruitY,true);
  delay(flash);
  lc.setLed(0,fruitX,fruitY,false);
  delay(flash);
  lc.setLed(0,fruitX,fruitY,true);
}

void readDirection() {
  valueX = analogRead(joystickX);
  valueY = analogRead(joystickY);
  if(valueX < 400) {
    snakeDirection = LEFT;
  }
  else if(valueX>600) {
    snakeDirection = RIGHT;
  }
  if(valueY < 400) {
    snakeDirection = TOP;
  }
  else if(valueY>600) {
    snakeDirection = DOWN;
  }
}

void nextstep(){
  for(int i=snakeLength-1; i>0; i--){
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }
  switch(snakeDirection){
    case TOP:
      snakeY[0] = snakeY[0]-1;
      break;
    case RIGHT:
      snakeX[0] = snakeX[0]+1;
      break;
    case DOWN:
      snakeY[0] = snakeY[0]+1;
      break;
    case LEFT:
      snakeX[0]=snakeX[0]-1;
      break;
  }
  if(snakeX[0] < MINX) {
    snakeX[0] = MAXX;
  }
  else if(snakeX[0] > MAXX) {
    snakeX[0] = MINX;
  }
  if(snakeY[0] < MINY) {
    snakeY[0] = MAXY;
  }
  else if(snakeY[0] > MAXY) {
    snakeY[0] = MINY;
  }
  for(int i=1; i < snakeLength; i++) {
    if((snakeX[0] == snakeX[i]) && (snakeY[0] == snakeY[i])) {
      showLost();
    }
  }
  if((snakeX[0] == fruitX) && (snakeY[0] == fruitY)){
    snakeLength++;
    if(snakeLength < MAX_SNAKE_LENGTH){      
      makeFruit();
    } 
    else {
      showWin();
    }
  }
}

void makeFruit(){
  int x, y;
  x = random(0, 8);
  y = random(0, 8);
  fruitX = x;
  fruitY = y;
}

// show happy face
void showWin() {
  lc.clearDisplay(0);
  lc.setColumn(0,6,B00100100);
  lc.setColumn(0,5,B00100100);
  lc.setColumn(0,2,B01000010);
  lc.setColumn(0,1,B00111100);
  delay(5000);
  //restart game
  setSnake();
}

// show sad face
void showLost() {
  lc.clearDisplay(0);
  lc.setColumn(0,6,B00100100);
  lc.setColumn(0,5,B00100100);
  lc.setColumn(0,2,B00111100);
  lc.setColumn(0,1,B01000010);
  delay(5000);
  //restart game
  setSnake();
}

void loop() {
  lc.clearDisplay(0);
  drawSnake();
  drawFruit();
  readDirection();
  nextstep();  
}
