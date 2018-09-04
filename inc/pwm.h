#ifndef __PWM_H__
#define __PWM_H__

int pwm_count();
int pwm_init(int id, unsigned int hz);
int pwm_init_all(unsigned int hz);
int pwm_set(int id, unsigned int dc);

#endif /* __PWM_H__ */
