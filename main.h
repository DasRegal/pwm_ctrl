#ifndef _MAIN_H
#define _MAIN_H

// 27 вывод
#define MISO 88
// 30 вывод
#define MOSI 114
// 18 вывод
#define CLK 115
// 34 вывод
#define CS1	105
// 31 вывод
#define CS2	87

#define		SONAR1	0
#define		R_SONAR	1
#define		F_SONAR	2
#define 	L_SONAR	3
#define 	B_SONAR	4

void Delay		(int time);
int  FlyProfile	(int);

#endif // _MAIN_H