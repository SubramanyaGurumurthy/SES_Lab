#include "ses_lcd.h" 
#include "ses_uart.h"
#include "util/delay.h"
#include "ses_button.h"
#include "ses_common.h"
#include "ses_led.h".
#include "ses_led.c"
#include "ses_button.c"
#include "ses_adc.h"
#include "ses_adc.c"

int main(){
 button_init();
uart_init(57600);
lcd_init();
adc_init();

while(1){
  bool deci_button = button_isRotaryPressed();
  bool deci_joystick = button_isJoystickPressed();

if ( deci_button == true){
    led_redInit();
    led_redOn();
    _delay_ms(2000);
    led_redOff();
}

if (deci_joystick == true){
 led_yellowInit();
   led_yellowOn();
    _delay_ms(2000);
    led_yellowOff();
int16_t temp = adc_getTemperature();
  _delay_ms(2500);
 lcd_clear();
 fprintf(uartout, "%d degrees\n",temp);
 fprintf(lcdout, "%d degrees\n",temp);

}

}
}
