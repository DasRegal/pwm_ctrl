#include <unistd.h>

#include "flight_routines.h"
#include "msp_pwm.h"
#include "main.h"

int  chan_buf[5];

void Armed(void)
{
	// #define		ROLL_SET(x) 	(chan_buf[0] = x * 10 + 1000 + 230)
	// #define		PITCH_SET(x) 	(chan_buf[1] = x * 10 + 1000 + 230)
	// #define 	THROTTLE_SET(x)	(chan_buf[2] = x * 10 + 1000 + 230)
	// #define 	YAW_SET(x)		(chan_buf[3] = x * 10 + 1000 + 230)
	// #define 	MODE_SET(x) 	(chan_buf[4] = x * 10 + 1000 + 230)


	// SetSPI(FR_ROLL, 0);
	// SetSPI(FR_PITCH, 1);
	// SetSPI(0, 2);
	// SetSPI(FR_YAW, 3);
	// SetSPI(0, 4);
	// SetSPI(100, 3);
	// SetSPI(FR_YAW, 3);
	ROLL_SET(FR_ROLL);
	PITCH_SET(FR_PITCH);
	THROTTLE_SET(0);
	YAW_SET(FR_YAW);
	MODE_SET(0);
	SetSPI2PWM(chan_buf, 5);
	
	Delay(1);
	YAW_SET(100);
	SetSPI2PWM(chan_buf, 5);
	
	Delay(4);
	YAW_SET(FR_YAW);
	SetSPI2PWM(chan_buf, 5);
}

void LiftUp(int throttle, char time, int alt)
{
	THROTTLE_SET(throttle);
	SetSPI2PWM(chan_buf, 5);
	Delay(time);

	// printf("Висим!!!\n");
	MODE_SET(100);
	THROTTLE_SET(alt);
	// printf("Режим.\n");
	SetSPI2PWM(chan_buf, 5);
	Delay(1);
}

void FlyForward(int pitch, char time)
{
	PITCH_SET(pitch);
	SetSPI2PWM(chan_buf, 5);

	Delay(time);

	// PITCH_SET(pitch - FR_PITCH);
	// SetSPI2PWM(chan_buf, 5);
	// Delay(1);
	PITCH_SET(FR_PITCH);
	SetSPI2PWM(chan_buf, 5);
}

void LiftDown(int throttle, char time)
{
	MODE_SET(0);
	SetSPI2PWM(chan_buf, 5);

	THROTTLE_SET(throttle);
	SetSPI2PWM(chan_buf, 5);
	Delay(time);

	// THROTTLE_SET(throttle-10);
	// SetSPI2PWM(chan_buf, 5);
	// Delay(1);

	THROTTLE_SET(throttle+4);
	SetSPI2PWM(chan_buf, 5);
	Delay(1);

	THROTTLE_SET(55);
	SetSPI2PWM(chan_buf, 5);
	Delay(1);

	THROTTLE_SET(0);
	SetSPI2PWM(chan_buf, 5);
}

void Disarmed(void)
{
	THROTTLE_SET(0);
	YAW_SET(0);
	SetSPI2PWM(chan_buf, 5);
	Delay(4);
	YAW_SET(FR_YAW);
	SetSPI2PWM(chan_buf, 5);
}