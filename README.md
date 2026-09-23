# Slipstream

A single-instrument, volume-aware execution engine in C++23.

## Build

```sh
cmake -B build -S . && cmake --build build
```

## Run
```sh
slipstream \
    --symbol SYNTH1 \
    --max-quantity 500 \
    --participation-cap 0.15 \
    --vwap-window-ms 30000 \
    --band-bps 25.5 \
    --md-host 127.0.0.1 --md-port 14200 \
    --oe-host 127.0.0.1 --oe-port 14300 \
    --transport tcp

md_client Quotes_and_Trades.csv --host 127.0.0.1 --port 14200
oe_client Quotes_and_Trades.csv --host 127.0.0.1 --port 14300
```
