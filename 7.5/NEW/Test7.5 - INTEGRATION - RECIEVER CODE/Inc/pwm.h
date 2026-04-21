#ifndef PWM_H
#define PWM_H

#include <stdint.h>

/* Set up hobby servo PWM on PA5 */
void PWM_InitServoPA5(void);

/* Set pulse width directly in microseconds */
void PWM_SetPulseUs(uint16_t pulse_us);

/* Set servo from heading
   0..359 degrees maps to 1000..2000 us */
void PWM_SetFromHeading(uint16_t heading_deg);

#endif
