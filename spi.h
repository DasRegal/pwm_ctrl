//*****************************************************************************
//
// Имя файла    : 'spi.h'
// Заголовок    : Драйвер SPI
// Автор        : Барышников Р. А.
// Контакты     : plexus_bra@rambler.ru
// Дата         : 26.06.2013
//
//*****************************************************************************

#ifndef _SPI_H
#define _SPI_H

// =============================================================================
//                                Классы
// =============================================================================

class CSPI
{
	private:
		int m_MISO;
		int m_MOSI;
		int m_CLK;
		int m_us;
		
	public:
		// Конструктор
		CSPI();
		void Init 		(int miso, int mosi, int clk);
		void WriteByte	(int8_t byte);
		void SetDelay	(int us);
};

#endif // _SPI_H