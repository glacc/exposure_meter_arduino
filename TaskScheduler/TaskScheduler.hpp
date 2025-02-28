#pragma once

#include <Arduino.h>

#include <avrcontext_arduino.h>

#ifndef __AVR__
#error AVR board only
#endif

#define TASK_NOTASK     0
#define TASK_READY      1
#define TASK_SUSPENDED  2

struct Task
{
  avr_context_t context;
  long delayStartMs;
  int delayDurationMs;
  uint8_t priority;
  uint8_t state;
};

namespace TaskScheduler
{
  extern Task *CurrentTask();

  extern void Init(Task *initialTaskList, uint8_t maxTasks);

  extern int AddTask(void (*taskFunc)(void *), void *stackAddr, uint16_t stackSize, uint8_t priority = 0);
  extern void SuspendTask(int taskId);
  extern void ResumeTask(int taskId);
  extern void EndTask();

  extern void Run();

  extern void Yield();
  extern void Delay(int ms);
  extern void Suspend();

  /* Internal functions */
  extern void ReSetupSystemTimer();
  extern void NextTask();
}
