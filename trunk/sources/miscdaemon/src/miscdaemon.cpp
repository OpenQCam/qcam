#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "miscstatus.h"

#define INTERVAL_TO_CHECK_DAEMON 20


int main(int argc, char** argv)
{
  int  fifo_fd;
  int  num;
  uint32_t waitTime = 0;
  uint32_t buf[1];

  uint32_t interval2CheckDaemon = 0x1 << INTERVAL_TO_CHECK_DAEMON;
  string output;

  if (argc >= 2) {
    waitTime = strtoul(argv[1],NULL,0) * 1000; // in us
  }
  else {
    waitTime = 60000; // 60000 us = 60 ms
  }

  if (waitTime < 60000) {
    waitTime = 60000;
    // limitationi: 60 ms, or it will polling the wrong status
  }

  printf("polling system MISC status once per %d us abcd \n", waitTime);

  while(1)  //polling the setup button
  {
    interval2CheckDaemon >>= 0x1;
    if( interval2CheckDaemon == 0x1){
      interval2CheckDaemon <<= INTERVAL_TO_CHECK_DAEMON;
      MISCStatus::DoSystemCommand("ps |grep `cat /var/run/qcam.pid`", output);
      if(!output.length()){
        printf("QCAM daemon seems dead, try to restart daemon....\n");
        MISCStatus::DoSystemCommand("sh /media/sda1/streamer/start_qcam.sh", output);
      }
    }

    usleep(waitTime); // wait
  }

  close(fifo_fd);
  return 0;
}

