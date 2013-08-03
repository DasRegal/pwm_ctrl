#ifndef _FLY_ROUT_H
#define _FLY_ROUT_H

#include "msp_pwm.h"

	#define		ROLL_SET(x) 	(chan_buf[0] = (x) * 10 + 1000 + 230)
	#define		PITCH_SET(x) 	(chan_buf[1] = (x) * 10 + 1000 + 230)
	#define 	THROTTLE_SET(x)	(chan_buf[2] = (x) * 10 + 1000 + 230)
	#define 	YAW_SET(x)		(chan_buf[3] = (x) * 10 + 1000 + 230)
	#define 	MODE_SET(x) 	(chan_buf[4] = (x) * 10 + 1000 + 230)

#define		FR_ROLL		45
#define		FR_PITCH	50
#define		FR_YAW		51

void Armed(void);
void LiftUp(int throttle, char time, int alt);
void FlyForward(int pitch, char time);
void LiftDown(int throttle, char time);
void Disarmed(void);

#endif // _FLY_ROUT_H