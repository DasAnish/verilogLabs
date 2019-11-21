#include "avalon_addr.h"

int avolon_read(unsigned int address) {
  volatile int *pointer = (volatile int *) address;
  return pointer[0];
}

void avolon_write(unsigned int address, int data) {
  volatile int *pointer = (volatile int *)  address;
  pointer[0] = data;
}

// our pixel format in memory is 5 bits of red, 6 bits of green, 5 bits of blue
#define PIXEL16(r,g,b) (((r & 0x1F)<<11) | ((g & 0x3F)<<5) | ((b & 0x1F)<<0))
// ... but for ease of programming we refer to colours in 8/8/8 format and discard the lower bits
#define PIXEL24(r,g,b) PIXEL16((r>>3), (g>>2), (b>>3))

#define PIXEL_WHITE PIXEL24(0xFF, 0xFF, 0xFF)
#define PIXEL_BLACK PIXEL24(0x00, 0x00, 0x00)
#define PIXEL_RED   PIXEL24(0xFF, 0x00, 0x00)
#define PIXEL_GREEN PIXEL24(0x00, 0xFF, 0x00)
#define PIXEL_BLUE  PIXEL24(0x00, 0x00, 0xFF)
#define DISPLAY_WIDTH	480
#define DISPLAY_HEIGHT	272

void vid_set_pixel(int x, int y, int colour)
{
  // derive a pointer to the framebuffer described as 16 bit integers
  volatile short *framebuffer = (volatile short *) (FRAMEBUFFER_BASE);
  // make sure we don't go past the edge of the screen
  if ((x<0) || (x>DISPLAY_WIDTH-1))
    return;
  if ((y<0) || (y>DISPLAY_HEIGHT-1))
    return;

  framebuffer[x+y*DISPLAY_WIDTH] = colour;
}

void hex_output(int value) {
  avolon_write(PIO_HEX_BASE, value);

}

void led_output(int value) {
  avolon_write(PIO_LED_BASE, value);
}

/*
Whenever there is a roll over we can note that as 
high -> low || low -> high
*/

int valChange(int prev, int new) {
  if (prev < new && new - prev > 100 ) { //Going down overflow
    return new - prev - 256;    
  } else if (prev > new && prev - new > 100) { //going up overflow
    return new - prev + 256;
  }else {
    return new - prev;
  }
}


void clear_screen() {

  volatile short *framebuffer = (volatile short *) (FRAMEBUFFER_BASE);
  
  for (int x=0; x < DISPLAY_WIDTH; x++) {
    for (int y=0; y < DISPLAY_HEIGHT; y++) {
      framebuffer[x + y * DISPLAY_WIDTH] = PIXEL_BLACK;
    }
  }
}

int main(void) {

  int true = 1, false = 0;
  int curPosX = 0;// DISPLAY_WIDTH >> 1;
  int curPosY = 0;//DISPLAY_HEIGHT >> 1;

  int prevLeft = 0;
  int prevRight = 0;

  int left, right;
  int buttons;

  while(true) {

    left = avolon_read(PIO_ROTARY_L);
    right = avolon_read(PIO_ROTARY_R);

    int leftChange = valChange(prevLeft, left);
    int rightChange = valChange(prevRight, right);

    prevLeft = left;
    prevRight = right;

    curPosX += leftChange;
    curPosY += rightChange;

    

    if (curPosX >= DISPLAY_WIDTH) {
      curPosX = DISPLAY_WIDTH -1;
    } else if (curPosX < 0) {
      curPosX = 0;
    }

    if (curPosY >= DISPLAY_HEIGHT) {
      curPosY = DISPLAY_HEIGHT - 1;
    } else if (curPosY < 0) {
      curPosY = 0;
    }

    hex_output(curPosX);
    led_output(curPosY);
    

    vid_set_pixel(curPosX, curPosY, PIXEL_WHITE);


    buttons = avolon_read(PIO_BUTTONS);

    if ((buttons >> 1) & 1 || (buttons >> 2) & 1) {
      clear_screen();
      curPosX = DISPLAY_WIDTH >> 1;
      curPosY = DISPLAY_HEIGHT >> 1;
    }
    
  }

}
