
#include "ses_button.h"
#include "ses_timer.h"
#include "ses_lcd.h"


#define BUTTON_PORT                    PORTB
#define BUTTON_PIN                      6

#define JOYSTICK_PORT                  PORTB
#define JOYSTICK_PIN                    7

#define BUTTON_NUM_DEBOUNCE_CHECKS       5
#define BUTTON_DEBOUNCE_POS_JOYSTICK    0x01
#define BUTTON_DEBOUNCE_POS_ROTARY      0x02

// Function definition
void(*rotary)();
void(*joystick)();

/*Function to Initialize the buttons, also the interrupt is enabled*/
void button_init(bool debouncing){
    
    //Initializing the buttons
    DDR_REGISTER(BUTTON_PORT)&= ~ (1<<BUTTON_PIN);
    BUTTON_PORT |= (1<<BUTTON_PIN);

    DDR_REGISTER(JOYSTICK_PORT) &= ~ (1<<JOYSTICK_PIN);
    JOYSTICK_PORT |= (1<<JOYSTICK_PIN);

    if(debouncing){
        //Enabling interrupt
        PCICR |=(1<<0x01);
        PCMSK0 |= (1<<BUTTON_PIN) || (1<<JOYSTICK_PIN);
        timer1_setCallback(&button_checkState);
        }
}

/*Implementing the interrupt service routine
 To check whether one of the button values changed.
 To execute the appropriate button callback function.
 And making sure that a button callback is only executed if a valid callback was set and the mask register
contains a 1.*/

ISR (PCINT0_vect){
    if(button_isJoystickPressed()){
        joystick();
	}
	else if(button_isRotaryPressed()){
		rotary();
    }
}

/* Function To check if the joystick is pressed*/
bool button_isJoystickPressed(void){
    if(((PIN_REGISTER(JOYSTICK_PORT) >> JOYSTICK_PIN)&1)==0){
        return true;
    }
    else {
        return false;
    }
}

/* Function To check if rotary button is pressed */ 
bool button_isRotaryPressed(void){
    if(((PIN_REGISTER(BUTTON_PORT) >> BUTTON_PIN)&1) == 0){
        return true;
    }
    else{
        return false;
    }
}


/*Funtion to check the state of the button*/
void button_checkState(void) {
    static uint8_t state[BUTTON_NUM_DEBOUNCE_CHECKS] = {};
    static uint8_t index = 0;
    static uint8_t debouncedState = 0;
    uint8_t lastDebouncedState = debouncedState;

    // each bit in every state byte represents one button
    state[index] = 0;
        if(button_isJoystickPressed()) {
            state[index] |= BUTTON_DEBOUNCE_POS_JOYSTICK;
            lastDebouncedState = 1;
            }
        if(button_isRotaryPressed()) {
            state[index] |= BUTTON_DEBOUNCE_POS_ROTARY;
            lastDebouncedState = 2;
            }
    index++;
    
        if (index == BUTTON_NUM_DEBOUNCE_CHECKS) {
            index = 0;
            }

    // init compare value and compare with ALL reads, only if
    // we read BUTTON_NUM_DEBOUNCE_CHECKS consistent "1's" in the state
    // array, the button at this position is considered pressed

    uint8_t j = 0xFF;
        for(uint8_t i = 0; i < BUTTON_NUM_DEBOUNCE_CHECKS; i++) {
                j = j & state[i];
            }
    lastDebouncedState = state[index-1];
    debouncedState = j;
    
       if(index == 4){
           if((debouncedState & BUTTON_DEBOUNCE_POS_JOYSTICK) && (lastDebouncedState & BUTTON_DEBOUNCE_POS_JOYSTICK)){
                joystick();
                }

            else if((debouncedState & BUTTON_DEBOUNCE_POS_ROTARY)&&(lastDebouncedState & BUTTON_DEBOUNCE_POS_ROTARY)){
                rotary();
                }
        }
}

/* callback function for button */
void button_setRotaryButtonCallback(pButtonCallback callback){
	 rotary = callback;
}

/* callback function for joystick */
void button_setJoystickButtonCallback(pButtonCallback callback){
	joystick = callback;
}


 