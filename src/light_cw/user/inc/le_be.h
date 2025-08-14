#ifndef _LE_BE_H_
#define _LE_BE_H_

#include <stdint.h>

uint16_t get_le_word(uint8_t *msg);
uint32_t get_le_dword(uint8_t *msg);
void set_le_word(uint8_t *msg, uint16_t value);
void set_le_dword(uint8_t *msg, uint32_t value);
uint16_t get_be_word(uint8_t *msg);
uint32_t get_be_dword(uint8_t *msg);
void set_be_word(uint8_t *msg, uint16_t value);
void set_be_dword(uint8_t *msg, uint32_t value);

#endif
