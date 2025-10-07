#ifndef WIIU_ENDIAN_H
#define WIIU_ENDIAN_H

#include <machine/endian.h>
#include <stdint.h>

uint16_t
bswap16(uint16_t int16);

uint32_t
bswap32(uint32_t int32);

uint64_t
bswap64(uint64_t int64);

uint16_t
be16toh(uint16_t big16);

uint32_t
be32toh(uint32_t big32);

uint64_t
be64toh(uint64_t big64);

uint16_t
htobe16(uint16_t host16);

uint32_t
htobe32(uint32_t host32);

uint64_t
htobe64(uint64_t host64);

uint16_t
htole16(uint16_t host16);

uint32_t
htole32(uint32_t host32);

uint64_t
htole64(uint64_t host64);

uint16_t
le16toh(uint16_t little16);

uint32_t
le32toh(uint32_t little32);

uint64_t
le64toh(uint64_t little64);

#endif
