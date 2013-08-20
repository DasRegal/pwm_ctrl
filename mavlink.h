#ifndef _MAVLINK_H
#define _MAVLINK_H

void InitMavlink(void);
void CloseMavlink(void);
int serial_wait(void);

extern int g_Compass;
extern int g_Head;

#endif // _MAVLINK_H