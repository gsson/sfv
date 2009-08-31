#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>
#include <stdlib.h>

uint32_t crc32(void * data, uint32_t crc, size_t length);
uint32_t crc32_file(const char *path);

#endif /*CRC_H_*/
