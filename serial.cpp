//*****************************************************************************
//
// Имя файла    : 'serial.cpp'
// Заголовок    : Драйвер для работы с COM-портом
// Автор        : Барышников Р. А.
// Контакты     : plexus_bra@rambler.ru
// Дата         : 10.04.2013
//
//*****************************************************************************

#include "serial.h"
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
///
///                            Настройка порта
///
// =============================================================================
/// \param  fd      Устройство
/// \param  speed   Скорость передачи данных
/// \param  parity  Контроль четности
/// \return         Возвращение ошибки
// =============================================================================

int SetInterfaceAttribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf ("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);
    // 8-bit
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    // Игнорирование Break-сигнала
    tty.c_iflag &= ~IGNBRK;
    // Неканонический режим терминального ввода
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    // Чтение не блокируется
    tty.c_cc[VMIN]  = 0;
    // Таймаут чтения 0,5 с.
    tty.c_cc[VTIME] = 5;
    // Отключение xon/xoff контроля
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Включение чтнеия, немодемный контроль
    tty.c_cflag |= (CLOCAL | CREAD);
    // Отключение четности
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

// =============================================================================
///
///                            Установка блокировки
///
// =============================================================================
/// \param  fd           Устройство
/// \param should_block  Блокировка
// =============================================================================

void SetBlocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf ("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf ("error %d setting term attributes", errno);
    }
}