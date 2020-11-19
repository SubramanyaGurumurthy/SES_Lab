#ifndef SES_MOTORFREQUENCY_H_
#define SES_MOTORFREQUENCY_H_

/*INCLUDES *******************************************************************/
#include "ses_common.h"

/*PROTOTYPES-----------------------------------------------------------------*/

/**
 * Initializes the motor
 */
void motorFrequency_init();

/**
 * Function to get recent frequency
 */
uint16_t motorFrequency_getRecent();

/**
 * Function to get the median of frequency values
 */
uint16_t motorFrequency_getMedian();

#endif /* SES_MOTORFREQUENCY_H_ */
