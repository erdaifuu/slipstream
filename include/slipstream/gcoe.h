#ifndef SLIPSTREAM_GCOE_H
#define SLIPSTREAM_GCOE_H

#include <cstdint>

namespace gcoe
{
#pragma pack(push, 1)

    // Wire Protocol - Order
    struct frame_header
    {
        uint16_t body_len{};    // length of body only; header excluded
        uint8_t msg_type{};     // 1 = Quote, 2 = Trade, 3 = Heartbeat, 4 = SessionControl
        uint8_t version{};      // Always 1 for this revision
    };

    // slipstream -> OE client, msg_type = 10
    struct new_order
    {
        uint64_t client_order_id{}; // strictly increasing, unique per process run
        char symbol[12]{};          // ASCII, null-padded, not null-terminated if full
        char status{};              // A for accepted, R for rejected
        uint64_t ts_ns{};           // your send time, either relative to the trade or absolute (decide)
        int64_t trade_id{};         // id associated with the trade that generated this new_order
        char side{};                // B or S
        uint32_t qty{};
        int64_t limit_px{};         // fixed point x10,000
    };

    // slipstream -> OE client, msg_type = 11
    struct exec_report
    {
        uint64_t client_order_id{}; // echo of the order this refers to
        uint64_t ts_ns{};           // gateway timestamp
        uint8_t status{};           // 0=ACK, 1=FILL, 2=PARTIAL, 3=REJECT
        uint32_t filled_qty{};      // cumulative ofr this order
        int64_t avg_px{};           // Fixed point x10,000
        uint8_t reason_code{};      // 0=none, 1=risk, 2=price, 3=size, 4=throttle
    };

#pragma pack(pop)

    static_assert(sizeof(frame_header) == 4);
    // static_assert(sizeof(new_order) == 42);
    static_assert(sizeof(exec_report) == 30);
}

#endif //SLIPSTREAM_GCOE_H
