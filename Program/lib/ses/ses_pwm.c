# include "ses_pwm.h"


#define PWM_OUTPUTB_TIMER0_PORT  PORTG
#define PWM_OUTPUTB_TIMER0_PIN   5
#define MOTOR_OFF 0XFA

void pwm_init(void) {
	DDR_REGISTER(PWM_OUTPUTB_TIMER0_PORT)|=(1<<PWM_OUTPUTB_TIMER0_PIN);
	OCR0B = MOTOR_OFF;
	PRR0 &= ~(1<<PRTIM0); // ENABLES THE TIMER 0
	//Enable fast PWM
	TCCR0A |= (1<<WGM00) | (1<<WGM01);
	TCCR0B &= ~(1<<WGM02);
	// disabling the prescaler
	TCCR0B |= (1<<CS00);
	//SetTing OC0B to set on compare
	TCCR0A |= (1<<COM0B1)|(1<<COM0B0);
}

void pwm_setDutyCycle(uint8_t dutyCycle) {
	OCR0B = dutyCycle;
}
