#ifndef _FLY_ROUT_H
#define _FLY_ROUT_H

#include "msp_pwm.h"

	#define		ROLL_SET(x) 	(SetSPI((x) * 10 + 1000 + 230, 0))
	#define		PITCH_SET(x) 	(SetSPI((x) * 10 + 1000 + 230, 1))
	#define 	THROTTLE_SET(x)	(SetSPI((x) * 10 + 1000 + 230, 2))
	#define 	YAW_SET(x)		(SetSPI((x) * 10 + 1000 + 230, 3))
	#define 	MODE_SET(x) 	(SetSPI((x) * 10 + 1000 + 230, 4))

#define		FR_ROLL		43
#define		FR_PITCH	53
#define		FR_YAW		51
#define		FR_THROTTLE 72

void Armed(void);
void LiftUp(int throttle, char time, int alt);
void FlyForward(int pitch, char time);
void LiftDown(int throttle, char time);
void Disarmed(void);

#endif // _FLY_ROUT_H