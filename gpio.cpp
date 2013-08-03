//*****************************************************************************
//
// Имя файла    : 'gpio.cpp'
// Заголовок    : Драйвер GPIO
// Автор        : Барышников Р. А.
// Контакты     : plexus_bra@rambler.ru
// Дата         : 24.04.2013
//
//*****************************************************************************

#include "gpio.h"
#include <stdio.h>
#include <unistd.h>

// =============================================================================
///
///		                            Инициализация
///
// =============================================================================

void GPIOInit (int gpio)
{
	FILE* fp;
	char gpioFileName[256];

	fp = NULL;

	// Запись в файл export номеров gpio
	if ((fp = fopen("/sys/class/gpio/export", "ab")) != NULL)
	{
		rewind(fp);
		fprintf(fp, "%d", gpio);
		fclose(fp);

		//printf("gpio%d exported\n", gpio);
	}
	else
	{
		//printf("Error exported gpio%d\n", gpio);
	}

	// Конфигурация gpio№ как OUT...
	sprintf(gpioFileName, "/sys/class/gpio/gpio%d/direction", gpio);
	if ((fp = fopen(gpioFileName, "rb+")) != NULL)
	{
		rewind(fp);
		fprintf(fp, "out");
		fclose(fp);

		//printf("gpio%d configured for OUTPUT... ", gpio;
		
		// ... и установка линий в "0"
		sprintf(gpioFileName, "/sys/class/gpio/gpio%d/value", gpio);
		if ((fp = fopen(gpioFileName, "rb+")) != NULL)
		{
			fprintf(fp, "0");
			fclose(fp);
			//printf("Value set to \"0\"\n");
		}
		else
		{
			//printf("Error value\n");
		}
	}
	else
	{
		//printf("Error configured gpio%d\n", gpio);
	}
}

// =============================================================================
///
///		                    Установка линии в "1"
///
// =============================================================================
///	\param   gpio  Номер линии
// =============================================================================

void GPIOSet  (int gpio)
{
	FILE* fp;
	fp = NULL;
	char gpioFileName[256];

	sprintf(gpioFileName, "/sys/class/gpio/gpio%d/value", gpio);
	if ((fp = fopen(gpioFileName, "rb+")) != NULL)
	{
		rewind(fp);
		fprintf(fp, "1");
		fclose(fp);
	}
}

// =============================================================================
///
///		                    Установка линии в "0"
///
// =============================================================================
///	\param   gpio  Номер линии
// =============================================================================

void GPIOClr  (int gpio)
{
	FILE* fp;
	fp = NULL;
	char gpioFileName[256];

	sprintf(gpioFileName, "/sys/class/gpio/gpio%d/value", gpio);
	if ((fp = fopen(gpioFileName, "rb+")) != NULL)
	{
		rewind(fp);
		fprintf(fp, "0");
		fclose(fp);
	}
}

// =============================================================================
///
///		                    Направление линии
///
// =============================================================================
///	\param   dir  Направление линии
// =============================================================================

void GPIODir (int gpio, EGPIODir dir)
{
	FILE* fp;
	fp = NULL;
	char gpioFileName[256];

	sprintf(gpioFileName, "/sys/class/gpio/gpio%d/direction", gpio);
	if ((fp = fopen(gpioFileName, "rb+")) != NULL)
	{
		rewind(fp);
		if (dir == DIR_IN) fprintf(fp, "in");
		else fprintf(fp, "out");
		fclose(fp);
	}
}

// =============================================================================
///
///		                    Проверка уровня линии
///
// =============================================================================
///	\param   gpio  Номер линии
/// \return  	   Уровень линии
// =============================================================================

int GPIOGet (int gpio)
{
	FILE* fp;
	fp = NULL;
	char gpioFileName[256];
	// char *buf[1];
	char val;

	sprintf(gpioFileName, "/sys/class/gpio/gpio%d/value", gpio);
	if ((fp = fopen(gpioFileName, "rb+")) != NULL)
	{
		rewind(fp);
		fread(&val, sizeof(char), 1, fp);
		fclose(fp);
		if (val == '0') return 0;
		else return 1;
	}
}