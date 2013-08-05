#include <unistd.h>

#include "flight_routines.h"
#include "msp_pwm.h"
#include "main.h"

int  chan_buf[5];

void Armed(void)
{
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