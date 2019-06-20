// bitstream utilities

#include <stdint.h>

uint8_t read_word(const void *stream, uint8_t *start_bit, uint8_t word_length)
{
	uint8_t *streamb = (uint8_t *)stream;
	uint8_t w = 0;
	uint8_t wmask = 1;
	while (word_length--)
	{
		uint8_t byte = *start_bit >> 3;
		uint8_t bit_mask = 1 << (*start_bit & 0x7);
		*start_bit++;

		if (streamb[byte] & mask)
			w |= wmask;
		wmask <<= 1;
	}
}

typedef struct {
	uint8_t	preamble;
	uint8_t	year;
	uint8_t	month;
	uint8_t day;
	uint8_t	hour;
	uint8_t	minute;
	uint8_t	second;
	bool	dst_enable;
	int8_t	timezone_hours;
	uint8_t	checksum;
} CMD_SET_RTC_t;

bool decode_set_rtc(const void *stream)
{
	CMD_SET_RTC_t cmd;

	uint8_t	preamble;
	uint8_t	year;
	uint8_t	month;
	uint8_t day;
	uint8_t	hour;
	uint8_t	minute;
	uint8_t	second;
	bool	dst_enable;
	int8_t	timezone;
	//uint8_t	checksum;

	uint8_t bit = 0;
	preamble	= read_word(stream, &bit, 8);
	year		= read_word(stream, &bit, 7);
	month		= read_word(stream, &bit, 4);
	day			= read_word(stream, &bit, 5);
	hour		= read_word(stream, &bit, 5);
	minute		= read_word(stream, &bit, 6);
	second		= read_word(stream, &bit, 6);
	month		= read_word(stream, &bit, 4);
	dst_enable	= read_word(stream, &bit, 1);
	timezone	= read_word(stream, &bit, 5);
	//bit++;	// padding
	//checksum	= read_word(stream, &bit, 8);
	
	if ((preamble != 0x0F) ||
		(month > 11) ||
		(day > 30) ||
		(hour > 23) ||
		(miunte > 59) ||
		(second > 59) ||
		(timezone < -14) ||
		(timezone > 14))
		return false;

	// check days in month
	
	// checksum
	uint8_t sum = 0;
	for (uint8_t i = 0; i < 6; i++)
		sum ^= *(uint8_t *)stream++;

	if (sum != *(uint8_t *)stream)
		return false;
}
