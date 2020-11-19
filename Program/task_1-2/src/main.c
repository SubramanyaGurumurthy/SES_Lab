#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
void wait(uint16_t millis)
{
uint16_t i; // 16 bit unsigned integer
    for (i = millis; i > 0 ; i--) {
    //prevent code optimization by using inline assembler
    asm volatile ( "nop" ); // one cycle with no operation
        }
}
int main(void) {
	DDRG |= 0x02;
	while (1) {
		wait(3197700);  /*for loop takes 1276 cycles per 256 count 
          //so after multiplication, 3197700 takes 1 sec wait*/
          PORTG ^= 0x02;
	}
	return 0;
}
