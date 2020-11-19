/*INCLUDES ************************************************************/
#include "ses_timer.h"
#include "ses_scheduler.h"
#include "util/atomic.h"
#include"ses_lcd.h"

/* PRIVATE VARIABLES **************************************************/

static taskDescriptor* taskList = NULL;
int single_check=0;
struct time_t uhr;

/*FUNCTION DEFINITION *************************************************/

/*
 * Function to update the scheduler
 */

static void scheduler_update(void) {
	clock_Update();
	taskDescriptor * task_temp;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_temp=taskList;
	}

	while(task_temp!=NULL) {

		if(task_temp->expire>0) {
			task_temp->expire--;
		}
		else if(task_temp->expire==0) {
			task_temp->execute=1;
		}
		task_temp=task_temp->next;
	}
}

void scheduler_init() {
	timer2_start();
	timer2_setCallback(&scheduler_update);

}

void scheduler_run() {
	while(1) {
		taskDescriptor * task_temp;
		ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
			task_temp = taskList;
		}
		while(task_temp!=NULL) {
			if (task_temp->execute==1) {
				if(task_temp->period!=0) {
					ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
						task_temp->expire = task_temp->period;
					}
					task_temp->task(task_temp->param);
					ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
						task_temp->execute=0;
					}
				}
				else if(task_temp->period==0) {
					single_check=1;
				}
			}
			if(single_check==1) {
				scheduler_remove(task_temp);
			}
			task_temp = task_temp->next;

		}
	}
}

bool scheduler_add(taskDescriptor * toAdd) {
	taskDescriptor * task_temp;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_temp=taskList;
	}
	if (toAdd == NULL) {
		return false;
	}
	if (task_temp == NULL) {
		toAdd->next = NULL;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			taskList = toAdd;
		}
		task_temp = taskList;
		return true;
	}
	else {
		while (task_temp != NULL) {
			if (task_temp == toAdd) {
				return false;
			}
			task_temp = task_temp->next;
		}
		toAdd->next = NULL;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			task_temp = taskList;
		}
		while (task_temp->next != NULL) {
			task_temp = task_temp->next;
		}
		task_temp->next = toAdd;
	}
	return true;
}

void scheduler_remove(taskDescriptor * toRemove) {
	taskDescriptor * temp;
	if(taskList->next==NULL) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			taskList=NULL;
			return;
		}
	}
	else if (taskList == toRemove) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			taskList = taskList->next;
			taskList->next = NULL;
			lcd_setCursor(0,3);
			fprintf(lcdout,"Task Removed");
			return;
		}
	}
	else {
		while(taskList->next!=NULL) {
			if((taskList!=toRemove) && (taskList->next==toRemove)) {
				temp = taskList->next;
				taskList->next = temp->next;
				temp->next = NULL;
				return;
			}
			taskList=taskList->next;
		}
	}
}
void clock_Update() {
	uhr.milli++;
	if(uhr.milli>=1000) {
		uhr.second++;
		if(uhr.second>=60) {
			uhr.minute++;
			if(uhr.minute>=60) {
				uhr.hour++;
				if(uhr.hour>=24) {
					uhr.hour=0;
				}
				uhr.minute=0;
			}
			uhr.second=0;
		}
		uhr.milli=0;
	}
}
systemTime_t scheduler_getTime() {
	uint32_t t=0;
	t=(uhr.hour<<8);
	t= (t<<8);
	t=t|(uhr.minute<<8)|uhr.second;
	return t;
}
void scheduler_setTime(systemTime_t time) {
	uhr.second=0;
	uhr.minute=(uint8_t)(time&0xFF);
	uhr.hour=(uint8_t)((time>>8)&0xFF);
}
