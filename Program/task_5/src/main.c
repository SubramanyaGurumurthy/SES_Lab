#include <avr/io.h>
#include <util/delay.h>
#include <ses_lcd.h>
#include <ses_uart.h>
#include "ses_led.h"
#include "ses_adc.h"
#include "ses_pwm.h"
#include "ses_motorFrequency.h"
#include "ses_button.h"
#include "ses_scheduler.h"

/* Constants and variables*******************************************/

#define MOTOR_OFF 0XFA
#define FREQ_TARGET 0x15E
#define A_OMEGA 4
#define PROP_GAIN 6
#define INTEGRAL_GAIN 5
#define DIFF_GAIN 0
#define SCALING_FACTOR 50
#define LED_ROWS_LIMIT 30
#define LED_COLUMN_LIMIT 50
#define MATH_PROP 1000
#define HIGHER_CONTROL_LIMIT 170
#define LOWER_CONTROL_LIMIT 150


uint8_t DUTY_CYCLE =170;
uint16_t frequency;
uint16_t median;
static uint16_t rpm;
static uint8_t motor_flag=0;
static uint8_t plot_flag=0;
bool task_add= false;

/*FUNCTION DEFINITION ********************************************************/

//function to run the motor to a certain duty cycle value
void motor_Start() {
	motor_flag ^= (1<<0);
	led_redToggle();
	if(motor_flag ==1){
		pwm_setDutyCycle(DUTY_CYCLE);
	}
	else {
		pwm_setDutyCycle(MOTOR_OFF);
	}
}

// Function to display the RPM and Median Values
void dispaly() {
	if(plot_flag==0){
		lcd_clear();
	}
	frequency=motorFrequency_getRecent();
	rpm=frequency*60;//Frequency to RPM conversion
	lcd_setCursor(0,0);
	fprintf(lcdout,"RPM:%u", rpm);
	median=motorFrequency_getMedian();
	lcd_setCursor(0,1);
	fprintf(lcdout,"Median:%u",median);
}

//To toggle between plotting and PID control operation
void plot_check(){
	plot_flag ^= (1<<0);
}

// Function to plot the frequency vs time graph
void draw_Plot() {
	static uint8_t plot_cnt=0;
	static uint8_t x=0,y=0;
	if(plot_flag==1) {
		if(plot_cnt<LED_COLUMN_LIMIT) {
			if(x==0){
				lcd_setPixel((LED_ROWS_LIMIT-y),x,1);
			}
			else{
				y=frequency/SCALING_FACTOR;
				lcd_setPixel((LED_ROWS_LIMIT-y),x,1);
			}
			plot_cnt++;
			x=x+2;//to plot values in an interval of 2 pixels
		}
		else {
			lcd_clear();
			plot_cnt=0;
			x=0;
		}
	}
}

// function to control the frequency of the motor using a PID controller
void pid_Control() {
	if(plot_flag==1 && motor_flag==1) {
		static int16_t errorProp=0, errorIntegral=0,errorDifferential=0,minValue=0,input=0;
		errorProp=FREQ_TARGET-frequency;
		if((errorProp+errorIntegral)>A_OMEGA) {
			minValue=A_OMEGA;
		}
		else if(A_OMEGA>(errorProp+errorIntegral)) {
			minValue=(errorProp+errorIntegral);
		}
		if(minValue>(-A_OMEGA)) {
			errorIntegral=minValue;
		}
		else if((-A_OMEGA)>minValue) {
			errorIntegral=(-A_OMEGA);
		}
		input= (PROP_GAIN*errorProp)+(INTEGRAL_GAIN*errorIntegral)+(DIFF_GAIN*(errorDifferential-errorProp));
		errorDifferential=errorProp;
		DUTY_CYCLE=DUTY_CYCLE-(input/MATH_PROP);
		if(DUTY_CYCLE<LOWER_CONTROL_LIMIT) {
			DUTY_CYCLE=LOWER_CONTROL_LIMIT;
			pwm_setDutyCycle(DUTY_CYCLE);
		}
		else if(DUTY_CYCLE>HIGHER_CONTROL_LIMIT) {
			DUTY_CYCLE=HIGHER_CONTROL_LIMIT;
			pwm_setDutyCycle(DUTY_CYCLE);
		}
		else {
			pwm_setDutyCycle(DUTY_CYCLE);
		}
	}
}
int main(void) {
	uart_init(57600);
	button_init(1);
	pwm_init();
	motorFrequency_init();
	scheduler_init();
	lcd_init();
	led_greenInit();
	led_yellowInit();
	led_redInit();
	led_redOff();
	led_greenOff();
	led_yellowOff();

	//scheduling for RPM and Median display///
	taskDescriptor dis_task;
	dis_task.task=dispaly;
	dis_task.param=NULL;
	dis_task.expire=1000;
	dis_task.period=1000;
	task_add=scheduler_add(&dis_task);

	//task for PID controller
	taskDescriptor pid_task;
	pid_task.task=pid_Control;
	pid_task.param=NULL;
	pid_task.expire=1000;
	pid_task.period=1000;
	task_add=scheduler_add(&pid_task);

	// task for plotting the frequency graph
	taskDescriptor plt_task;
	plt_task.task=draw_Plot;
	plt_task.param=NULL;
	plt_task.expire=1000;
	plt_task.period=1000;
	task_add=scheduler_add(&plt_task);

	sei();
	button_setJoystickButtonCallback(&plot_check);// Joystick button to start the plot and to turn on the PID
	button_setRotaryButtonCallback(&motor_Start); // Rotary button to toggle the motor
	if(task_add){
		scheduler_run();
	}
	else {
		fprintf(lcdout,"not added");
	}
	return 0;
}
