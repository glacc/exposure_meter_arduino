/* 

  Preemptive Task Scheduler by Glacc

  Based on AVR-context

*/

#include <avr/wdt.h>
#include <avr/sleep.h>

#include "TaskScheduler.hpp"

/*
using namespace TaskScheduler;

static Task *TaskScheduler::taskList = nullptr;
static uint8_t TaskScheduler::maxTasks;
static uint8_t TaskScheduler::currTask;

extern "C"
{
  static avr_context_t *volatile currContext;
}
static avr_context_t TaskScheduler::dummyContext;

// static uint16_t TaskScheduler::mainRoutineStackPtr;
*/

static Task *taskList = nullptr;
static uint8_t maxTasks;
static uint8_t currTask = 0;

extern "C"
{
  avr_context_t *volatile TaskScheduler_currContext;
}
static avr_context_t dummyContext;

static void TaskScheduler::Init(Task *initialTaskList, uint8_t initialMaxTasks)
{
  taskList = initialTaskList;
  maxTasks = initialMaxTasks;

  avr_getcontext(&dummyContext);
}

static void TaskScheduler::AddTask(void (*taskFunc)(), void *stackAddr, uint16_t stackSize)
{
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
  ReSetupSystemTimer();
}

__attribute__((naked)) static void TaskScheduler::Yield()
{
  asm volatile
  (
    "ret\n"
  );
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

static void TaskScheduler::ReSetupSystemTimer()
{
  // /*
  // disable interrupts
  cli();

  MCUSR &= ~(1<<WDRF);

  // reset watchdog timer
  wdt_reset();

  // configure WD timer //
  // enable WD timer configuration mode
  WDTCSR |= 1 << WDCE | 1 << WDE;
  // reset WD timer
  WDTCSR = 0;
  // configure period
  wdt_enable(WDTO_15MS);
  // use WD timer in interrupt mode
  WDTCSR |= 1 << WDIE;

  // enable interrupts
  sei(); 
  // */

  /*
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
  */
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

          TaskScheduler_currContext = &task->context;

          break;
        }

        continue;
      }

      TaskScheduler_currContext = &task->context;

      break;
    }

    currTask++;
    if (currTask >= maxTasks)
      currTask = 0;
  }
}

ISR(WDT_vect, ISR_NAKED)
// ISR(TIMER2_OVF_vect, ISR_NAKED)
{
  // save the context of the current task
  AVR_SAVE_CONTEXT_GLOBAL_POINTER(
      // "cli\n", // disable interrupts during task switching
      "\n",
      TaskScheduler_currContext);

  // switch to the other task.
  TaskScheduler::NextTask();
  
  // /*
  // re-enable watchdog timer interrupts to avoid reset
  WDTCSR |= 1 << WDIE;
  // */

  /*
  TIFR2 = 0x00;
  */

  // restore the context of the task to which we have just switched.
  AVR_RESTORE_CONTEXT_GLOBAL_POINTER(TaskScheduler_currContext);

  // return from the interrupt and activate the restored context.
  asm volatile
  (
    "reti\n"
  );
}
