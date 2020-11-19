#ifndef SES_ROTARY_H_
#define SES_ROTARY_H_

/* INCLUDES ******************************************************************/

#include "ses_common.h"

/* FUNCTION PROTOTYPES *******************************************************/
/*
 * function to initalize the rotary encoder
 */
void rotary_init();

/*
 *check the direction of encoder turn
 */
void check_rotary();

/*
 * Get the state of clockwise turn of rotary
 */
bool rotary_isClockwiseTurned(void);

/*
 *  Get the state of counter clockwise turn of rotary
 */
bool rotary_isCounterClockwiseTurned(void);
typedef void (*pTypeRotaryCallback)();

void rotary_setClockwiseCallback(pTypeRotaryCallback callback);

void rotary_setCounterClockwiseCallback(pTypeRotaryCallback callback);


#endif /* SES_ROTARY_H_ */
