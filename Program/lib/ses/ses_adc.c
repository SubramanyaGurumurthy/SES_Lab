#include "ses_common.h"
#include "ses_adc.h"
#include "ses_button.h"

#define TEMP_PORT     	PORTF
#define TEMP_PIN         2

#define LIGHTSENS_PORT   PORTF
#define LIGHTSENS_PIN       4

#define JOYSTICK1_PORT   PORTF
#define JOYSTICK1_PIN       5

#define MICROPHONE_PORT   PORTF
#define MICRO_PIN0         0
#define MICRO_PIN1         1

#define ADC_VREF_SRC    REFS0
#define ADC_PRESCALE    ADPS0


#define ADC_TEMP_MAX   	 373
#define ADC_TEMP_MIN 	 273
#define ADC_TEMP_RAW_MAX 1024
#define ADC_TEMP_RAW_MIN  0
#define ADC_TEMP_FACTOR	 0.1

uint16_t ADCL_VAL;

void adc_init(void){
    // Deactivate peripherals
	DDR_REGISTER(TEMP_PORT) &= ~(1 << TEMP_PIN);
	DDR_REGISTER(JOYSTICK1_PORT) &= ~(1<< JOYSTICK1_PIN);
	DDR_REGISTER(LIGHTSENS_PORT) &= ~(1 << LIGHTSENS_PIN);
	TEMP_PORT &= ~(1<<TEMP_PIN);
	JOYSTICK1_PORT &= ~(1<<JOYSTICK1_PIN);
	LIGHTSENS_PORT &= ~(1<<LIGHTSENS_PIN);
	
    // Disable power reduction for the ADC
	PRR0 &= ~(1<<PRADC);
	
    //Voltage configuration
	ADMUX |= (3<<ADC_VREF_SRC);
	
    // Prescale
	ADCSRA |= (3<<ADC_PRESCALE);
	
    // ADC right adjusted
	ADMUX &= ~(1<<ADLAR);
	
    //Disable Auto triggering
	ADCSRA &= ~ (1<< ADATE);
	
    // Enable ADC
	ADCSRA |= (1<<ADEN);

}

uint16_t adc_read(uint8_t adc_channel){

	if(adc_channel == 0x00 || (adc_channel>0x01 && adc_channel<0x09)){
		ADMUX |= (adc_channel);
		}
	else {
		return ADC_INVALID_CHANNEL;
	}

	// start of conversion
	ADCSRA |= (1<<ADSC);
	while(!((ADCSRA>>ADSC)&1));
		ADCL_VAL= ADCL;
		ADCL_VAL= ADCL_VAL| (ADCH<<8);
		return ADCL_VAL;
}

int16_t adc_getTemperature(void){
	uint16_t adc = adc_read(ADC_TEMP_CH); 	
	uint16_t slope = (ADC_TEMP_MAX - ADC_TEMP_MIN) / (ADC_TEMP_RAW_MAX - ADC_TEMP_RAW_MIN);
	uint16_t offset = ADC_TEMP_MAX - (ADC_TEMP_RAW_MAX * slope);
	return (adc * slope + offset) / ADC_TEMP_FACTOR;
}
