/* 

  Preemptive Task Scheduler by Glacc

  Based on: https://gist.github.com/Barteks2x/2f0487554401275ebd600c220ad874fd

 */

#include "TaskScheduler.hpp"

using namespace TaskScheduler;

static Task *TaskScheduler::taskList = nullptr;
static uint8_t TaskScheduler::maxTasks;
static uint8_t TaskScheduler::currTask;

// static uint16_t TaskScheduler::mainRoutineStackPtr;

static void TaskScheduler::Init(Task *initialTaskList, uint8_t initialMaxTasks)
{
  taskList = initialTaskList;
  maxTasks = initialMaxTasks;
}

static void TaskScheduler::AddTask(void (*taskFunc)(), void *stackAddr, uint16_t stackSize)
{
  uint8_t sreg = SREG;

  for (int i = 0; i < maxTasks; i++)
  {
    if (taskList[i].state != TASK_NOTASK)
      continue;

    Task *task = &taskList[i];
    
    task->stackSize = stackSize;

    /* Initialize task stack */
    void *taskStackPtr = (void *)(stackAddr + stackSize - 1);

    /* Execute EndTask() when task routine ends. */
    *(uint16_t *)(taskStackPtr - 1) = EndTask;
    taskStackPtr -= 2;
    /* Enter task when scheduled. */
    *(uint16_t *)(taskStackPtr - 1) = (uint16_t)taskFunc;
    taskStackPtr -= 2;
    /* Initialize register for context switching. */
    for (int r = 0; r < 33; r++)
    {
      *(uint8_t *)taskStackPtr = 0;

      if (r == 1)
        *(uint8_t *)taskStackPtr = sreg;

      taskStackPtr--;
    }
    taskStackPtr--;

    task->stackPtr = (uint16_t)taskStackPtr; 

    task->delayDurationMs = -1;
    task->state = TASK_READY;

    break;
  }
}

static void TaskScheduler::EndTask()
{
  taskList[currTask].state = TASK_NOTASK;
  Yield();
}

static void TaskScheduler::Run()
{
  /* Setup timer 2. */
  SetupTimer2();

  /* Go to first task. */
  SP = taskList[currTask].stackPtr;

  /* Restore registers of first task. */
  asm volatile
  (
    "pop r0\n\t"
    "pop r1\n\t"
    "pop r2\n\t"
    "pop r3\n\t"
    "pop r4\n\t"
    "pop r5\n\t"
    "pop r6\n\t"
    "pop r7\n\t"
    "pop r8\n\t"
    "pop r9\n\t"
    "pop r10\n\t"
    "pop r11\n\t"
    "pop r12\n\t"
    "pop r13\n\t"
    "pop r14\n\t"
    "pop r15\n\t"
    "pop r16\n\t"
    "pop r17\n\t"
    "pop r18\n\t"
    "pop r19\n\t"
    "pop r20\n\t"
    "pop r21\n\t"
    "pop r22\n\t"
    "pop r23\n\t"
    "pop r24\n\t"
    "pop r25\n\t"
    "pop r26\n\t"
    "pop r27\n\t"
    "pop r28\n\t"
    "pop r29\n\t"
    "pop r30\n\t"
    "pop r31\n\t"
    "out __SREG__, r31\n\t"
    "pop r31\n\t"
  ::);
}

static void TaskScheduler::Yield()
{
  /* Reset timer 2. */
  

  /* Save all registers of current task. */
  asm volatile
  (
    "push r31\n\t"
    "in r31, __SREG__\n\t"
    "push r31\n\t"
    "push r30\n\t"
    "push r29\n\t"
    "push r28\n\t"
    "push r27\n\t"
    "push r26\n\t"
    "push r25\n\t"
    "push r24\n\t"
    "push r23\n\t"
    "push r22\n\t"
    "push r21\n\t"
    "push r20\n\t"
    "push r19\n\t"
    "push r18\n\t"
    "push r17\n\t"
    "push r16\n\t"
    "push r15\n\t"
    "push r14\n\t"
    "push r13\n\t"
    "push r12\n\t"
    "push r11\n\t"
    "push r10\n\t"
    "push r9\n\t"
    "push r8\n\t"
    "push r7\n\t"
    "push r6\n\t"
    "push r5\n\t"
    "push r4\n\t"
    "push r3\n\t"
    "push r2\n\t"
    "push r1\n\t"
    "clr r1\n\t"  //compiler expects this
    "push r0\n\t"
  ::);

  register uint16_t stackPtr = SP;
  taskList[currTask].stackPtr = stackPtr;

  NextTask();

  stackPtr = taskList[currTask].stackPtr;
  SP = stackPtr;

  /* Restore registers of next task. */
  asm volatile
  (
    "pop r0\n\t"
    "pop r1\n\t"
    "pop r2\n\t"
    "pop r3\n\t"
    "pop r4\n\t"
    "pop r5\n\t"
    "pop r6\n\t"
    "pop r7\n\t"
    "pop r8\n\t"
    "pop r9\n\t"
    "pop r10\n\t"
    "pop r11\n\t"
    "pop r12\n\t"
    "pop r13\n\t"
    "pop r14\n\t"
    "pop r15\n\t"
    "pop r16\n\t"
    "pop r17\n\t"
    "pop r18\n\t"
    "pop r19\n\t"
    "pop r20\n\t"
    "pop r21\n\t"
    "pop r22\n\t"
    "pop r23\n\t"
    "pop r24\n\t"
    "pop r25\n\t"
    "pop r26\n\t"
    "pop r27\n\t"
    "pop r28\n\t"
    "pop r29\n\t"
    "pop r30\n\t"
    "pop r31\n\t"
    "out __SREG__, r31\n\t"
    "pop r31\n\t"
  ::);
}

static void TaskScheduler::Delay(int ms)
{
  Task *task = &taskList[currTask];

  if (ms <= 0)
  {
    task->delayDurationMs = -1;
    return;
  }

  task->delayStartMs = millis();
  task->delayDurationMs = ms;

  Yield();
}

static void TaskScheduler::SetupTimer2()
{
  noInterrupts();

  // Setup Timer2 overflow to fire every 8ms (125Hz)
  //   period [sec] = (1 / f_clock [sec]) * prescale * (255-count)
  //                  (1/16000000)  * 1024 * (255-130) = .008 sec

  TCCR2B = 0x00;        // Disable Timer2 while we set it up

  TCNT2  = 130;         // Reset Timer Count  (255-130) = execute ev 125-th T/C clock
  TIFR2  = 0x00;        // Timer2 INT Flag Reg: Clear Timer Overflow Flag
  TIMSK2 = 0x01;        // Timer2 INT Reg: Timer2 Overflow Interrupt Enable
  TCCR2A = 0x00;        // Timer2 Control Reg A: Wave Gen Mode normal
  TCCR2B = 0x07;        // Timer2 Control Reg B: Timer Prescaler set to 1024

  interrupts();
}

static void TaskScheduler::NextTask()
{
  while (true)
  {
    Task *task = &taskList[currTask];
    if (task->state == TASK_READY)
    {
      if (task->delayDurationMs > 0)
      {
        if ((millis() - task->delayStartMs) > task->delayDurationMs)
        {
          task->delayDurationMs = -1;
          
          break;
        }

        continue;
      }

      break;
    }

    currTask++;
    if (currTask >= maxTasks)
      currTask = 0;
  }
}

ISR(TIMER2_OVF_vect, ISR_NAKED)
{
  /* Save all registers of current task. */
  asm volatile
  (
    "push r31\n\t"
    "in r31, __SREG__\n\t"
    "push r31\n\t"
    "push r30\n\t"
    "push r29\n\t"
    "push r28\n\t"
    "push r27\n\t"
    "push r26\n\t"
    "push r25\n\t"
    "push r24\n\t"
    "push r23\n\t"
    "push r22\n\t"
    "push r21\n\t"
    "push r20\n\t"
    "push r19\n\t"
    "push r18\n\t"
    "push r17\n\t"
    "push r16\n\t"
    "push r15\n\t"
    "push r14\n\t"
    "push r13\n\t"
    "push r12\n\t"
    "push r11\n\t"
    "push r10\n\t"
    "push r9\n\t"
    "push r8\n\t"
    "push r7\n\t"
    "push r6\n\t"
    "push r5\n\t"
    "push r4\n\t"
    "push r3\n\t"
    "push r2\n\t"
    "push r1\n\t"
    "clr r1\n\t"  //compiler expects this
    "push r0\n\t"
  ::);

  register uint16_t stackPtr = SP;
  taskList[currTask].stackPtr = stackPtr;

  NextTask();

  stackPtr = taskList[currTask].stackPtr;
  SP = stackPtr;

  /* Restore registers of next task. */
  asm volatile
  (
    "pop r0\n\t"
    "pop r1\n\t"
    "pop r2\n\t"
    "pop r3\n\t"
    "pop r4\n\t"
    "pop r5\n\t"
    "pop r6\n\t"
    "pop r7\n\t"
    "pop r8\n\t"
    "pop r9\n\t"
    "pop r10\n\t"
    "pop r11\n\t"
    "pop r12\n\t"
    "pop r13\n\t"
    "pop r14\n\t"
    "pop r15\n\t"
    "pop r16\n\t"
    "pop r17\n\t"
    "pop r18\n\t"
    "pop r19\n\t"
    "pop r20\n\t"
    "pop r21\n\t"
    "pop r22\n\t"
    "pop r23\n\t"
    "pop r24\n\t"
    "pop r25\n\t"
    "pop r26\n\t"
    "pop r27\n\t"
    "pop r28\n\t"
    "pop r29\n\t"
    "pop r30\n\t"
    "pop r31\n\t"
    "out __SREG__, r31\n\t"
    "pop r31\n\t"
    "reti\n\t"
  ::);
}
