# Sparkland - Cryptocurrency Market Data Aggregation System

[Mohammed Boujemaoui Boulaghmoudi](cv.pdf) | C++ Senior Software Engineer - Quant Developer  
mohabouje@gmail.com | [Github](https://github.com/mohabouje) | [Web](https://mohabouje.github.io)

High-performance C++ application that subscribes to cryptocurrency exchange WebSocket feeds (Coinbase, Bybit), parses trade data from JSON messages, and calculates sliding window statistics (mean, median, min, max) over configurable time periods.

**Implementation Highlights:**
- **Zero-copy JSON parsing**: Direct decoding from network buffer with no extra copies
- Runtime-configurable exchange selection with compile-time dispatch
- Dual metrics implementations: O(n log n) scan vs O(log n) stream
- WebSocket over TLS using Boost.Beast
- Fixed-point decimal arithmetic for financial precision
- Modular component-based architecture


## Usage

```bash
# Build and run with Coinbase, 5-minute window, export to CSV
xmake config -m release && xmake build metrics-capture
xmake run metrics-capture -e coinbase -i BTC-USDT -w 5 -d 60 -o trades.csv

# Run with Bybit, stream metrics, 10-minute window
xmake run metrics-capture -e bybit -m stream -i BTCUSDT -w 10 -o output.csv
```

| Argument | Description | Default | Options |
|-|-|-|-|
| `-e, --exchange` | Exchange to connect to | `coinbase` | `coinbase`, `bybit` |
| `-m, --metrics` | Metrics implementation | `stream` | `stream`, `scan` |
| `-i, --instrument` | Trading pair to track | `BTC-USDT` | Any valid pair |
| `-w, --window` | Window size (minutes) | `5` | Positive integer |
| `-d, --duration` | Run duration (minutes) | `60` | Positive integer |
| `-o, --output` | CSV output file path | _(none)_ | Any valid path |

**Fields:**
- `timestamp`: Event time in nanoseconds since Unix epoch
- `minimum`: Minimum trade price in sliding window
- `maximum`: Maximum trade price in sliding window
- `median`: Median trade price in sliding window
- `mean`: Average (arithmetic mean) trade price in sliding window

All prices use fixed-point decimal representation for exact financial precision.



## Architecture & Component Design

### Composable Layered Architecture

The system is built from small, focused, composable components that separate concerns and enable reusability:

```
┌─────────────────────────────────────────────────────────┐
│              Application Layer                           │
│         (metrics-capture binary)                         │
│  Runtime argument parsing → Compile-time dispatch        │
└────────────────┬────────────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────────────┐
│              Exchange Factory                            │
│   Compile-time dispatch: Coinbase/Bybit contracts       │
└────────────────┬────────────────────────────────────────┘
                 │
    ┌────────────┼────────────┐
    │            │            │
┌───▼──────┐ ┌──▼────────┐ ┌▼──────────┐
│ Exchange │ │ Protocol  │ │  Metrics  │
│  Layer   │ │   Layer   │ │   Layer   │
└────┬─────┘ └────┬──────┘ └─────┬─────┘
     │            │              │
     │       ┌────▼────┐    ┌────▼─────┐
     │       │ Codec   │    │ Timeline │
     │       │ Reflect │    │Scan/Stream│
     │       │  Meta   │    └──────────┘
     │       └─────────┘
     │
┌────▼─────────────────┐
│   Network Layer      │
│ WebSocket/TLS/TCP    │
└──────────────────────┘
```

### Core Components

- **codec/**: Zero-copy JSON parsing with `daw_json_link` - no allocation overhead
- **reflect/**: Compile-time reflection for enum↔string conversion
- **logger/**: Structured logging with `std::format`
- **meta/**: Compile-time maps and type utilities for dispatch optimization
- **types/**: Strong financial types (`price`, `quantity`) with fixed-point arithmetic
- **network/**: WebSocket/TLS abstraction using Boost.Beast
- **protocol/**: Exchange-neutral messages with frozen hashmaps for O(1) dispatch
- **exchange/**: Coinbase/Bybit integration with compile-time factory selection
- **components/**: Reusable session templates and scheduling
- **metrics/**: Sliding window statistics with scan (O(n log n)) vs stream (O(log n)) implementations


### Metrics Implementations: Scan vs Stream

| Implementation | Time Complexity | Space Complexity | Best For |
||-||-|
| **Scan** | O(n log n) | O(n) | Low event rate, small windows |
| **Stream** | O(log n) | O(4n) | High event rate, frequent reads |

**Scan Multimeter:**
- Shared timeline across all metrics (memory efficient)
- On each read: sorts for median, scans for min/max/mean
- Best for: < 100 events, infrequent metric sampling

**Stream Multimeter:**
- Independent data structures per metric
- Median: priority queue, Min/Max: heaps, Mean: running sum
- Best for: 1000+ events/second, frequent reads

**Performance Results:**
- Stream vs Scan at 1000 events/s: **451× faster**
- Stream maintains O(log n) regardless of window size

### Design Benefits

1. **Separation of Concerns**: Each component has single responsibility (network, protocol, metrics, etc.)
2. **Compile-Time Optimization**: User selects exchange/metrics via CLI, templates optimize hot paths at compile time
3. **Type Safety**: Strong types (`price`, `quantity`) and result monads eliminate runtime errors
4. **Zero-Copy Architecture**: Direct JSON parsing from network buffer with no intermediate copies
5. **Scalable Performance**: Stream metrics handle 1000+ events/second with O(log n) complexity

### Testing and Dependencies

**Prerequisites:** C++20 compiler, [xmake](https://xmake.io/), Conan 2.0+

**Key Dependencies:** Boost.Beast, OpenSSL, daw_json_link, Google Test/Benchmark, CLI11

**Build Commands:**
```bash
xmake config -m release && xmake build   # Release build
xmake test                               # Run all tests (90 test cases)
xmake run metrics-capture --help         # Usage info
```

### Project Structure

```
sparkland/
├── apps/metrics-capture/   # Main application binary
├── codec/                  # Zero-copy JSON parsing (daw_json_link)
├── components/             # Session/scheduler templates
├── exchange/               # Exchange integrations (Coinbase/Bybit)
├── logger/                 # Structured logging
├── meta/                   # Compile-time metaprogramming
├── metrics/                # Sliding window statistics + benchmarks
├── network/                # WebSocket/TLS layer (Boost.Beast)
├── protocol/               # Message protocols and codecs
├── reflect/                # Compile-time reflection
├── result/                 # Result monad
├── types/                  # Strong financial types
└── xmake.lua               # Build configuration
```

## References

- **Assignment**: [task.pdf](task.pdf) - HFT Quants Developer home assignment
- **Coinbase API**: [Advanced Trade WebSocket API](https://docs.cdp.coinbase.com/advanced-trade/docs/)
- **Author CV**: [cv.pdf](cv.pdf)

## License

This project was created as a technical assessment for Sparkland Trading.

**Author**: Mohammed Boujemaoui Boulaghmoudi
**Contact**: mohabouje@gmail.com
