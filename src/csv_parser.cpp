#include "../include/slipstream/csv_parser.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>

constexpr int TYPE = 1;
static int64_t TRADE_ID = 1;

namespace QuoteSymbols
{
    enum
    {
        Timestamp,
        Type,
        Symbol,
        BidPrice,
        BidQty,
        AskPrice,
        AskQty
    };
}

namespace TradeSymbols
{
    enum
    {
        Timestamp,
        Type,
        Symbol,
        Price,
        Qty
    };
}

/**
 * Works as intended.
 * Skips the first few commented lines on top of the CSV file.
 * @param file
 */
void skip_csv_header(std::ifstream& file)
{
    std::string line;

    while (std::getline(file, line))
    {
        if (!line.starts_with('#')) break;
    }
}

/**
 * parses quantity from string and assigns it to result
 * @param str
 * @param result
 */
void parse_qty(const std::string& str, auto& result)
{
    std::from_chars(str.data(), str.data() + str.size(), result);
}

/**
 * parses price from string and returns. Keeps track of x10000.
 * @param str Assumed to be of the form int.xx where xx is two digits.
 * @return
 */
int64_t parse_price(const std::string& str)
{
    int64_t first{}, last{};
    auto [p, ecf] = std::from_chars(str.data(), str.data() + str.size(), first);

    // if dot exists
    if (p != str.data() + str.size())
    {
        auto [q, ecl] = std::from_chars(p + 1, str.data() + str.size(), last);
    }

    return first * 10000 + last * 100;
}

/**
 * parses timestamp in the form HH:MM:SS:MMS and returns it in nanoseconds
 * @param timestamp
 * @return
 */
std::chrono::nanoseconds parse_timestamp(const std::string& timestamp)
{
    auto d = [&](const size_t i)
    {
        const auto temp = static_cast<unsigned>(timestamp[i] - '0');
        if (temp > 9) throw std::range_error{"invalid timestamp"};
        return temp;
    };

    const std::chrono::hours h { d(0) * 10 + d(1) };
    const std::chrono::minutes m { d(3) * 10 + d(4) };
    const std::chrono::seconds s { d(6) * 10 + d(7) };
    const std::chrono::milliseconds ms { d(9) * 100 + d(10) * 10 + d(11) };
    return h + m + s + ms;
}

gcmd::market_data parse_csv(const std::string& filename)
{
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) std::cout << "OPEN\n";

    skip_csv_header(file);

    // READ CSV
    std::getline(file, line);

    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        if (!token.empty())
        {
            std::cout << "TOKEN:" << token << '\n';
            tokens.push_back(token);
        }
    }

    gcmd::market_data md;
    switch (tokens.at(TYPE).at(0))
    {
    case 'Q':
        {
            std::cout << "Q\n";
            auto& q = md.emplace<gcmd::quote_body>();
            // parse the text into trade body
            tokens.at(QuoteSymbols::Symbol).copy(q.symbol, sizeof(q.symbol));
            q.ts_ns = parse_timestamp(tokens.at(QuoteSymbols::Timestamp)).count();
            parse_qty(tokens.at(QuoteSymbols::BidQty), q.bid_qty);
            q.bid_px = parse_price(tokens.at(QuoteSymbols::BidPrice));
            parse_qty(tokens.at(QuoteSymbols::AskQty), q.ask_qty);
            q.ask_px = parse_price(tokens.at(QuoteSymbols::AskPrice));
            break;
        }
    case 'T':
        {
            std::cout << "T\n";
            auto& t = md.emplace<gcmd::trade_body>();
            // parse the text into trade body
            tokens.at(TradeSymbols::Symbol).copy(t.symbol, sizeof(t.symbol));
            t.ts_ns = parse_timestamp(tokens.at(TradeSymbols::Timestamp)).count();
            parse_qty(tokens.at(TradeSymbols::Qty), t.qty);
            t.px = parse_price(tokens.at(TradeSymbols::Price));
            t.aggressor = '?'; // TODO: infer the actual quote from the previous trade
            parse_qty(tokens.at(TradeSymbols::Qty), t.qty);
            t.id = TRADE_ID++;
            break;
        }
    }
    return md;
}