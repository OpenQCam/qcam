/*
// ==================================================================
// This confidential and proprietary software may be used only as
// authorized by a licensing agreement from Quanta Computer Inc.
//
//            (C) COPYRIGHT Quanta Computer Inc.
//                   ALL RIGHTS RESERVED
//
// ==================================================================
// ------------------------------------------------------------------
// Date    : 2010/04/15
// Version :
// Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// ------------------------------------------------------------------
// Purpose : Thread helper class
// ==================================================================
*/

#ifndef _THREAD_H
#define _THREAD_H

#include "common.h"
#include <pthread.h>

enum THREAD_TYPE {
  THREAD_T_RECORD = 0,
  THREAD_T_COMMAND,
  THREAD_T_IVA
};

class Mutex {
  protected:
    pthread_mutex_t _lock;
  public:
    Mutex();
    ~Mutex();
    void Lock();
    void UnLock();
};

class Cond : public Mutex {
  private:
    pthread_cond_t _cond;
  public:
    Cond();
    ~Cond();
    void Wait();
    void Signal();
};


/**
* @Brief Thread helper class for wrappering pthread
*/
class Thread {
private:
  static uint32_t _idGenerator;
  THREAD_TYPE _type;
  bool _isRunning;
  bool _isDetachable;
  pthread_t _threadId;
  pthread_attr_t _threadAttr;

protected:
  uint32_t _uniqueId;
  void *_pArg;
  //static void* Exec(void *);
public:
  Thread(THREAD_TYPE type);
  Thread(THREAD_TYPE type, bool dettach);
  virtual ~Thread();

  virtual void* Run() = 0;

  void StartThread();
  void StartThread(void *arg);
  void JoinThread();
  bool IsRunning();
  pthread_t GetThreadId();
  uint32_t GetId();
  THREAD_TYPE GetType();

private:
  // prevent from copy and assignment
  Thread(const Thread &arg);
  Thread& operator=(const Thread& rhs);

};


#endif
