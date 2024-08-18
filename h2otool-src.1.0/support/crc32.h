#ifndef CRC32_H_
#define CRC32_H_

#define CRC32_INITIAL_VALUE 0xFFFFFFFF

uint32 crc32(uint8 *buffer, uint32 len, uint32 checksum);

#endif /*CRC32_H_*/
