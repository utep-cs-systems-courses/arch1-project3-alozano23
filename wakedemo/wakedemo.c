#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include <math.h>

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}
/*
 the header for making above into assembly, make file called switch.c
 #define LED BIT6
 #define SW1 1
 #define SW2 2
 #define SW3 4
 #define SW4 8
 #define SWITCHES 15
 // define the functions being used with extern *return type* *function name*
 // extern int switches
 .balign2 //allocates space in which you will use whatever variables
 .text
 .global switch_interrupt_handler //telling the program we are using a function that will be used by other files
 .extern switch_update_interrupt_sense
 .extern switches
switch_interrupt_handler:
 call #switch_update_interrupt_sense
 xor.b #255, r12 //xor.b because you are xor-ing one byte at a time, not a word at a time r12>p2val
 and.b #15, r12 //p2val &SWITCHES -> 0000 1111
 mov.b r12, &switches
 pop r0
 */

// axis zero for col, axis 1 for row
short drawPos[2] = {10,10}, controlPos[2] = {10,10};
short velocity[2] = {3,8}, limits[2] = {screenWidth-36, screenHeight-8};

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;

int state = 0;
int row = screenHeight / 2, col = screenWidth / 2;
int length = 30;
u_char sCol, sRow = 0;
u_char sLen = 20;
u_int cornerColor = COLOR_GREEN;
u_int midColor = COLOR_PURPLE;
int tempHertz = 764;

void buzzer_init()
{
    timerAUpmode();    
    P2SEL2 &= ~(BIT6 | BIT7);
    P2SEL &= ~BIT7; 
    P2SEL |= BIT6;
    P2DIR = BIT6;
}

void buzzer_set_period(short cycles) /* buzzer clock = 2MHz.  (period of 1k results in 2kHz tone) */
{
  //the buzzer itself
  CCR0 = cycles;  
  CCR1 = cycles >> 1;		/* one half cycle */
}

void wdt_c_handler()
{
  static int secCount = 0;
  if(secCount%9 == 0){
    switch(midColor){
      case COLOR_GREEN:
	P1OUT |= LED;
	P1OUT ^= LED;
	P1OUT |= LED;
	break;
      case COLOR_PINK:
	P1OUT &= ~LED;
	break;
      case COLOR_PURPLE:
	P1OUT &= ~LED;
	P1OUT |= LED;
	P1OUT ^= LED;
	break;
      default:
        P1OUT ^= LED;
	P1OUT ^= LED;
	P1OUT ^= LED;
    }
  }
  secCount ++;
  if (secCount >= 100) {		/* 10/sec */
    secCount = 0;
    redrawScreen = 1;
  }
  if(secCount == 10){
    buzzer_set_period(0);
  }
}
  
void update_shape();
void update_shape_2();
void changeCornerState();
void changeState();
void state_control();


void main()
{
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  //buzzer_set_period(tempHertz);
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(COLOR_BLUE);
  while (1) {			/* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      //update_shape_2();
      state_control();
      
    }
    //P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    //P1OUT |= LED;	/* led on */
  }
}
        
void
update_shape()
{
  static unsigned char row = screenHeight / 2, col = screenWidth / 2;
  static char blue = 31, green = 0, red = 31;
  static unsigned char step = 0;
  if (switches & SW4) return;
  if (step <= 60) {
    int startCol = col - step;
    int endCol = col + step;
    int width = 1 + endCol - startCol;
    // a color in this BGR encoding is BBBB BGGG GGGR RRRR
    unsigned int color = (blue << 11) | (green << 5) | red;
    fillRectangle(startCol, row+step, width, 1, color);
    fillRectangle(startCol, row-step, width, 1, color);
    if (switches & SW3) green = (green + 1) % 64;
    if (switches & SW2) blue = (blue + 2) % 32;
    if (switches & SW1) red = (red - 3) % 32;
    step ++;
  } else {
     clearScreen(COLOR_BLUE);
     step = 0;
  }
}



/* Switch on S2 */
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}


void state_control(){
  if(switches & SW1){
    midColor = COLOR_PINK;
    cornerColor = COLOR_PURPLE;
    buzzer_init();
    buzzer_set_period(tempHertz);
    //P1OUT &= ~LED;
  }
  if(switches &SW2){
    midColor = COLOR_RED;
    cornerColor = COLOR_PINK;
    buzzer_init();
    buzzer_set_period(tempHertz*pow(2, -0.083));
  }
  if(switches &SW3){
    midColor = COLOR_GREEN;
    cornerColor = COLOR_RED;
    P1OUT ^=LED;
    buzzer_init();
    buzzer_set_period(tempHertz*pow(2, -0.250));
  }
  if(switches &SW4){
    midColor = COLOR_PURPLE;
    cornerColor = COLOR_GREEN;
    buzzer_init();
    buzzer_set_period(tempHertz*pow(2, -0.500));
  }
  update_shape_2();
}
void update_shape_2(){
  switch(state){
    case 3:
      changeState();
      changeCornerState();
      fillRectangle(col, row, length, length, midColor);
      clearScreen(COLOR_BLUE);
      
      state = 0;
      break;
    default:
      changeState();
      changeCornerState();
      state++;
      break;
  }
    
}

void changeState(){ 
  switch(state){
     case 0:
       for(int a = 0; a < length; ++a){
	 drawPixel(col+a, row+length, COLOR_GREEN);
	 if( a < length / 2){
	    drawPixel(col+a, row+length+a, COLOR_GREEN);
	    drawPixel(col+length-a, row+length+a, COLOR_GREEN);
	 }  
       }
       col -= length;
       break;
     case 1:
       for(int a = 0; a < length; ++a){
	 drawPixel(col+length, row+a, COLOR_GREEN);
	 if(a < length/2){
	   drawPixel(col+length-a, row+a, COLOR_GREEN);
	   drawPixel(col+length-a, row+length-a, COLOR_GREEN);
	 }
       }
       col +=length;
       break;
     case 2:
       for(int a = 0; a < length; ++a){
	 drawPixel(col+a, row, COLOR_GREEN);
	 if(a < length / 2){
	   drawPixel(col+a, row-a, COLOR_GREEN);
	   drawPixel(col+length-a, row-a, COLOR_GREEN);
	 }
       }
       col+=length;
       break;
     case 3:
       for(int a = 0; a < length; ++a){
	 drawPixel(col, row+a, COLOR_GREEN);
	 if(a < length /2){
	   drawPixel(col+a, row+a, COLOR_GREEN);
	   drawPixel(col+a, row+length-a, COLOR_GREEN);
	 }
       }
       row = screenHeight / 2, col = screenWidth/2;
       break;
  }
}
void changeCornerState(){
  switch(state){
    case 0:
      fillRectangle(sCol, sRow, sLen, sLen, cornerColor);
      sCol = screenWidth-20;
      break;
    case 1:
      fillRectangle(sCol, sRow, sCol, sLen, cornerColor);
      sCol = 0;
      sRow = screenHeight-20;
      break;
    case 2:
      fillRectangle(sCol, sRow, sLen, sLen, cornerColor);
      sCol = screenWidth-20;
      break;
    case 3:
      fillRectangle(sCol, sRow, sLen, sLen, cornerColor);
      sCol = 0;
      sRow = 0;
      break;
  }
  
}
    
	    
