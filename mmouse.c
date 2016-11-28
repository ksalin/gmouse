#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>

int main(void)
{
  int fd, status, dtr = -1, old_dtr = -1, res;
  struct termios options;
  char buf[3];
      struct timeval start, end;

    long mtime, seconds, useconds;    

    gettimeofday(&start, NULL);

  fd = open("/dev/ttyS3", O_RDWR | O_NOCTTY | O_NDELAY);
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
  //options.c_cflag &= ~CNEW_RTSCTS;
  options.c_cflag     &=  ~CRTSCTS;
  options.c_cflag |= (CLOCAL | CREAD | CS7 | CSTOPB);
  options.c_lflag     =   0;
  options.c_iflag &= ~IXON;
  options.c_iflag &= ~IXOFF;
  options.c_oflag &= ~OPOST;
  //cfmakeraw(&fd);
  tcsetattr(fd, TCSANOW, &options);

  for(;;)
  {
    ioctl(fd,TIOCMGET,&status);
    if (status & TIOCM_DSR) // DTR
    {
      //printf("DTR not set\n");
      dtr = 0;
    }
    else
    {
      //printf("DTR is set\n");
      dtr = 1;
    }
    if (dtr != old_dtr)
    {
      old_dtr = dtr;
      printf("DTR toggled!\n");
      buf[0]=77;
      //usleep(100000);
      //if (dtr==1) {
        printf("Writing 'M'\n");
        for (int i=1; i<100; i++)
        {
        res = write(fd, &buf[0], 1);
        if (res != 1)
        {
          printf("Could not write 1 byte!\n");
        }
        usleep(1000);
        }  
    }
    //printf("%i\n", status);
    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    //printf("Elapsed time: %ld milliseconds\n", mtime);

    if (mtime >= 1000/40)
    {
      start = end;
  
      buf[0]=0b01001010;
      buf[1]=0b00000100;
      buf[2]=0b00000100;
      printf("Sending event\n");
      res = write(fd, &buf[0], 3);
      if (res != 3)
      {
        printf("Could not write 3 bytes!\n");
      }
    }
  }

  return 0;
}