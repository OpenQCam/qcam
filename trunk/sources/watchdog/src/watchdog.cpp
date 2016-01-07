/* ============================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 *            (C) COPYRIGHT Quanta Computer Inc.
 *                   ALL RIGHTS RESERVED
 *
 * ============================================================================
 * ----------------------------------------------------------------------------
 * Date    :
 * Version :
 * Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 * ----------------------------------------------------------------------------
 * Purpose :
 * ============================================================================
 */

#include "watchdog.h"

#define MAX_BUFFER 2048

WatchDog::WatchDog(const char* pidPath, const char* scriptPath, uint8_t interval)
: _interval2check(interval),
  _pidPath(pidPath),
  _restartScriptPath(scriptPath),
  _pUXClient(0)
{
  _pUXClient = new UnixDomainSocketClient("/tmp/testsocket");
  _counter = 10;
}

WatchDog::~WatchDog()
{
}

void WatchDog::ResetCounter()
{
  _counter = 10;
}

bool WatchDog::RestartDaemon(){
//  FILE *fp;
//  char buf[2048];
//  string killCommand = "kill -9 `cat " + _pidPath + "`";
//  string restartCommand = "sh " + _restartScriptPath;
//  printf("Try to kill process %s and  start daemon %s after 5s\n", STR(_pidPath), STR(_restartScriptPath));
//  system(STR(killCommand));
//  sleep(5);
//  fp = popen(STR(restartCommand), "r");
//  if (fp == NULL) {
//    printf("Failed to run command\n");
//    return false;
//  }
//  /* Read the output a line at a time - output it. */
//  if(fgets(buf, MAX_BUFFER, fp) != NULL){
//    printf("%s\n", buf);
//  }
//  /* close */
//  pclose(fp);
//  ResetCounter();
//
  return true;
}

bool WatchDog::CheckDaemon()
{

  if(_counter < 0){
    printf("Daemon seemed died! Restart Daemon...\n");
    RestartDaemon();
  }
  return true;

}

void WatchDog::Run()
{
  uint8_t ping[1] = {0x40};
  _pUXClient->RegisterMessageCallBack(reinterpret_cast<IMessageCallBack *>(this));
  do{
    if(_pUXClient->Connect()){
      for(;;){
        _pUXClient->Send(ping, 1);
        if(!_pUXClient->Run()){
          printf("Unable to ping daemon!\n");
          _pUXClient->Close();
          break;
        }
        sleep(_interval2check);
        CheckDaemon();
        _counter--;
      }
    }else{
      RestartDaemon();
    }
    sleep(_interval2check);
  }while(1);

}

void WatchDog::OnMessage(IOBuffer &buffer) {
  ResetCounter();
  buffer.IgnoreAll();
}

int main(int argc, char *argv[])
{
  if(argc < 4){
    printf("Usage: watchdog PIDPath DaemonPath Interval\n");
    printf("Example: watchdog /var/run/qcam.pid /media/sda1/streamer/daemon.sh 5\n");
  }else{
    string daemonPath(argv[2]);
    string interval(argv[3]);
    if(!fileExists(daemonPath)){
      printf("Daemon script didn't exist, path %s\n", STR(daemonPath));
      return 0;
    }
    if(!isNumeric(interval)){
      printf("interval should be integer only %s\n", STR(interval));
      return 0;
    }
    WatchDog watchdog(argv[1], argv[2], (uint8_t)(atoi(argv[3]) & 0xFF));
    watchdog.Run();
  }
  return 0;
}
