/* INCLUDES ******************************************************************/
#include "ses_timer.h"
#include "ses_lcd.h"

/* DEFINES & MACROS **********************************************************/
#define TIMER1_CYC_FOR_5MILLISEC    0x4E2
#define TIMER2_CYC_FOR_1MILLISEC	0xF9 

/*FUNCTION DEFINITION ********************************************************/
void(*timer2ptr)();
void(*timer1ptr)();

void timer2_setCallback(pTimerCallback cb) {
	timer2ptr = cb;
}

void timer2_start() {
	//Selecting CTC mode
	TCCR2A |= (1<<WGM21);

	//Selecting the prescaler for 64
	TCCR2B |= (1<<CS22);

	// Enabling the timer and setting the interrupt mask reg
	TIMSK2 |= (1<<OCIE2A);
	
	//Clearing the interrupt flag
	TIFR2 |= (1<<OCF2A);

	//Setting interrupts for every 1 ms
	OCR2A = (TIMER2_CYC_FOR_1MILLISEC);

}


void timer2_stop() {
  TIMSK2 &= ~(1<<OCIE2A);
}

void timer1_setCallback(pTimerCallback cb) {
	timer1ptr = cb;

}


void timer1_start() {
	//Selecting CTC mode
	TCCR1B |= (1<< WGM12);

	// Selecting Prescaler 8 
	TCCR1B |= (1<<CS11);

	//Enabling Interrupt mask register with output compare match A
	TIMSK1 |= (1<<OCIE1A);

	//Enabling interrupt Flag register
	TIFR1 |= OCF1A;

	//Output compare "226 = 0xE2" after calculating it for 8 bit prescaler
	OCR1AH |= 0x04;
	OCR1AL  |= 0xFA;
}


void timer1_stop() {
	TCCR1B &= ~ (1<<CS11);
}

/*interrupt function for timer 1*/
 ISR(TIMER1_COMPA_vect) {
 	timer1ptr();
	
 }

/*Interrupt function for timer 2*/
ISR(TIMER2_COMPA_vect) {
	timer2ptr();
	

}
