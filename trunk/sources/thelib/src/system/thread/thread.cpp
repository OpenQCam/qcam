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
// Date    : 2011/04/15
// Version :
// Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// ------------------------------------------------------------------
// Purpose : Implement thread helper class
// ==================================================================
*/

#include "system/thread/thread.h"

#include "common.h"

//#include <iostream>
//#include <stdlib.h>

using namespace std;


Mutex::Mutex()
{
  pthread_mutex_init(&_lock, NULL);
}
Mutex::~Mutex()
{
  pthread_mutex_destroy(&_lock);
}
void Mutex::Lock() {
  pthread_mutex_lock(&_lock);
}
void Mutex::UnLock() {
  pthread_mutex_unlock(&_lock);
}

Cond::Cond()
{
  pthread_cond_init(&_cond, NULL);
}
Cond::~Cond()
{
  pthread_cond_destroy(&_cond);
}
void Cond::Wait()
{
  pthread_cond_wait(&_cond, &_lock);
}
void Cond::Signal()
{
  pthread_cond_signal(&_cond);
}

uint32_t Thread::_idGenerator = 0;

Thread::Thread(THREAD_TYPE type)
  : _type(type)
  , _isRunning(false)
  , _isDetachable(false)
  , _pArg(0)
{
  _uniqueId = ++_idGenerator;
  INFO("uid = %d, gid=%d", _uniqueId, _idGenerator);
}

Thread::Thread(THREAD_TYPE type, bool dettach)
  : _type(type)
  , _isRunning(false)
  , _isDetachable(true)
  , _pArg(0)
{
  _uniqueId = ++_idGenerator;
  INFO("uid = %d, gid=%d", _uniqueId, _idGenerator);
}
Thread::~Thread()
{
  INFO("thread was deleted");
}

//void* Thread::Exec(void *obj) {
extern "C" void* Exec(void *obj) {
  Thread *pT = static_cast<Thread *>(obj);
  pT->Run();
  pT->JoinThread();
  pthread_exit(NULL);
  return NULL;
}

void Thread::StartThread()
{
  if(!_isRunning){
    if(pthread_attr_init(&_threadAttr) != 0){
      FATAL("init thread attr failed");
      exit(0);
    }
    if(_isDetachable){
      if(pthread_attr_setdetachstate(&_threadAttr, PTHREAD_CREATE_DETACHED) != 0){
        FATAL("set thread attr dettach failed");
        exit(0);
      }
    }
    //if( (ret = pthread_create(&_threadId, &_threadAttr, &Thread::Exec, (void *)this)) != 0) {
    if(pthread_create(&_threadId, &_threadAttr, Exec, static_cast<void *>(this)) != 0) {
      FATAL("thread create failed");
      exit(0);
    }
    _isRunning = true;
    INFO("%s", STR((_isDetachable) ? "Detachable thread was created" : "thread was created"));
  } else {
    FATAL("thread is already running");
  }
}

void Thread::StartThread(void *arg)
{
  _pArg = arg;
  StartThread();
}

void Thread::JoinThread()
{
  pthread_join(_threadId, NULL);
  _isRunning = false;
}

bool Thread::IsRunning()
{
  return _isRunning;
}

pthread_t Thread::GetThreadId()
{
  return _threadId;
}

uint32_t Thread::GetId()
{
  return _uniqueId;
}

THREAD_TYPE Thread::GetType()
{
  return _type;
}

