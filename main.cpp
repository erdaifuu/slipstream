#include <iostream>
#include "src/csv_parser.cpp"

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.

constexpr std::string_view FILENAME = "../data/Quotes_and_Trades.csv";

int main()
{
    gcmd::market_data md = parse_csv(FILENAME.data());
}