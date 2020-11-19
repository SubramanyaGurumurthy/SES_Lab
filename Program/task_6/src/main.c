/*INCLUDES********************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <ses_lcd.h>
#include <ses_uart.h>
#include "ses_led.h"
#include "ses_adc.h"
#include "ses_scheduler.h"
#include "ses_button.h"
#include "ses_rotary.h"


/*Global Declarations*********************************************/

typedef struct fsm_s Fsm; //< typedef for alarm clock state machine
typedef struct event_s Event; //< event type for alarm clock fsm
typedef uint8_t fsmReturnStatus; //< typedef to be used with above enum
typedef fsmReturnStatus (*State)(Fsm *, const Event*);
typedef struct event_s {
	uint8_t signal; //< identifies the type of event
} Event;

static uint8_t min,hour,sec=0;
static bool alarm_updateFlag=false;
static bool task1_add=false;
static bool alarm_act=false;
static uint32_t sys_time=0;
Fsm theFsm;
taskDescriptor task1;
taskDescriptor task2;
taskDescriptor task3;


/** return values */
enum {
	ENTRY,
	EXIT,
	JOYSTICK_PRESSED,
	ROTARY_PRESSED,
	CLOCKWISE,
	COUNTER_CLOCKWISE,
	RET_HANDLED, //< event was handled
	RET_IGNORED, //< event was ignored; not used in this implementation
	RET_TRANSITION //< event was handled and a state transition occurred
};
struct fsm_s {
	State state; //< current state, pointer to event handler
	bool isAlarmEnabled; //< flag for the alarm status
	struct time_t timeSet;//< multi-purpose var for system time and alarm time
};

/*Function Declarations*********************************************/

fsmReturnStatus set_ClockHour(Fsm * fsm, const Event* event);
fsmReturnStatus set_ClockMin(Fsm * fsm, const Event* event);
fsmReturnStatus set_AlarmMin(Fsm * fsm, const Event* event);
fsmReturnStatus set_AlarmHour(Fsm * fsm, const Event* event);
fsmReturnStatus running(Fsm * fsm, const Event* event);
fsmReturnStatus alarm_enable(Fsm * fsm, const Event* event);
void display();
void alarm_check();
void clock_display();
void green_ledTog();

/*FUNCTION DEFINITION*******************************************/

/* dispatches events to state machine, called in application*/
inline static void fsm_dispatch(Fsm* fsm, const Event* event) {

	static Event entryEvent = {.signal = ENTRY};
	static Event exitEvent = {.signal = EXIT};
	State s = fsm->state;
	fsmReturnStatus r = fsm->state(fsm, event);
	if (r==RET_TRANSITION) {
		s(fsm, &exitEvent); //< call exit action of last state
		fsm->state(fsm, &entryEvent); //< call entry action of new state
	}
}

/* sets and calls initial state of state machine */
inline static void fsm_init(Fsm* fsm, State init) {
	Event entryEvent = {.signal = ENTRY};
	fsm->state = set_ClockHour;
	fsm->state(fsm, &entryEvent);
}

/*external event based dispatches*/
static void joystickPressedDispatch(void * param) {
	Event e = {.signal = JOYSTICK_PRESSED};
	fsm_dispatch(&theFsm, &e);
}
static void rotaryPressedDispatch(void * param) {

	Event e = {.signal = ROTARY_PRESSED};
	fsm_dispatch(&theFsm, &e);
}
static void ClockwiseDispatch(void * param) {
	Event e = {.signal = CLOCKWISE};
	fsm_dispatch(&theFsm, &e);
}
static void CounterClockwiseDispatch(void * param) {
	Event e = {.signal = COUNTER_CLOCKWISE};
	fsm_dispatch(&theFsm, &e);
}

/*state to set the system time*/
fsmReturnStatus set_ClockHour(Fsm * fsm, const Event* event) {
	switch(event->signal) {
	case ENTRY:
		lcd_setCursor(0,0);
		fprintf(lcdout,"HH:MM");
		lcd_setCursor(0,1);
		fprintf(lcdout,"Set System Time");
		break;
	case JOYSTICK_PRESSED:
		fsm->state=set_ClockMin;
		return RET_TRANSITION;
	case ROTARY_PRESSED:
		fsm->timeSet.hour++;
		if(fsm->timeSet.hour>=24) {
			fsm->timeSet.hour=0;
			lcd_clear();
		}
		display();
		lcd_setCursor(0,1);
		fprintf(lcdout,"Setting Hours");
		fsm->state=set_ClockHour;
		break;
	case CLOCKWISE:
		fsm->timeSet.hour++;
		if(fsm->timeSet.hour>=24) {
			fsm->timeSet.hour=0;
			lcd_clear();
		}
		display();
		fsm->state=set_ClockHour;
		break;
	case COUNTER_CLOCKWISE:
		fsm->timeSet.hour--;
		if(fsm->timeSet.hour<=0 || fsm->timeSet.hour>23) {
			fsm->timeSet.hour=23;
		}
		display();
		fsm->state=set_ClockHour;
		break;
	case EXIT:
		break;
	default:
		return RET_IGNORED;
	}
	return 0;
}

fsmReturnStatus set_ClockMin(Fsm * fsm, const Event* event) {
	switch(event->signal) {
	case ENTRY:
		break;
	case JOYSTICK_PRESSED:
		fsm->state=running;
		return RET_TRANSITION;
	case ROTARY_PRESSED:
		fsm->timeSet.minute++;
		if(fsm->timeSet.minute>=60) {
			fsm->timeSet.minute=0;
		}
		lcd_setCursor(0,0);
		display();
		lcd_setCursor(0,1);
		fprintf(lcdout,"Setting Minutes");
		fsm->state=set_ClockMin;
		break;
	case CLOCKWISE:
		fsm->timeSet.minute++;
		if(fsm->timeSet.minute>=60) {
			fsm->timeSet.minute=0;
		}
		display();
		fsm->state=set_ClockMin;
		break;
	case COUNTER_CLOCKWISE:
		fsm->timeSet.minute--;
		if(fsm->timeSet.minute<0 || fsm->timeSet.minute>60) {
			fsm->timeSet.minute=60;
		}
		display();
		fsm->state=set_ClockMin;
		break;
	case EXIT:
		sys_time=(fsm->timeSet.hour<<8)|fsm->timeSet.minute;
		scheduler_setTime(sys_time);

		//task for toggling green LED when clock starts
		task1.task=green_ledTog;
		task1.param=NULL;
		task1.expire=1000;
		task1.period=1000;
		task1_add=scheduler_add(&task1);

		//task to start the system clock
		task2.task=clock_display;
		task2.param=NULL;
		task2.expire=1000;
		task2.period=1000;
		scheduler_add(&task2);
		break;
	default:
		return RET_IGNORED;
	}
	return 0;
}

/*state to set the alarm time*/
fsmReturnStatus set_AlarmHour(Fsm * fsm, const Event* event) {
	switch(event->signal) {
	case ENTRY:
		fsm->timeSet.hour=0;
		fsm->timeSet.minute=0;
		alarm_updateFlag=true;
		lcd_clear();
		lcd_setCursor(0,0);
		fprintf(lcdout,"00:00");
		lcd_setCursor(0,1);
		fprintf(lcdout,"Set Hour");
		break;
	case JOYSTICK_PRESSED:
		fsm->state=set_AlarmMin;
		return RET_TRANSITION;
	case ROTARY_PRESSED:
		fsm->timeSet.hour++;
		if(fsm->timeSet.hour>=24) {
			fsm->timeSet.hour=0;
			lcd_clear();
		}
		lcd_setCursor(0,0);
		display();
		lcd_setCursor(0,1);
		fprintf(lcdout,"Set Hours");
		fsm->state=set_AlarmHour;
		break;
	case CLOCKWISE:
		fsm->timeSet.hour++;
		if(fsm->timeSet.hour>=24) {
			fsm->timeSet.hour=0;
			lcd_clear();
		}
		display();
		fsm->state=set_AlarmHour;
		break;
	case COUNTER_CLOCKWISE:
		fsm->timeSet.hour--;
		if(fsm->timeSet.hour<=0 || fsm->timeSet.hour>23) {
			fsm->timeSet.hour=23;
		}
		display();
		fsm->state=set_AlarmHour;
		break;
	case EXIT:
		break;
	default:
		return RET_IGNORED;
	}
	return 0;
}

fsmReturnStatus set_AlarmMin(Fsm * fsm, const Event* event) {
	switch(event->signal) {
	case ENTRY:
		lcd_clear();
		lcd_setCursor(0,0);
		fprintf(lcdout,"%d,%d",fsm->timeSet.hour,fsm->timeSet.minute);
		lcd_setCursor(0,1);
		fprintf(lcdout,"Set Minutes");
		break;
	case JOYSTICK_PRESSED:
		fsm->state=running;
		return RET_TRANSITION;
	case ROTARY_PRESSED:
		fsm->timeSet.minute++;
		if(fsm->timeSet.minute>=60) {
			lcd_clear();
			fsm->timeSet.minute=0;
		}
		lcd_setCursor(0,0);
		display();
		lcd_setCursor(0,1);
		fprintf(lcdout,"Set Minutes");
		fsm->state=set_AlarmMin;
		break;
	case CLOCKWISE:
		fsm->timeSet.minute++;
		if(fsm->timeSet.minute>=60) {
			fsm->timeSet.minute=0;
			lcd_clear();
		}
		display();
		fsm->state=set_AlarmMin;
		break;
	case COUNTER_CLOCKWISE:
		fsm->timeSet.minute--;
		if(fsm->timeSet.minute<0 || fsm->timeSet.minute>59) {
			fsm->timeSet.hour=59;
		}
		display();
		fsm->state=set_AlarmMin;
		break;
	case EXIT:
		alarm_updateFlag=false;

		//task to toggle LEDs at Exit event of Alarm
		task3.task=alarm_check;
		task3.param=NULL;
		task3.expire=250;
		task3.period=250;
		scheduler_add(&task3);
		break;
	default:
		return RET_IGNORED;
	}
	return 0;
}

/*state to run the system time*/
fsmReturnStatus running(Fsm * fsm, const Event* event) {
	switch(event->signal) {
	case ENTRY:
		fsm->isAlarmEnabled=false;
		led_yellowOff();
		led_redOff();
		break;
	case JOYSTICK_PRESSED:
		fsm->state=set_AlarmHour;
		return RET_TRANSITION;

	case ROTARY_PRESSED:
		fsm->state=alarm_enable;
		return RET_TRANSITION;
		break;
	case EXIT:
		break;
	default:
		return RET_IGNORED;
	}
	return 0;
}

/*state to enable the alarm*/
fsmReturnStatus alarm_enable(Fsm * fsm, const Event* event) {
	switch(event->signal) {
	case ENTRY:
		fsm->isAlarmEnabled=true;
		led_yellowOn();
		fsm->state=alarm_enable;
		break;
	case JOYSTICK_PRESSED:
		if(alarm_act==true) {
			fsm->state=running;
			return RET_TRANSITION;
		}
		else {
			break;
		}
	case ROTARY_PRESSED:
		fsm->state=running;
		return RET_TRANSITION;
	case EXIT:
		break;
	default:
		return RET_IGNORED;
	}
	return 0;
}

/*function to display the clock setting*/
void display() {
	lcd_clear();
	lcd_setCursor(0,0);
	fprintf(lcdout,"%d:%d",theFsm.timeSet.hour,theFsm.timeSet.minute);
}

/*clock display function*/
void clock_display() {

	uint32_t time=0;
	time=scheduler_getTime();
	if(alarm_updateFlag==false) {
		sec=(uint8_t)(time&0xFF);
		min=(uint8_t)((time>>8)&0xFF);
		hour=(uint8_t)((time>>16)&0xFF);//to store the time in 32 bit integer
		if(sec==0||min==0||hour==0) {
			lcd_clear();
		}
		lcd_setCursor(0,0);
		fprintf(lcdout,"%d:%d:%d",hour,min,sec);
	}
}

/*function to toggle the LEDs according to ringing of alarm*/
void alarm_check() {
	static uint8_t ctr=0;
	if(theFsm.isAlarmEnabled && (hour==theFsm.timeSet.hour)&& (min==theFsm.timeSet.minute) && alarm_updateFlag==false ) {
		ctr++;
		alarm_act=true;
		if(ctr<=20) {
			led_redToggle();
		}
		else {
			theFsm.state=running;
			alarm_act=false;
			ctr=0;
			Event e = {.signal = ENTRY};
			fsm_dispatch(&theFsm, &e);
		}
	}
}
//Function to toggle green led
void green_ledTog() {
	led_greenToggle();
}

int main(void) {
	uart_init(57600);
	lcd_init();
	button_init(1);
	led_redInit();
	led_redOff();
	led_yellowInit();
	led_yellowOff();
	led_greenInit();
	led_greenOff();
	scheduler_init();
	rotary_init();

	rotary_setClockwiseCallback(&ClockwiseDispatch);//callback to check clockwise turn of encoder
	rotary_setCounterClockwiseCallback(&CounterClockwiseDispatch);//callback to check counter clockwise turn of encoder
	button_setJoystickButtonCallback(&joystickPressedDispatch);//callback to check the rotary button press event
	button_setRotaryButtonCallback(&rotaryPressedDispatch);//callback to check the joystick button press event
	sei();
	fsm_init(&theFsm,NULL);
	while(1) {
		check_rotary();
		if(task1_add) {
			scheduler_run();
		}
	}
	return 0;
}



