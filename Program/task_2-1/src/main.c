#include <avr/io.h>
#include <util/delay.h>
#include <ses_lcd.h>
#include <ses_uart.h>
#include "ses_led.c"
#include "ses_adc.c"


	/*int main(void){
		uart_init(57600);
		fprintf(uartout, "START");

		while(1){
			fprintf(lcdout, "START\t");
		}

		return 0;
	}*/
int main(void){
	uart_init(57600);
	lcd_init();
	adc_init();
	button_init(0);
	led_redInit();
	led_redOff();
	led_greenInit();
	led_greenOff();
	fprintf(uartout, "START");
	char num=0;
	while(1){
		lcd_setCursor(0x00, 0x00);
		fprintf(lcdout,"Start:  %d",num);
		_delay_ms(800);
		num++;
		if(button_isJoystickPressed()==true && button_isRotaryPressed()==true ){
			led_redOn();
			led_greenOn();
		}
		else if(button_isRotaryPressed()==true){
			led_greenOn();
		}
		else if(button_isJoystickPressed()==true){
			led_redOn();
		}
		else {
			led_greenOff();
			led_redOff();
		}
		lcd_clear();
		int result = adc_read(0x02);
		lcd_setCursor(0x00,0x01);
		fprintf(lcdout,"number: %d",result);
	}
	return 0;
}



