/**
 * Mouse Systems Serial Mouse Emulator
 * (C) 2016 Jussi Salin under GPLv3
 *
 * Emulates a serial mouse of Mouse Systems protocol on a serial
 * port. It can be connected to an old PC. Relative mouse data
 * is collected in a grabbed SDL window. Close the window to exit.
 *
 * I initially created this to try out an old painting program
 * that came with the original Genius Mouse serial mouse.
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <SDL/SDL.h>


/**
 * Compile time settings
 */

// Sensitivity multiplier setting, to adjust mouse sensitivity.
#define SENSITIVITY 1

// Maximum reports/second. 24 is theoretical maximum. (1200/(1+7+2)/5)
#define RATE 20

// Dimensions of SDL window. Should not matter if mouse grab succeeds.
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define WINDOW_BPP 8

// Debug output
#define DEBUG 1

// Serial port device
#define SERIAL_DEV "/dev/ttyS3"


/**
 * Main program
 */
int main(void)
{
  int fd, status, dtr = -1, old_dtr = -1, res;
  struct termios options;
  char buf[5];
  int x = 0, y = 0, left = 0, middle = 0, right = 0, button_changed = 0;

  // Open and initialize serial port. Configure it for raw 1200bps 7N2.
  fd = open(SERIAL_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
  {
    printf("Unable to open serial port!\n");
    return 1;
  }
  fcntl(fd, F_SETFL, 0);
  tcgetattr(fd, &options);
  cfsetispeed(&options, B1200);
  cfsetospeed(&options, B1200);
  options.c_cflag &= ~CSIZE;
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CRTSCTS;
  options.c_cflag |= (CLOCAL | CREAD | CS8 | CSTOPB);
  options.c_lflag = 0;
  options.c_iflag &= ~IXON;
  options.c_iflag &= ~IXOFF;
  options.c_oflag &= ~OPOST;
  tcsetattr(fd, TCSANOW, &options);

  // Initialize SDL
  SDL_Event event;
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
  {
      printf("Couldn't initialize SDL!\n");
      return 1;
  }
  atexit(SDL_Quit);
  SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, 0);
  SDL_WM_GrabInput(SDL_GRAB_ON);

  // Main loop  
  Uint32 last_time = SDL_GetTicks();
  for(;;)
  {
    // Collect relative mouse data
    while ( SDL_PollEvent(&event) == 1 )
    {
      switch (event.type) 
      {
      case SDL_MOUSEMOTION:
        x += event.motion.xrel;
        y += event.motion.yrel;
        #ifdef DEBUG
        printf("%i,%i -> %i,%i\n", event.motion.xrel, event.motion.yrel, x, y);
        #endif
        break;

      case SDL_MOUSEBUTTONDOWN:
        #ifdef DEBUG
        printf("Button down\n");
        #endif
        if (event.button.button == SDL_BUTTON_LEFT) left = 1;
        else if (event.button.button == SDL_BUTTON_MIDDLE) middle = 1;
        else if (event.button.button == SDL_BUTTON_RIGHT) right = 1;
        button_changed = 1;
        break;
      
      case SDL_MOUSEBUTTONUP:
        #ifdef DEBUG
        printf("Button up\n");
        #endif
        if (event.button.button == SDL_BUTTON_LEFT) left = 0;
        else if (event.button.button == SDL_BUTTON_MIDDLE) middle = 0;
        else if (event.button.button == SDL_BUTTON_RIGHT) right = 0;
        button_changed = 1;
        break;

      case SDL_QUIT: 
        return 0;

      default:
        break;
      }
    }

    // Send mouse packets at intervals, if there has been relative mouse movement
    Uint32 this_time = SDL_GetTicks();
    if ((this_time-last_time>=1000/RATE) && ((x!=0) || (y!=0) || (button_changed==1)))
    {
      last_time = this_time;

      buf[0] = 0b10000111;
      if (left == 1) buf[0] &= 0b11111011;
      if (middle == 1) buf[0] &= 0b11111101;
      if (right == 1) buf[0] &= 0b11111110;

      buf[1] = (float)x * SENSITIVITY;
      buf[2] = (float)-y * SENSITIVITY;

      buf[3] = 0;
      buf[4] = 0;

      x = 0;
      y = 0;
      button_changed = 0;

      #ifdef DEBUG
      printf("Sending mouse packet...\n");
      #endif
      res = write(fd, &buf[0], 5);
      if (res != 5) printf("Could not write mouse packet!\n");
    }
  }

  return 0;
}