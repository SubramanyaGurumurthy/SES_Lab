#include <stdint.h>
#include <avr/io.h>
#include "ses_adc.h"
#include "ses_uart.h"
#include "util/delay.h"
#include "ses_common.h"
#include "ses_led.h"
#include "ses_timer.h"
#include "ses_button.h"
#include "ses_timer.c"
#include "ses_lcd.h"

static int16_t count = 0;

void softwareTimer(void){
  lcd_clear();
  led_yellowOff();
    lcd_setCursor(0,2);
      fprintf(lcdout,"%d",count);
      _delay_ms(2000);
      lcd_clear();
  led_yellowToggle();
  count ++;
  }


int main(){

led_yellowInit();
led_greenInit();
lcd_init();


while(1){
timer2_start();
timer2_setCallback(&softwareTimer);
 sei();  
}
return 0;
}
