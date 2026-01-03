#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <stdexcept>

#include "network_utils.h" // 引入网络相关的跨平台工具
#include "nlohmann/json.hpp" // 引入 nlohmann/json
#include "spdlog/spdlog.h" // 引入 spdlog

// 使用 nlohmann::json 命名空间
using json = nlohmann::json;

// ---定义字节序枚举 --- 目前主流的PC（Intel/AMD x86/x64架构）都使用小端。
enum class ByteOrder {
    BigEndian,    // 网络字节序,网络字节序就是大端序
    LittleEndian  // 主机字节序
};

// 协议头结构体
// #pragma pack(push, 1) // 设置新的对齐边界为1字节。这告诉编译器：“请紧凑地排列成员，不要插入任何填充字节”。
// #pragma pack(push, 1)
struct ATHeader {
    uint16_t protocol_id;   // 2 bytes
    uint16_t flags;         // 2 bytes
    uint32_t sequence;      // 4 bytes
    uint32_t body_length;   // 4 bytes
    uint32_t hash_value;    // 4 bytes

    static const uint16_t PROTOCOL_ID = 0x4154; // 'AT'
    // 大小端设置
    static const ByteOrder byte_order = ByteOrder::BigEndian;
    std::vector<uint8_t> pack() const;
    void unpack(const uint8_t* data, size_t size);
};
// #pragma pack(pop) // 从堆栈中恢复之前保存的对齐设置

class ATProtocol {
public:
    static constexpr uint16_t PROTOCOL_ID = 0x4154; // 'AT'
    static constexpr uint16_t FLAG_REQUEST = 0x0001;
    static constexpr uint16_t FLAG_RESPONSE = 0x0002;
    static constexpr uint16_t FLAG_ERROR = 0x0004;
    static constexpr size_t HEADER_SIZE = sizeof(ATHeader); // 16 bytes

    ATProtocol();
    ~ATProtocol() = default;

    // 打包数据
    std::vector<uint8_t> pack(uint16_t flags, const std::string& body, uint32_t sequence = 0);

    // 解包数据
    bool unpack(const std::vector<uint8_t>& data, uint16_t& flags, uint32_t& sequence, std::string& body);
    uint32_t get_next_sequence();

private:

    uint32_t _sequence_number;
    std::mutex _seq_mutex;

    ATHeader header;
};


