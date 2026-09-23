#ifndef SLIPSTREAM_GCMD_H
#define SLIPSTREAM_GCMD_H

#include <cstdint>
#include <vector>

namespace gcmd
{
#pragma pack(push, 1)

    // Wire Protocol - Market Data
    struct frame_header
    {
        uint16_t body_len{};    // length of body only; header excluded
        uint8_t msg_type{};     // 1 = Quote, 2 = Trade, 3 = Heartbeat, 4 = SessionControl
        uint8_t version{};      // Always 1 for this revision
    };

    struct quote_body
    {
        char symbol[12]{};      // ASCII, null-padded, not null-terminated if full
        uint64_t ts_ns{};       // nanoseconds since Unix epoch
        uint32_t bid_qty{};
        int64_t bid_px{};       // Fixed point x10,000
        uint32_t ask_qty{};
        int64_t ask_px{};       // Fixed point x10,000
    };

    struct trade_body
    {
        char symbol[12]{};      // ASCII, null-padded, not null-terminated if full
        uint64_t ts_ns{};       // nanoseconds since Unix epoch
        uint32_t qty{};
        int64_t px{};           // Fixed point x10,000
        char aggressor{};       // 'B', 'S', or '?' if unknown
        int64_t id{};           // Trade identifier
    };

    struct heartbeat_body
    {
        uint64_t ts_ns{};       // server send time
    };

    struct sessioncontrol_body
    {
        uint64_t ts_ns{};       // nanoseconds since Unix epoch
        uint8_t state{};        // 0 = OPEN, 1 = HALT, 2 = CLOSE
    };

    using market_data = std::variant<quote_body, trade_body, heartbeat_body, sessioncontrol_body>;

#pragma pack(pop)

    static_assert(sizeof(frame_header) == 4);
    static_assert(sizeof(quote_body) == 44);
    // static_assert(sizeof(trade_body) == 49);
    static_assert(sizeof(heartbeat_body) == 8);
    static_assert(sizeof(sessioncontrol_body) == 9);
}

#endif //SLIPSTREAM_GCMD_H
