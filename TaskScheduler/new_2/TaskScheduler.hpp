#pragma once

#include <Arduino.h>

#include <avrcontext_arduino.h>

#ifndef __AVR__
#error AVR board only
#endif

/*
  States:

    0: No task
    1: Ready

*/

#define TASK_NOTASK 0
#define TASK_READY  1
#define TASK_YIELD  2

struct Task
{
  avr_context_t context;
  long delayStartMs;
  int delayDurationMs;
  uint8_t state;
};

namespace TaskScheduler
{
  /*
  extern Task *taskList;
  extern uint8_t maxTasks;
  extern uint8_t currTask;
  */

  /*
  extern "C"
  {
    extern avr_context_t *volatile currContext;
  }
  extern avr_context_t dummyContext;
  */

  // extern uint16_t mainRoutineStackPtr;

  extern Task *CurrentTask();

  extern void Init(Task *initialTaskList, uint8_t maxTasks);

  extern void AddTask(void (*taskFunc)(), void *stackAddr, uint16_t stackSize);
  extern void EndTask();

  extern void Run();

  extern void Yield();
  extern void Delay(int ms);

  /* Internal function */
  extern void ReSetupSystemTimer();
  extern void NextTask();
}
