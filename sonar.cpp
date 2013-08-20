//*****************************************************************************
//
// Имя файла    : 'sonar.cpp'
// Заголовок    : Драйвер для работы с сонаром
// Автор        : Барышников Р. А.
// Контакты     : plexus_bra@rambler.ru
// Дата         : 10.04.2013
//
//*****************************************************************************

#include "sonar.h"
#include "serial.h"
#include "spi.h"
#include "main.h"

#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

	CSonar sonar;

// =============================================================================
//                                 Класс CSonar
// =============================================================================

// =============================================================================
///
///                                Конструктор
///
// =============================================================================

CSonar::CSonar()
{
	// SPIInitCS(CS2);
	m_CS = spi.InitCS(CS2);
	printf("CS SONAR %d\n", m_CS);
}

// =============================================================================
///
///                            Настройка порта
///
// =============================================================================
/// \param  portName  Имя устройства
// =============================================================================

void CSonar::Create(char* portName)
{
	char dev[256];
	sprintf(dev, "/dev/%s", portName);

	m_Fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
	if (m_Fd < 0)
	{
		printf("error %d opening %s", errno, portName);
		return;
	}
	SetInterfaceAttribs(m_Fd, B9600, 0);
	SetBlocking(m_Fd, 0);
}

// =============================================================================
///
///                        Чтение значения сонара
///
// =============================================================================
/// \return Значение в мм.
// =============================================================================
/// На com-порт приходит строчка вида "Rxxxx", где xxxx - расстояние в мм.
// =============================================================================

int CSonar::ReadValue(void)
{
	char buf[5];
	while (1)
	{
		read (m_Fd, buf, 1);
		if( buf[0] == 'R')
			break;
	}
	read (m_Fd, buf, 4);
	buf[4] = 0;
	return atoi(buf);
}

void CSonar::SPIInitCS(int line)
{
	m_CS = spi.InitCS(line);
}

int CSonar::ReadSonar(int line)
{
	int val;
	spi.ClrCS(m_CS);
	spi.WriteByte(line);
	val = spi.ReadByte();
	spi.SetCS(m_CS);

	float k;
	if (line == SONAR1) val *= 5.8;
	else val = val * 1.55 + 2.4;
	
	return val;
}