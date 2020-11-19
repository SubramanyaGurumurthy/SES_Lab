#ifndef SES_PWM_H_
#define SES_PWM_H_

/*INCLUDES *******************************************************************/
#include "ses_common.h"

/*PROTOTYPES-----------------------------------------------------------------*/

/**
 * Initializes the pulse width modulation mode
 */
void pwm_init(void);

/**
 * Function to run the motor at a required duty cycle
 */
void pwm_setDutyCycle(uint8_t dutyCycle);


#endif /* SES_PWM_H_ */
