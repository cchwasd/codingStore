// at_protocol.cpp

#include <cstring> // For memcpy
#include <stdexcept>
#include <algorithm> // For std::reverse
#include <functional> // For std::hash
#include <sstream> // For logging hex output if needed
#include "at_protocol.h"
#include "crc32.hpp" // For CRC32 computation

std::vector<uint8_t> ATHeader::pack() const {
    std::vector<uint8_t> buffer(sizeof(ATHeader));
    // std::memcpy(buffer.data(), this, sizeof(ATHeader));
    if (byte_order == ByteOrder::BigEndian) {
        // Network byte order (big-endian)
        uint16_t pid = htons(protocol_id);
        uint16_t flg = htons(flags);
        uint32_t seq = htonl(sequence);
        uint32_t len = htonl(body_length);
        uint32_t hash = htonl(hash_value);

        std::memcpy(buffer.data(), &pid, sizeof(pid));
        // *reinterpret_cast<uint16_t*>(buffer.data() + 0) = pid;
        std::memcpy(buffer.data() + 2, &flg, sizeof(flg));
        std::memcpy(buffer.data() + 4, &seq, sizeof(seq));
        std::memcpy(buffer.data() + 8, &len, sizeof(len));
        std::memcpy(buffer.data() + 12, &hash, sizeof(hash));
    } else {
        // Little-endian
        std::memcpy(buffer.data(), this, sizeof(ATHeader));
    }

    return buffer;
}

void ATHeader::unpack(const uint8_t* data, size_t size){
    if (size < sizeof(ATHeader)) {
        throw std::runtime_error("Insufficient data to unpack ATHeader");
    }

    if (byte_order == ByteOrder::BigEndian) {
        // Network byte order (big-endian)
        uint16_t pid, flg;
        uint32_t seq, len, hash;

        std::memcpy(&pid, data, sizeof(pid));
        std::memcpy(&flg, data + 2, sizeof(flg));
        std::memcpy(&seq, data + 4, sizeof(seq));
        std::memcpy(&len, data + 8, sizeof(len));
        std::memcpy(&hash, data + 12, sizeof(hash));

        protocol_id = ntohs(pid);
        // protocol_id = ntohs(*reinterpret_cast<const uint16_t*>(data + 0));
        flags = ntohs(flg);
        sequence = ntohl(seq);
        body_length = ntohl(len);
        hash_value = ntohl(hash);
    } else {
        // Little-endian
        std::memcpy(this, data, sizeof(ATHeader));
    }
}

ATProtocol::ATProtocol()
    : _sequence_number(0)
{
    header.protocol_id = PROTOCOL_ID;
    header.flags = 0;
    header.sequence = 0;
    header.body_length = 0;
    header.hash_value = 0;
}

// Get next sequence number (thread-safe)
uint32_t ATProtocol::get_next_sequence() {
    std::lock_guard<std::mutex> lock(_seq_mutex);
    return ++_sequence_number;
}


// Pack data into wire format
std::vector<uint8_t> ATProtocol::pack(uint16_t flags, const std::string& body, uint32_t sequence) {
    if (sequence == 0) {
        sequence = get_next_sequence();
    }

    ATHeader header{};
    header.protocol_id = PROTOCOL_ID;
    header.flags = flags;
    header.sequence = sequence;
    header.body_length = static_cast<uint32_t>(body.size());

    header.hash_value = crc32::calculate_hash(body);

    // Prepare the packet buffer
    std::vector<uint8_t> header_buf = header.pack();  
    std::vector<uint8_t> packet(HEADER_SIZE + body.size());
    std::memcpy(packet.data(), header_buf.data(), HEADER_SIZE);
    if (!body.empty())
        std::memcpy(packet.data() + HEADER_SIZE, body.data(), body.size());
    // packet.insert(packet.end(), header_buf.begin(), header_buf.end());
    // packet.insert(packet.end(), body.begin(), body.end());
    
    spdlog::debug("Packed packet: Seq={}, Flags=0x{:04X}, Len={}, Hash=0x{:08X}", sequence, flags, body.size(), header.hash_value);
    return packet;
}

// Unpack data from wire format
bool ATProtocol::unpack(const std::vector<uint8_t>& data, uint16_t& flags, uint32_t& sequence, std::string& body) {
    if (data.size() < HEADER_SIZE) {
        spdlog::error("Data too short to contain header");
        return false;
    }

    header.unpack(data.data(), HEADER_SIZE);

    if (header.protocol_id != PROTOCOL_ID) {
        spdlog::error("Invalid protocol ID");
        return false;
    }

    if (data.size() < HEADER_SIZE + header.body_length) {
        spdlog::error("Packet data size ({}) less than expected (header {} + body {})", data.size(), HEADER_SIZE, header.body_length);
        return false;
    }

    // body = (reinterpret_cast<const char*>(data.data()) + HEADER_SIZE,  header.body_length); // Assign the extracted body
    body.assign(reinterpret_cast<const char*>(data.data()) + HEADER_SIZE, header.body_length);
    flags = header.flags;
    sequence = header.sequence;

    uint32_t calculated_hash = crc32::calculate_hash(body);
    if (calculated_hash != header.hash_value) {
         spdlog::warn("Hash mismatch: received 0x{:08X}, calculated 0x{:08X}. Proceeding anyway.", header.hash_value, calculated_hash);
         // Depending on strictness, you could throw an exception here instead.
    }

    spdlog::debug("Unpacked packet: Seq={}, Flags=0x{:04X}, Len={}, Hash=0x{:08X}", sequence, flags, header.body_length, header.hash_value);
    return true;
}



