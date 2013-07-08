//*****************************************************************************
//
// Имя файла    : 'cd74ac153.cpp'
// Заголовок    : Драйвер мультиплексора CD74AC153
// Автор        : Барышников Р. А.
// Контакты     : plexus_bra@rambler.ru
// Дата         : 24.04.2013
//
//*****************************************************************************

#include "cd74ac153.h"
#include "gpio.h"

// =============================================================================
//                           Глобальные переменные
// =============================================================================

	CCD74	cd74ac;

// =============================================================================
///
///                            	   Класс CCD74
///
// =============================================================================

// =============================================================================
///
///                            Конструктор класса
///
// =============================================================================

CCD74::CCD74()
{

}

void CCD74::Init (void)
{
	GPIOInit(INPUT_A);
	GPIOInit(INPUT_B);
}

// =============================================================================
///
///                           Переключение лмнмм
///
// =============================================================================
/// \param  line  Номер линии
// =============================================================================

void CCD74::SwitchLine (ESwLine line)
{
	switch (line)
	{
		case LINE1:
		{
			GPIOClr(INPUT_A);
			GPIOClr(INPUT_B);
			break;
		}
		case LINE2:
		{
			GPIOSet(INPUT_A);
			GPIOClr(INPUT_B);
			break;
		}
		case LINE3:
		{
			GPIOClr(INPUT_A);
			GPIOSet(INPUT_B);
			break;
		}
		case LINE4:
		{
			GPIOSet(INPUT_A);
			GPIOSet(INPUT_B);
			break;
		}
	}
}