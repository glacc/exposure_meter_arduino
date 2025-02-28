#pragma once

#include <Arduino.h>

#include <avr/wdt.h>
#include <avr/sleep.h>

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

struct Task
{
  uint16_t stackPtr;
  uint16_t stackSize;
  long delayStartMs;
  int delayDurationMs;
  uint8_t state;
};

namespace TaskScheduler
{
  extern Task *taskList;
  extern uint8_t maxTasks;
  extern uint8_t currTask;

  // extern uint16_t mainRoutineStackPtr;

  extern void Init(Task *initialTaskList, uint8_t maxTasks);

  extern void AddTask(void (*taskFunc)(), void *stackAddr, uint16_t stackSize);
  extern void EndTask();

  extern void Run();

  extern void Yield();
  extern void Delay(int ms);

  /* Internal function */
  extern void SetupTimer2();
  extern void NextTask();
}
