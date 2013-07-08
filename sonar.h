//*****************************************************************************
//
// Имя файла    : 'sonar.h'
// Заголовок    : Драйвер для работы с сонаром
// Автор        : Барышников Р. А.
// Контакты     : plexus_bra@rambler.ru
// Дата         : 10.04.2013
//
//*****************************************************************************

#ifndef _SONAR_H
#define _SONAR_H

// =============================================================================
//                                Классы
// =============================================================================

class CSonar
{
	private:
		// Устройство порта
		int	 m_Fd;
	public:
		// Конструктор
		CSonar();
		// Создание сонара
		void Create   (char* portName);
		// Чтение значения сонара
		int ReadValue (void);
};

#endif // _SONAR_H
