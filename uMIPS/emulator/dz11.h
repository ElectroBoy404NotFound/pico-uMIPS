/*
	(c) 2021 Dmitry Grinberg   https://dmitry.gr
	Non-commercial use only OR licensing@dmitry.gr
*/

#ifndef _DZ11_H_
#define _DZ11_H_

#include <stdbool.h>
#include <stdint.h>

bool dz11init(void);

//feed chars
void dz11charRx(uint8_t line, uint8_t chr);			//will overflow
uint8_t dz11numBytesFreeInRxBuffer(uint8_t lineNo);

//externally provided
extern void dz11charPut(uint8_t line, uint8_t chr);
extern void dz11rxSpaceNowAvail(uint8_t line);				//called when a char is dequeued

#endif
