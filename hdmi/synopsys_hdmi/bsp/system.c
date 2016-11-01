/*
 * system.c
 *
 *  Created on: Jun 25, 2010
 *      Author: klabadi & dlopo
 * 
 * 
 * 	@note: this file should be re-written to match the environment the 
 * 	API is running on
 */
#ifdef __cplusplus
extern "C"
{
#endif
#include "system.h"
#include "log.h"
#include "error.h"
#include <common.h>

void system_SleepMs(unsigned ms)
{
	udelay(ms * 1000);
}

int system_ThreadCreate(void* handle, thread_t pFunc, void* param)
{
	return FALSE;
}
int system_ThreadDestruct(void* handle)
{
	return FALSE;
}

int system_Start(thread_t thread)
{
	return 0;
}

int system_InterruptDisable(interrupt_id_t id)
{
	return FALSE;
}

int system_InterruptEnable(interrupt_id_t id)
{
	return FALSE;
}

int system_InterruptAcknowledge(interrupt_id_t id)
{
	return FALSE;
}
int system_InterruptHandlerRegister(interrupt_id_t id, handler_t handler,
		void * param)
{
	return FALSE;
}

int system_InterruptHandlerUnregister(interrupt_id_t id)
{
	return FALSE;
}

#ifdef __cplusplus
}
#endif
