/*
 * Copyright (c) 2013, Three Ocean (to@bcloud.us). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "system/thread/threadmanager.h"

//map<uint8_t, THREAD_QUEUE*> ThreadManager::_availableThreads;

map<THREAD_TYPE, queue<Thread*>*> ThreadManager::_availableThreadQueue;
map<uint32_t, Thread*> ThreadManager::_threadPool;
Mutex ThreadManager::_mutex;

bool ThreadManager::CreateThreadPool(THREAD_TYPE type)
{
  _availableThreadQueue[type] = new queue<Thread*>;
  return true;
}

void ThreadManager::AddThreadToPool(Thread *pThread)
{
  _threadPool[pThread->GetId()] = pThread;
}

void ThreadManager::Cleanup()
{
  Thread *pThread = NULL;
  for(map<THREAD_TYPE, queue<Thread*>*>::iterator i=_availableThreadQueue.begin();
      i != _availableThreadQueue.end();) {
    while( (pThread = GetAvailableThread(MAP_KEY(i))) != NULL) {
      MAP_ERASE1(_threadPool, pThread->GetId());
      delete pThread;
      pThread = NULL;
    }
    _availableThreadQueue.erase(i++);
  }
}

Thread* ThreadManager::GetAvailableThread(THREAD_TYPE type)
{
  _mutex.Lock();
  if(!MAP_HAS1(_availableThreadQueue, type)){
    WARN("Unknown thread type %d", type);
    _mutex.UnLock();
    return NULL;
  }
  queue<Thread*> *pThreadQueue = _availableThreadQueue[type];
  if(pThreadQueue->empty()){
    WARN("No available thread");
    _mutex.UnLock();
    return NULL;
  }
  Thread *pThread = pThreadQueue->front();
  pThreadQueue->pop();
  INFO("available threads(%d) %d", (uint8_t)type, pThreadQueue->size());
  _mutex.UnLock();
  return pThread;
}
bool ThreadManager::EnqueueThread(THREAD_TYPE type, Thread *pThread)
{
  _mutex.Lock();
  if(!MAP_HAS1(_availableThreadQueue, type)){
    WARN("Unknown thread type %d", type);
    _mutex.UnLock();
    return false;
  }
  queue<Thread*> *pThreadQueue = _availableThreadQueue[type];
  pThreadQueue->push(pThread);
  //_threadPool[pThread->GetId()] = pThread;
  INFO("available threads(%d) %d", (uint8_t)type, GetNumberOfAvailableThread(type));
  _mutex.UnLock();
  return true;
}

uint32_t ThreadManager::GetNumberOfAvailableThread(THREAD_TYPE type)
{
  if(!MAP_HAS1(_availableThreadQueue, type)){
    WARN("Unknown thread type %d", type);
    return false;
  }
  return _availableThreadQueue[type]->size();
}

Thread* ThreadManager::FindThreadById(uint32_t id)
{
  if(!MAP_HAS1(_threadPool, id)){
    return NULL;
  }
  return _threadPool[id];
}
