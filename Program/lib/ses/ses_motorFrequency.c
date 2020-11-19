#include "ses_motorFrequency.h"
#include "ses_led.h"
#include "ses_lcd.h"
#include "util/atomic.h"
/* Constants and variables*******************************************/

#define N  9  // Intervals for the Median
#define TEN_MS  0X9C4
#define TIME_CHECK_UPPER_LIMIT 10
#define REV_DIVISION  6

static uint16_t frq= 0;
static uint16_t rev_Counter=0;
static uint16_t median_arr[N];
static uint16_t sort_arr[N];
static bool median_flag=false;

/*FUNCTION DEFINITION ********************************************************/

void motorFrequency_init() {
	//Interrupt Initialization
	EIMSK &= ~(1<<INT0);
	EICRA |= (1<<ISC00);
	EIFR |= (1<<INTF0);
	EIMSK |= (1<<INT0); // or enable all bits 0xff
	//Timer 5 Initialization
	TCCR5B =(1<<WGM52);
	TCCR5B |= (1<<CS51)|(1<<CS50); // prescaler of 64
	TIMSK5 |=(1<<OCIE5A);
	TIFR5 |=(1<<OCF5A);
	OCR5AH |= 0xC3; // for 50000 TICKS
	OCR5AL |=0x50;
}
uint16_t motorFrequency_getRecent() {
	return frq;

}
uint16_t motorFrequency_getMedian() {
	uint16_t temp;
	if(median_flag){
		for(int i=0;i<N;i++) {
			sort_arr[i]=median_arr[i];
		}
		//Sorting
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			for(int i=0;i<N-1;i++) {
				for(int j=0;j<N-i-1;j++) {
					if(sort_arr[j]>sort_arr[j+1]) {
						temp=sort_arr[j];
						sort_arr[j]=sort_arr[j+1];
						sort_arr[j+1]=temp;
					}
				}
			}
			if(N%2==0){
				return sort_arr[(N-1)/2];
			}
			else {
				return ((sort_arr[N/2]+sort_arr[N/2+1])/2);
			}
		}
	}
	else {
		return 0;
	}
	return 0;
}

//function to store frequencies in median array
void time_Lapse() {
	static int cnt=0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		for(int i=0;i<N-1;i++) {
			median_arr[i]= median_arr[i+1];
		}
		median_arr[N-1]=frq;
		cnt++;
		if(cnt==N) {
			median_flag=true;
		}
	}
}

//Interrupt for timer 5
ISR(TIMER5_COMPA_vect) {
	led_greenOn();
	frq=0;
	time_Lapse();
}
//External Interrupt
ISR(INT0_vect) {
	uint16_t clck_time;
	led_yellowToggle();
	led_greenOff();
	static uint8_t rev=0;
	static uint16_t time=0;
	rev++;
	clck_time=(TCNT5H<<8) | TCNT5L;
	time = clck_time/TEN_MS;
	if(rev==REV_DIVISION && (time <=TIME_CHECK_UPPER_LIMIT && time>0)) {
		rev_Counter++;
		frq= 100*rev_Counter/time;//frequency calculation
		time_Lapse();
		rev=0;
	}
	else if (time > TIME_CHECK_UPPER_LIMIT) {
		rev_Counter=0;
		TCNT5H=0;
		TCNT5L=0;
	}
}
