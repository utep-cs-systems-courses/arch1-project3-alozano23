/** \file lcddemo.c
 *  \brief A simple demo that draws a string and square
 */

#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

/** Initializes everything, clears the screen, draws "hello" and a square */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;

  clearScreen(COLOR_BLUE);

   drawString5x7(20,20, "hello", COLOR_GREEN, COLOR_RED);

   fillRectangle(30,30, 60, 60, COLOR_ORANGE);
   clearScreen(COLOR_BLUE);
    
  int row = 50;
  int column = 50;

   for( int j = 0; j < 60; ++j) {
    drawPixel(column, row, COLOR_WHITE);
    column++;
  }
  clearScreen(COLOR_BLUE);
     
   int a;
   int length = 20;
   int fCol, fRow = 50;
  for(a = 0; a < length; a++){
    drawPixel(fCol+a, fRow+a, COLOR_BLACK);
    drawPixel(fCol+a, fRow, COLOR_PURPLE);
    drawPixel(fCol+length, fRow+a, COLOR_GREEN);
    
    if(a%2 == 1){
      //The idea of this for loop is to draw a vertical line when it reaches an
      //odd column so it can make a checker pattern of sorts
      for(int i = length-a; i > 0; i--){
	//the following code is supposed to do what the top is doing but horizontally
        drawPixel(i+a, a+fRow, COLOR_PINK);//horizontal
        drawPixel(length-a, fRow+i, COLOR_RED);//vertical
        drawPixel(i+a, fRow+i, COLOR_WHITE);//diagonal
      }
    }
  }
//length = 20;
  fCol = 35;
  fRow = 100;
  for(a = 0; a < length; a++){
     drawPixel(fCol, fRow+a, COLOR_PINK);
     drawPixel(fCol+a, fRow, COLOR_PINK);
     drawPixel(fCol+a, fRow+length, COLOR_PINK);
     drawPixel(fCol+length, fRow+a, COLOR_PINK);
     if(a%2 == 1){
        for(int i = 0; i < length; i++){
//drawPixel(fCol+i, fRow+a, COLOR_WHITE);
//drawPixel(fCol+a, fRow+i, COLOR_RED);
          drawPixel(fCol+i, fRow+length-a-i, COLOR_GREEN);
        }
     }
  }
  /*
  int sCol = 50,sRow = 100;
  int s;
  int length = 20;
  //This for loop is for an asterisk
  for(s = 0; s < length; ++s){
    drawPixel(sCol+s, sRow+s, COLOR_PINK); // southwest diangonal
    drawPixel(sCol+(length/2), sRow+s, COLOR_PINK); //straight down
    drawPixel(sCol+s, sRow+(length/2), COLOR_PINK);//straight horizontal
    drawPixel(sCol+s,(sRow+length)-s, COLOR_PINK); // should draw the northeast diangonal
  }
  */

  
}
