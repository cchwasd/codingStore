#pragma one
#include <cstdint>
#include <string>

namespace crc32 {
    uint32_t reflect(uint32_t data, size_t num){
        uint32_t reflection = 0;
        for (size_t bit = 0; bit < num; ++bit) {
            if (data & 0x01) {
                reflection |= (1 << ((num - 1) - bit));
            }
            data >>= 1;
        }
        return reflection;
    }

    uint32_t crc_table[256];

    struct CRC32Initializer {
        CRC32Initializer() {
            const uint32_t polynomial = 0x04C11DB7;
            for (uint32_t i = 0; i < 256; ++i) {
                uint32_t crc = reflect(i, 8) << 24;
                for (size_t j = 0; j < 8; ++j) {
                    if (crc & 0x80000000) {
                        crc = (crc << 1) ^ polynomial;
                    } else {
                        crc <<= 1;
                    }
                }
                crc_table[i] = reflect(crc, 32);
            }
        }
    } crc32_init;

    int32_t calculate_hash(const std::string &data) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < data.length(); ++i) {
            uint8_t byte = static_cast<uint8_t>(data[i]);
            uint8_t index = (crc ^ byte) & 0xFF;
            crc = (crc >> 8) ^ crc_table[index];
        }
        // return ~crc;
        return static_cast<int32_t>(crc ^ 0xFFFFFFFF);
    }
};