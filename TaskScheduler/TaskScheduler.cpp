/* 

  Preemptive Task Scheduler by Glacc

  (Based on the example from AVR-context)

*/

#include <avr/wdt.h>
#include <avr/sleep.h>

#include "TaskScheduler.hpp"

namespace
{
  Task *taskList = nullptr;
  uint8_t maxTasks;
  uint8_t currTaskId = 0;

  extern "C"
  {
    avr_context_t *volatile TaskScheduler_currContext;
  }
  avr_context_t dummyContext;
}

Task *TaskScheduler::CurrentTask()
{
  return &taskList[currTaskId];
}

void TaskScheduler::Init(Task *initialTaskList, uint8_t initialMaxTasks)
{
  taskList = initialTaskList;
  maxTasks = initialMaxTasks;

  memset(initialTaskList, 0, sizeof(Task) * maxTasks);

  avr_getcontext(&dummyContext);
}

int TaskScheduler::AddTask(void (*taskFunc)(void *), void *stackAddr, uint16_t stackSize, uint8_t priority)
{
  int taskId;
  for (int i = 0; i < maxTasks; i++)
  {
    if (taskList[i].state != TASK_NOTASK)
      continue;

    Task *task = &taskList[i];
    
    avr_getcontext(&task->context);
    avr_makecontext(
      &task->context,
      stackAddr,
      stackSize,
      &dummyContext,
      taskFunc,
      nullptr
    );

    task->delayDurationMs = -1;
    task->priority = priority;
    task->state = TASK_READY;

    taskId = i;

    break;
  }

  return taskId;
}

void TaskScheduler::SuspendTask(int taskId)
{
  cli();

  Task *task = &taskList[taskId];
  if (task->state == TASK_READY)
    task->state = TASK_SUSPENDED;

  if (taskId == currTaskId)
    Yield();

  sei();
}

void TaskScheduler::ResumeTask(int taskId)
{
  cli();

  Task *task = &taskList[taskId];
  if (task->state == TASK_SUSPENDED)
    task->state = TASK_READY;

  Task *currTask = &taskList[currTaskId];
  if (task->priority > currTask->priority)
    Yield();

  sei();
}

void TaskScheduler::EndTask()
{
  taskList[currTaskId].state = TASK_NOTASK;
  Yield();
}

void TaskScheduler::Run()
{
  asm volatile
  (
    "cli\n"
  );

  /* Setup timer. */
  ReSetupSystemTimer();

  asm volatile
  (
    "sei\n"
  );
}

__attribute__((naked)) void TaskScheduler::Yield()
{
  asm volatile
  (
    "cli\n"
  );

  AVR_SAVE_CONTEXT_GLOBAL_POINTER(
      "\n",
      TaskScheduler_currContext);
  
  TaskScheduler::NextTask();

  ReSetupSystemTimer();

  AVR_RESTORE_CONTEXT_GLOBAL_POINTER(TaskScheduler_currContext);

  asm volatile
  (
    "sei\n"
    "ret\n"
  );
}

void TaskScheduler::Delay(int ms)
{
  Task *task = &taskList[currTaskId];

  if (ms <= 0)
  {
    task->delayDurationMs = -1;
    return;
  }

  task->delayStartMs = millis();
  task->delayDurationMs = ms;

  Yield();
}

void TaskScheduler::Suspend()
{
  cli();

  Task *task = &taskList[currTaskId];
  task->state = TASK_SUSPENDED;

  Yield();

  sei();
}

void TaskScheduler::ReSetupSystemTimer()
{
  // Setup Timer2 overflow to fire every 8ms (125Hz)
  //   period [sec] = (1 / f_clock [sec]) * prescale * (255-count)
  //                  (1/16000000)  * 1024 * (255-130) = .008 sec

  TCCR2B = 0x00;        // Disable Timer2 while we set it up

  TCNT2  = 130;         // Reset Timer Count  (255-130) = execute ev 125-th T/C clock
  TIFR2  = 0x00;        // Timer2 INT Flag Reg: Clear Timer Overflow Flag
  TIMSK2 = 0x01;        // Timer2 INT Reg: Timer2 Overflow Interrupt Enable
  TCCR2A = 0x00;        // Timer2 Control Reg A: Wave Gen Mode normal
  TCCR2B = 0x07;        // Timer2 Control Reg B: Timer Prescaler set to 1024
}

void TaskScheduler::NextTask()
{
  int nextTaskId = -1;
  int highestRunningTaskPriority = -1;

  int taskIdToLookUp = currTaskId;
  while (true)
  {
    taskIdToLookUp++;
    if (taskIdToLookUp >= maxTasks)
      taskIdToLookUp = 0;

    Task *task = &taskList[taskIdToLookUp];

    // Skip no-task task slot
    if (task->state == TASK_NOTASK)
      continue;

    // If task is in delayed state
    if (task->delayDurationMs > 0)
    {
      if ((millis() - task->delayStartMs) > task->delayDurationMs)
        task->delayDurationMs = -1;
    }

    // If task is not in delayed state
    if (task->delayDurationMs <= 0)
    {
      // and the task is in ready state
      if (task->state == TASK_READY)
      {
        // Find the ready task with highest priority
        if (task->priority > highestRunningTaskPriority)
        {
          nextTaskId = taskIdToLookUp;
          highestRunningTaskPriority = task->priority;
        }
      }
    }

    if (taskIdToLookUp == currTaskId)
      break;
  }

  if (nextTaskId != -1)
  {
    currTaskId = nextTaskId;

    Task *task = &taskList[currTaskId];
    TaskScheduler_currContext = &task->context;
  }

  /*

  // Old task scheduling code without priority

  while (true)
  {
    currTaskId++;
    if (currTaskId >= maxTasks)
      currTaskId = 0;

    Task *task = &taskList[currTaskId];

    if (task->state != TASK_NOTASK)
    {
      if (task->delayDurationMs > 0)
      {
        if ((millis() - task->delayStartMs) > task->delayDurationMs)
        {
          task->delayDurationMs = -1;

          if (task->state == TASK_READY)
          {
            TaskScheduler_currContext = &task->context;
            break;
          }
        }
      }
      else if (task->state == TASK_READY)
      {
        TaskScheduler_currContext = &task->context;
        break;
      }
    }
  }
  */
}

ISR(TIMER2_OVF_vect, ISR_NAKED)
{
  // save the context of the current task
  AVR_SAVE_CONTEXT_GLOBAL_POINTER(
      "cli\n", // disable interrupts during task switching
      TaskScheduler_currContext);

  // switch to the other task.
  TaskScheduler::NextTask();

  // restore the context of the task to which we have just switched.
  AVR_RESTORE_CONTEXT_GLOBAL_POINTER(TaskScheduler_currContext);

  // return from the interrupt and activate the restored context.
  asm volatile
  (
    "reti\n"
  );
}
