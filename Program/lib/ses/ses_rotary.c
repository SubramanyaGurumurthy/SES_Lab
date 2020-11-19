/* INCLUDES ******************************************************************/
#include "ses_common.h"
#include "ses_rotary.h"
#include "ses_lcd.h"

/* DEFINES & MACROS **********************************************************/


#define ROTARY_ENCODER_SIGNAL_A_PORT	PORTB
#define ROTARY_ENCODER_SIGNAL_A_PIN		5
#define	ROTARY_ENCODER_SIGNAL_B_PORT	PORTG
#define ROTARY_ENCODER_SIGNAL_B_PIN		2
#define ENCODER_NUM_DEBOUNCE_CHECKS		5	
#define MAX_SAMPLE 1220
#define MIN_SAMPLE 120
#define MIN_CAP 800
#define A_HIGH 0
#define A_LOW  1
#define B_HIGH 4
#define B_LOW  5

/* FUNCTION DEFINITION *******************************************************/

void (*cwpt)();
void (*ccwpt)();

void rotary_init(){
	ROTARY_ENCODER_SIGNAL_A_PORT|=(1<<ROTARY_ENCODER_SIGNAL_A_PIN);
	ROTARY_ENCODER_SIGNAL_B_PORT|=(1<<ROTARY_ENCODER_SIGNAL_B_PIN);
	DDR_REGISTER(ROTARY_ENCODER_SIGNAL_A_PORT)&=~(1<<ROTARY_ENCODER_SIGNAL_A_PIN);
	DDR_REGISTER(ROTARY_ENCODER_SIGNAL_B_PORT)&=~(1<<ROTARY_ENCODER_SIGNAL_B_PIN);
}

void check_rotary() {
	uint8_t check_a=0;
	uint8_t check_b=0;
	static uint8_t cw_cnt=0;
	static uint8_t ccw_cnt=0;
	static uint8_t p = 0;
	static bool sampling = false;
	bool a = PIN_REGISTER(ROTARY_ENCODER_SIGNAL_A_PORT) & (1 << ROTARY_ENCODER_SIGNAL_A_PIN);
	bool b = PIN_REGISTER(ROTARY_ENCODER_SIGNAL_B_PORT) & (1 << ROTARY_ENCODER_SIGNAL_B_PIN);
	if (a != b)
		sampling = true;
	if (sampling && p <MAX_SAMPLE) {
		check_a = (a) ? A_HIGH : A_LOW;
		check_b = (b) ? B_HIGH : B_LOW;

		if(check_a==A_LOW && check_b==B_HIGH) {
			cw_cnt++;
		}
		if(check_a==A_HIGH && check_b==B_LOW) {
			ccw_cnt++;
		}
		p++;
		if(p==MIN_SAMPLE) {
			if(cw_cnt>ccw_cnt && cw_cnt>MIN_CAP ) {
				cwpt();
				cw_cnt=0;
				ccw_cnt=0;
			}
			if(ccw_cnt>MIN_CAP && ccw_cnt>cw_cnt) {
				ccwpt();
				cw_cnt=0;
				ccw_cnt=0;
			}
			p=0;
			sampling=false;
			a=1;
			b=1;
		}
	}
}
void rotary_setClockwiseCallback(pTypeRotaryCallback callback) {
	cwpt=callback;

}
void rotary_setCounterClockwiseCallback(pTypeRotaryCallback callback) {
	ccwpt=callback;

}
