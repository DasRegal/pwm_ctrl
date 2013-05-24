//*****************************************************************************
//
// Имя файла    : 'pwm.h'
// Заголовок    : Драйвер для работы с ШИМ
// Автор        : Барышников Р. А.
// Контакты     : plexus_bra@rambler.ru
// Дата         : 23.04.2013
//
//*****************************************************************************

#ifndef _PWM_H
#define _PWM_H

#include <pthread.h>

// =============================================================================
//                                Классы
// =============================================================================
 void * ThreadPWM(void *arg);

class CPWMCtrl
{
	friend  void * ThreadPWM(void *arg);
	protected: 
	pthread_t m_Thread;
	private:
		// Устройство порта
		int m_Prc;
		int m_Chan;
		
	public:
		// Конструктор
		CPWMCtrl();
		
		//int g_Val;
		// Создание сонара
		int Create   (int chan);
		//
		void Setdt(int prc);
		
		void Prnt(void);
		
};

//void * ThreadPWM(void *arg);

#endif // _PWM_H