#ifndef _health
#define _health

#include <Arduino.h>

/** Prints why the device last restarted. Call once in setup(). */
void healthInit();

/** Remembers a task so its stack usage is logged; lets you size the stacks from real numbers. */
void healthWatchTask(const char *name, TaskHandle_t handle);

/**
 * Background task: logs memory, and restarts the device only when it has to:
 *  - the heap stayed low for two minutes (a leak or fragmentation), or
 *  - once a night, if it has been running for a long time.
 */
void healthTask(void *parameter);

#endif
