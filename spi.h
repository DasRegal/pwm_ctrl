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

#define		N_CS	10

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
		int m_CS[N_CS];
		
	public:
		// Конструктор
		CSPI();
		void Init 		(int miso, int mosi, int clk);
		void WriteByte	(char byte);
		char ReadByte	(void);
		void SetDelay	(int us);
		int  InitCS		(int pin);
		void SetCS		(int index);
		void ClrCS		(int index);
};

extern	CSPI spi;

#endif // _SPI_H
