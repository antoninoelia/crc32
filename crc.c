#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define CRC32_POLY ((uint32_t)(0x04C11DB7))
#define MSB_32(x) ((uint32_t)x & (1u << 31))
#define INITIAL_CRC32 ((uint32_t)(0xFFFFFFFF))

static uint32_t crc32(uint32_t initial_crc, uint32_t data) {
    uint32_t crc = initial_crc;
    crc = crc ^ data;

    int j = 32;
    while (j-- > 0) crc = MSB_32(crc) ? ((crc << 1) ^ CRC32_POLY) : (crc << 1);

    return crc;
}

static uint32_t stream_crc32(uint8_t const* data, size_t data_len) {
    if (!data) return INITIAL_CRC32;

    uint32_t crc = INITIAL_CRC32;
    size_t i = 0;

    // Support only Little Endian arch
    while (i + 4 <= data_len) {
        uint32_t d =    ((uint32_t)data[i]) | 
                        ((uint32_t)data[i + 1] << 8) |
                        ((uint32_t)data[i + 2] << 16) |
                        ((uint32_t)data[i + 3] << 24);
        crc = crc32(crc, d);
        i += 4;
    }

    if (i < data_len) {
        uint32_t d = 0;
        size_t j = 0;
        while (i < data_len) {
            d |= ((uint32_t)data[i] << (j * 8));
            i++;
            j++;
        }
        crc = crc32(crc, d);
    }
    return crc;
}

int main(void) {
    uint32_t val = 0x12341234;
    uint32_t res;
    res = crc32(INITIAL_CRC32, val);
    // 0xDA622CDD is value computed by stm32f401re board with HAL_CRC_Calculate
    printf("res = %u %X\n", res, res);
    assert(res == 0xDA622CDD);
    uint32_t new_val[2] = {0x12341234, 0xDA622CDD};
    res = stream_crc32((uint8_t const*)&new_val, 8);
    // If we compute crc32 of data + crc32(data) we obtain zero.
    printf("res = %u %X\n", res, res);
    assert(res == 0);
    // test data without length multiple of 4 bytes
    // 0x3DA1135C is value computed by stm32f401re board with HAL_CRC_Calculate
    // and input {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00, 0x00}
    const uint8_t buf[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    res = stream_crc32(buf, 6);
    printf("res = %u %X\n", res, res);
    assert(res == 0x3DA1135C);

    const uint8_t newbuf[12] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00,
                                0x00, 0x5C, 0X13, 0XA1, 0X3D};  // 0x3da1135c
    res = stream_crc32(newbuf, 12);
    printf("res = %u %X\n", res, res);
    assert(res == 0);
}
