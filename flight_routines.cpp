#include <unistd.h>
#include <stdio.h>
#include "flight_routines.h"
#include "msp_pwm.h"
#include "main.h"
#include "sonar.h"

int  chan_buf[5];

void Armed(void)
{
	ROLL_SET(FR_ROLL);
	PITCH_SET(FR_PITCH);
	THROTTLE_SET(0);
	YAW_SET(FR_YAW);
	MODE_SET(0);
	//SetSPI2PWM(chan_buf, 5);
	
	Delay(1);
	YAW_SET(100);
	//SetSPI2PWM(chan_buf, 5);
	
	Delay(4);
	YAW_SET(FR_YAW);
	//SetSPI2PWM(chan_buf, 5);
}

void LiftUp(int throttle, char time, int alt)
{
	// THROTTLE_SET(40);
	// Delay(1);
	// for (int i = 0; i < 500; ++i)
	// {
	// 	usleep(1000);
	// }
	// THROTTLE_SET(throttle);
	// Delay(time);
		
	MODE_SET(100);
	printf("Режим.\n");
	THROTTLE_SET(alt);
	// Delay(1);
}

void FlyForward(int pitch, char time)
{
	PITCH_SET(pitch);
	//SetSPI2PWM(chan_buf, 5);

	Delay(time);

	// PITCH_SET(pitch - FR_PITCH);
	// //SetSPI2PWM(chan_buf, 5);
	// Delay(1);
	PITCH_SET(FR_PITCH);
	//SetSPI2PWM(chan_buf, 5);
}

void LiftDown(int throttle, char time)
{
	MODE_SET(0);

	while(throttle > 0)
	{

		throttle -= 7;
		if (throttle < 40 )
		{
			throttle = 0;
			THROTTLE_SET(throttle);
			break;
		}
		THROTTLE_SET(throttle);
		for (int i = 0; i < 100; ++i)
		{
			usleep(1000);
		}
		throttle += 7;
		THROTTLE_SET(throttle);
		for (int i = 0; i < 100; ++i)
		{
			usleep(1000);
		}
		throttle -= 1;
		if (throttle < 40 )
		{
			throttle = 0;
			THROTTLE_SET(throttle);
			break;
		}
	}

	// THROTTLE_SET(throttle);
	// Delay(time);

	// THROTTLE_SET(throttle+4);
	// Delay(1);

	THROTTLE_SET(0);
}

void Disarmed(void)
{
	THROTTLE_SET(0);
	YAW_SET(0);
	//SetSPI2PWM(chan_buf, 5);
	Delay(4);
	YAW_SET(FR_YAW);
	//SetSPI2PWM(chan_buf, 5);
}