#ifndef __PWM_H__
#define __PWM_H__

enum pwm_id {
	PWM0,
	PWM1,
	PWM2,
	NPWM,
};

int pwm_init(enum pwm_id id, unsigned int hz);
int pwm_set(enum pwm_id id, unsigned int dc);

#endif /* __PWM_H__ */
