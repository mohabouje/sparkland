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

### Core Infrastructure Components

#### **1. `codec/` - JSON Encoding/Decoding**
- **Purpose**: Zero-copy JSON parsing with compile-time type safety
- **Key Features**:
  - Uses `daw_json_link` for high-performance deserialization
  - Compile-time schema validation
  - No runtime reflection overhead
- **Benefit**: Fast message parsing without dynamic allocation

#### **2. `reflect/` - Compile-Time Reflection**
- **Purpose**: Type introspection and enum string conversion
- **Key Features**:
  - `enum_to_string` and `enum_from_string` for type-safe CLI parsing
  - No runtime overhead - all resolved at compile time
- **Benefit**: Type-safe configuration without boilerplate

#### **3. `logger/` - Structured Logging**
- **Purpose**: Formatted logging with `std::format` integration
- **Key Features**:
  - Level-based logging (info, error, debug)
  - Type-safe format strings
- **Benefit**: Clean diagnostics without printf-style errors

#### **4. `meta/` - Metaprogramming Utilities**
- **Purpose**: Compile-time data structures and type manipulation
- **Key Features**:
  - `meta::map` - Compile-time key-value mapping (exchange factory)
  - `meta::typed<Value>` - Value-to-type conversion
  - `meta::list` - Type list manipulation
- **Benefit**: Runtime dispatch with compile-time optimization

#### **5. `types/` - Strong Type System**
- **Purpose**: Type-safe financial primitives
- **Key Features**:
  - `price`, `quantity` - Fixed-point decimals (no floating-point errors)
  - `timestamp`, `trade_id` - Strong type wrappers
  - `result<T>` - Monadic error handling
- **Benefit**: Eliminates unit confusion and precision loss

#### **6. `network/` - Transport Layer**
- **Purpose**: WebSocket/TLS/TCP abstraction
- **Key Features**:
  - Boost.Beast for WebSocket over TLS
  - Async I/O with io_context
  - Connection lifecycle management
- **Benefit**: Exchange-agnostic transport layer

#### **7. `protocol/` - Message Protocol Layer**
- **Purpose**: Exchange-neutral message types
- **Key Features**:
  - Generic `trade_summary`, `subscribe`, `heartbeat` messages
  - Exchange-specific codecs (Coinbase/Bybit)
  - Frozen hashmaps for O(1) message type dispatch
- **Benefit**: Single codebase supports multiple exchanges

**Message Type Dispatch Example:**
```cpp
// Compile-time hashmap for O(1) type identification
constexpr auto mapper = frozen::unordered_map<frozen::string, std::size_t, 3>{
    {"heartbeat", 0}, {"subscriptions", 1}, {"ticker", 2}
};
```

#### **8. `exchange/` - Exchange Integration**
- **Purpose**: Compose network + protocol + transformers
- **Key Features**:
  - `coinbase/` - Coinbase-specific transformer
  - `bybit/` - Bybit-specific transformer
  - `factory/` - Compile-time exchange selection
- **Benefit**: Add new exchanges without modifying core logic

#### **9. `components/` - Reusable Session Templates**
- **Purpose**: Generic session orchestration
- **Key Features**:
  - `session_id` - Application identification
  - `feeder::codegen` - Compose encoder/decoder/transformer
  - `scheduler` - Event scheduling
- **Benefit**: Consistent session pattern across exchanges

#### **10. `metrics/` - Sliding Window Statistics**
- **Purpose**: Time-windowed metric calculation
- **Key Features**:
  - `timeline<T>` - Automatic event expiration
  - `scan::multimeter` - O(n log n) recalculation
  - `stream::multimeter` - O(log n) incremental updates
- **Benefit**: Tunable performance vs. memory tradeoff


## Metrics Implementations: Scan vs Stream

### Algorithmic Comparison

The system provides two implementations optimized for different use cases:

| Implementation | Time Complexity | Space Complexity | Best For |
||-||-|
| **Scan** | O(n log n) | O(n) | Low event rate, small windows, infrequent reads |
| **Stream** | O(log n) | O(4n) | High event rate, large windows, frequent reads |

### Scan Multimeter: O(n log n) Recalculation

**Algorithm:**
- Shares single `timeline` across all metrics (memory efficient)
- On each metric read: sorts timeline for median, scans for min/max/mean
- Simple implementation with excellent cache locality

```cpp
auto multimeter = metrics::scan::multimeter<trade_summary>(std::chrono::minutes(5));

// Per-event cost:
// 1. Insert into timeline: O(1)
// 2. Remove expired events: O(k) where k = expired count
// 3. Sort for median: O(n log n)
// 4. Linear scan for min/max/mean: O(n)
```

**Performance Characteristics:**
- **Cache-friendly**: Sequential memory access patterns
- **Low memory overhead**: Single shared timeline container
- **Small window advantage**: At small n (< 100 events), sorting overhead is minimal
- **Infrequent reads**: Amortizes O(n log n) cost when metrics sampled less frequently than events

### Stream Multimeter: O(log n) Incremental Updates

**Algorithm:**
- Each metric maintains independent timeline + specialized data structure
- Median: priority queue with middle element tracking
- Min/Max: heaps for logarithmic insert/delete
- Mean: running sum + count (O(1))

```cpp
auto multimeter = metrics::stream::multimeter<trade_summary>(std::chrono::minutes(5));

// Per-event cost:
// 1. Insert into timeline: O(1)
// 2. Update median priority queue: O(log n)
// 3. Update min/max heaps: O(log n)
// 4. Update running mean: O(1)
```

**Performance Characteristics:**
- **Predictable latency**: Consistent O(log n) worst-case performance
- **Window size independence**: Minimal performance degradation with larger windows
- **High throughput**: Handles 1000+ events/second efficiently
- **Frequent reads**: No recalculation cost on each metric access

## Benefits of This Design

### 1. **Separation of Concerns**
Each component has a single, well-defined responsibility:
- **Network**: Connection management
- **Protocol**: Message encoding/decoding
- **Exchange**: Exchange-specific transformations
- **Metrics**: Statistical calculations
- **Application**: Orchestration

**Benefit**: Easy to test, debug, and extend individual components without affecting others.

### 2. **Compile-Time Dispatch with Runtime Flexibility**
- **Runtime**: User selects exchange/metrics via CLI
- **Compile-time**: Template instantiation optimizes hot path

```cpp
// User: "metrics-capture -e coinbase -m stream"
// Runtime: Parse "coinbase" → exchange_id::coinbase
// Compile-time: Instantiate execute<exchange_id::coinbase, type::stream>()
```

**Benefit**: Zero runtime overhead for type dispatch, full optimization of critical paths.

### 3. **Type Safety Eliminates Runtime Errors**
- **Strong types**: `price`, `quantity`, `timestamp` prevent unit confusion
- **Result monad**: Explicit error handling without exceptions
- **Enum reflection**: String→Enum conversion validated at parse time

**Benefit**: Entire classes of bugs (unit errors, precision loss) are compile-time errors.

### 4. **Reusability Through Composition**
- **Exchange factory**: Add new exchange = implement contract + register in factory
- **Metrics**: Timeline + algorithm separation enables easy experimentation
- **Session template**: Same pattern for all exchanges

**Benefit**: Adding Binance/Kraken support requires only exchange-specific transformer, reuses entire infrastructure.

### 5. **Performance by Design**
- **Zero-copy parsing**: JSON decoded directly into structs
- **Fixed-point math**: No floating-point precision loss
- **Compile-time dispatch**: No virtual function overhead
- **Cache-friendly scan**: Sequential memory access for small datasets

**Benefit**: Latency-sensitive paths are optimized at compile time.

### 6. **Testability**
- Each component has clear inputs/outputs
- Timeline logic isolated from metrics algorithms
- Mock exchanges via contract pattern
- Unit tests cover scan/stream implementations independently

**Benefit**: Comprehensive test coverage with minimal mocking infrastructure.

### 7. **Extensibility**
Want to add a new metric (e.g., standard deviation)?
- Implement `stream::stddev` and `scan::stddev`
- Add field to `metrics` struct
- Zero changes to network/protocol/exchange layers

**Benefit**: Feature additions are localized and non-invasive.

## Building and Testing

### Prerequisites

- **Compiler**: C++20 or later (GCC 11+, Clang 14+, MSVC 2022+)
- **Build System**: [xmake](https://xmake.io/)
- **Package Manager**: Conan 2.0+

### Dependencies

All dependencies are automatically managed via Conan:
- **Networking**: Boost.Asio, Boost.Beast, OpenSSL
- **JSON**: daw_json_link (zero-copy parsing)
- **Testing**: Google Test, Google Benchmark
- **Utilities**: CLI11, magic_enum, frozen (compile-time maps), xxhash

See [xmake.lua](xmake.lua) for full dependency list.

### Project Structure

```
sparkland/
├── apps/
│   └── metrics-capture/    # Main application binary
├── codec/                  # JSON encoding/decoding (daw_json_link)
├── components/             # Reusable session/scheduler templates
├── core/                   # Utilities (assert, result monad)
├── exchange/               # Exchange-specific implementations
│   ├── coinbase/           # Coinbase connector, transformer, session
│   ├── bybit/              # Bybit implementation
│   └── factory/            # Compile-time exchange factory
├── logger/                 # Structured logging with std::format
├── meta/                   # Metaprogramming (map, typed, list)
├── metrics/                # Sliding window metrics
│   ├── benchmark/          # Performance benchmarks
│   ├── include/spl/metrics/
│   │   ├── scan/           # O(n log n) recalculative metrics
│   │   └── stream/         # O(log n) incremental metrics
│   └── test/               # Unit tests
├── network/                # WebSocket/TLS/TCP layer (Boost.Beast)
├── protocol/               # Protocol definitions and codecs
│   ├── coinbase/           # Coinbase WebSocket protocol
│   ├── bybit/              # Bybit WebSocket protocol
│   ├── feeder/             # Generic market data protocol
│   └── common/             # Shared types (exchange_id, price, etc.)
├── reflect/                # Compile-time reflection (enum conversion)
├── result/                 # Result monad for error handling
├── types/                  # Strong types (price, quantity, timestamp)
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
