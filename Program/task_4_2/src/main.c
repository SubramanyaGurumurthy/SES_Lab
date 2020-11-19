/*INCLUDES********************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <ses_lcd.h>
#include <ses_uart.h>
#include "ses_led.h"
#include "ses_adc.h"
#include "ses_scheduler.h"
#include "ses_button.h"

/*Global Declarations*********************************************/

enum led{red, yellow, green};
static int count;
void stop_clock(void);
static uint8_t clk_tog = 0x00;
bool t1=false;
bool t2=false;
bool t3=false;
bool t4=false;

/*Function Declarations*******************************************/


/*
 * Task to select the color to toggle
 */
void task_1(void* choice) {
	int color;
	color=(int)choice;
	switch(color){
	case red:
		led_redToggle();
		break;
	case yellow:
		led_yellowToggle();
		break;
	case green:
		led_greenToggle();
		break;
	}
}

/*
 * Task to call button check state every 5 ms.
 */

void task_2() {
	button_checkState();
}

/*
 * Task to toggle the yellow Led when pressing the Joystick button
 *
 */
void task_3(){

	button_setJoystickButtonCallback(&led_yellowToggle);
	if(count==5)
	{
		led_yellowOff();
		count=0;
	}
	else
	{
		count++;
	}
	if(button_isJoystickPressed())
	{
		count=0;
	}
}

/*
 * Task to implement a stopwatch
 */
void task_4() {
	//button_init(0);
	static int timer=0;
	static int sec=0;
	static int msec=0;
	lcd_setCursor(0,0);
	fprintf(lcdout,"Time:- %d:%d",sec,msec);
	button_setRotaryButtonCallback(&stop_clock);
	if(clk_tog==1) {
		timer=timer+10;
		sec=timer/100;
		msec=timer%100;
	}
}
void stop_clock(void){
	clk_tog ^= (1<<0);
}

int main(void) {
	uart_init(57600);
	lcd_init();
	adc_init();
	button_init(0);
	led_redInit();
	led_redOff();
	led_yellowInit();
	led_yellowOff();
	led_greenInit();
	led_greenOff();
	scheduler_init();

	// task 1 description
	taskDescriptor task1;
	task1.task=task_1;
	task1.param= (int*)green;
	task1.expire= 2000;
	task1.period=2000;

	//task2 description
	taskDescriptor task2;
	task2.task=task_2;
	task2.param=NULL;
	task2.expire=5;
	task2.period=5;

	//task3 description
	taskDescriptor task3;
	task3.task=task_3;
	task3.param=NULL;
	task3.expire=1000;
	task3.period=1000;

	//task4 description
	taskDescriptor task4;
	task4.task=task_4;
	task4.param=NULL;
	task4.expire=100;
	task4.period=100;

	t1=scheduler_add(&task1);
	t2=scheduler_add(&task2);
	t3=scheduler_add(&task3);
	t4=scheduler_add(&task4);
	sei();

	if(t1&& t2 && t3 && t4) {
		scheduler_run();
	}
	else {
		lcd_setCursor(0,3);
		fprintf(lcdout,"Task Not Added");
	}
	return 0;
}





