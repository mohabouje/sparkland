# Sparkland - Cryptocurrency Market Data Aggregation System

**[Mohammed Boujemaoui Boulaghmoudi](cv.pdf)**
C++ Senior Software Engineer - Quant Developer
mohabouje@gmail.com | [Github](https://github.com/mohabouje) | [Web](https://mohabouje.github.io)

---

## Overview

High-performance C++ application that subscribes to Coinbase WebSocket ticker feed, parses trade prices from JSON messages, and calculates sliding window statistics (mean, median, min, max) over configurable time periods. Designed for latency-sensitive quantitative trading with emphasis on computational efficiency in the metrics calculation path.

**Task Requirements ([task.pdf](task.pdf)):**
- ✓ Subscribe to Coinbase ticker via WebSocket
- ✓ Parse trade price from JSON ticker data
- ✓ Calculate mean/median/range over 5-minute sliding window (scalable to 1+ hour)
- ✓ Log time, trade_id, trade_price, and calculated metrics to CSV
- ✓ Optimized for memory and speed in latency-sensitive path
- ✓ Comprehensive unit tests for sliding window calculations

**Implementation Highlights:**
- Dual metrics implementations: O(n log n) scan vs O(log n) stream
- Zero-copy JSON parsing with compile-time message dispatch
- WebSocket over TLS using Boost.Beast
- Fixed-point decimal arithmetic for financial precision
- Configurable window durations (5 minutes to hours)

---

## Architecture

**Layered Design:**
```
Application → Exchange → Protocol → Network → Core Infrastructure
                    ↓
                Metrics (Timeline + Scan/Stream)
```

**Key Modules:**
- **`network/`**: WebSocket over TLS (Boost.Beast), connection management, async I/O
- **`protocol/`**: JSON codec, message types (ticker, subscribe, heartbeat), transformers
- **`exchange/`**: Coinbase/Bybit integration composing network + protocol layers
- **`components/`**: Reusable session templates with encoder/decoder/scheduler
- **`metrics/`**: Sliding window statistics with dual implementations (detailed below)
- **`types/`**: Fixed-point price/quantity, strong typing, result monad
- **`codec/`, `reflect/`, `meta/`**: JSON parsing, reflection, compile-time dispatch

---

## WebSocket Subscription to Coinbase Ticker

### Connection & Subscription Flow

The application establishes a WebSocket connection to Coinbase and subscribes to the ticker channel:

```cpp
// 1. Create session
using production = spl::exchange::coinbase::feeder::session<environment::production>;
auto context = spl::network::context();
auto session = production(context, session_id{"app", "coinbase"});

// 2. Connect to wss://ws-feed.exchange.coinbase.com/
session.connect();

// 3. Subscribe to BTC-USD ticker channel
session.send(subscribe{
    .exchange_id = exchange_id::coinbase,
    .instrument_id = instrument_id{"BTC-USD"},
    .channel = channel::trades
});
```

**WebSocket Message Processing Pipeline:**
```
JSON Message → Decoder (type dispatch) → Transformer → trade_summary → Handler
                   ↓ (frozen hashmap O(1))
            ticker::ticker struct
```

**Example Coinbase Ticker Message:**
```json
{
  "type": "ticker",
  "sequence": 12345678,
  "product_id": "BTC-USD",
  "price": "50000.00",
  "time": "2024-10-18T12:34:56.789Z",
  "trade_id": 87654321,
  "last_size": "0.01",
  "side": "buy"
}
```

**Parsed to `trade_summary`:**
```cpp
struct trade_summary {
    exchange_id exchange_id;        // coinbase
    instrument_id instrument_id;    // (optional, from product_id)
    trade_id trade_id;              // 87654321
    aggressor_side side;            // buy
    price price;                    // 50000.00 (fixed-point)
    quantity quantity;              // 0.01 (fixed-point)
    sequence sequence;              // 12345678
    timestamp timestamp;            // parsed from ISO8601
};
```

### Transformer: Exchange-Specific → Generic

The `transformer` converts Coinbase's ticker format to the generic `trade_summary`:

**Key Operations:**
- ISO8601 timestamp parsing → nanosecond precision
- String price/quantity → Fixed-point decimal types
- Side string ("buy"/"sell") → Enum `aggressor_side`
- Type-safe transformation eliminates runtime errors

```cpp
// Example: ticker → trade_summary conversion
auto operator()(ticker::ticker const& input, Handler&& handler) {
    auto price = spl::types::price::from(input.price);
    auto quantity = spl::types::quantity::from(input.last_size);
    auto side = aggressor_side(input.side == "buy");
    auto timestamp = parse_iso8601(input.time);  // "2024-10-18T12:34:56.789Z" → ns

    return handler(trade_summary{
        .exchange_id = exchange_id::coinbase,
        .side = side,
        .price = price,
        .quantity = quantity,
        .sequence = sequence(input.sequence),
        .timestamp = timestamp
    });
}
```

**Decoder Dispatch Mechanism:**
Uses frozen hash map for compile-time O(1) message type identification:
```cpp
constexpr auto mapper = frozen::unordered_map<frozen::string, std::size_t, 3>{
    {"heartbeat", 0}, {"subscriptions", 1}, {"ticker", 2}
};
```

---

## Sliding Window Metrics Calculation

### Requirement: Mean/Median/Range Over 5-Minute Window

The metrics system calculates statistics over a configurable sliding time window (5 minutes for this task, scalable to hours).

**Design Goals:**
1. **Memory efficiency**: Only store events within the window
2. **Speed**: Optimize for latency-sensitive calculation path
3. **Scalability**: Support 5 minutes to 1+ hour windows
4. **Accuracy**: Use fixed-point arithmetic for financial precision

### Timeline: Automatic Event Expiration

`spl::metrics::timeline<T>` provides the foundation for windowed calculations:

```cpp
template <typename T, template<typename...> class ContainerT = std::deque>
struct timeline {
    explicit timeline(std::chrono::nanoseconds period);  // e.g., 5 minutes

    auto emplace_back(T&& event) -> T&;     // Add event
    auto flush(timestamp) -> void;           // Remove expired events
    auto size() const -> std::size_t;
    auto begin/end() -> iterator;            // STL-compatible iteration
};
```

**Automatic Expiration Mechanism:**
- On `emplace_back()`, checks if window duration exceeds configured period
- Events older than `current_timestamp - period` are automatically removed via `flush()`
- Provides clean iterator interface to metrics calculators
- Uses `std::deque` for efficient front removal (O(1))

### Two Implementation Strategies

The system provides two algorithmic approaches optimized for different use cases:

#### 1. Scan Multimeter: O(n log n) Recalculation

**Algorithm:**
- Shares single `timeline` across all metrics (memory efficient)
- On each `operator()` call: sorts timeline for median, scans for min/max/mean
- Simple implementation, lower memory overhead

```cpp
auto multimeter = spl::metrics::scan::multimeter<trade_summary>(std::chrono::minutes(5));

for (auto const& trade : trades) {
    auto metrics = multimeter(trade);
    // Internally:
    // 1. timeline.emplace_back(trade)
    // 2. timeline.flush() - remove expired
    // 3. Sort timeline for median: O(n log n)
    // 4. Scan for min/max/mean: O(n)
}
```

**Time Complexity Per Event:**
- Insert: O(1) amortized
- Expire old events: O(k) where k = expired count
- Compute median: O(n log n) via sorting
- Compute min/max/mean: O(n) via linear scan
- **Total: O(n log n)**

**Space Complexity:** O(n) - single timeline

**Best Use Case:**
- Low event rates (< 10 events/second)
- **Infrequent metric reads** (e.g., once per second instead of per-event)
- Small windows (< 10 seconds)
- When amortizing sorting cost over multiple events

**Performance Note:** If `multimeter.read()` is called less frequently than event arrival, scan implementation performs significantly better overall by amortizing the O(n log n) cost.

#### 2. Stream Multimeter: O(log n) Incremental Updates

**Algorithm:**
- Each metric maintains its own timeline + specialized data structures
- Median: priority queue with iterators to middle elements
- Min/Max: heaps for O(log n) insertion/removal
- Mean: running sum + count for O(1) updates
- Updates incrementally on each event

```cpp
auto multimeter = spl::metrics::stream::multimeter<trade_summary>(std::chrono::minutes(5));

for (auto const& trade : trades) {
    auto metrics = multimeter(trade);
    // Internally:
    // 1. timeline.emplace_back(trade)
    // 2. timeline.flush(timestamp, callback)
    //    - Callback removes expired from each metric's structures
    // 3. median_.update(trade)  - O(log n) heap operations
    // 4. max_.update(trade)     - O(log n) heap operations
    // 5. min_.update(trade)     - O(log n) heap operations
    // 6. mean_.update(trade)    - O(1) running average
}
```

**Time Complexity Per Event:**
- Insert into timeline: O(1) amortized
- Expire events: O(log n) per expired event (heap removals)
- Update median: O(log n) priority queue operations
- Update min/max: O(log n) heap operations
- Update mean: O(1) arithmetic
- **Total: O(log n)**

**Space Complexity:** O(4n) - independent timeline per metric

**Best Use Case:**
- High event rates (> 100 events/second)
- **Frequent metric reads** (on every event)
- Large windows (> 60 seconds)
- Consistent latency requirements
- Predictable O(log n) worst-case performance

### Metrics Output Structure

Both implementations return the same result type:

```cpp
struct metrics {
    spl::types::price minimum;    // Lowest trade price in window
    spl::types::price maximum;    // Highest trade price in window
    spl::types::price median;     // Median trade price (sorted middle)
    spl::types::price mean;       // Average trade price (sum / count)
    std::chrono::nanoseconds timestamp;  // Event timestamp
};
```

**Fixed-Point Price Type:**
- Uses `spl::types::price` for exact decimal representation
- Avoids floating-point precision errors in financial calculations
- Internally stores price as scaled integer (e.g., price in cents)

### Algorithm Comparison Example

**Scenario: 1000 events/sec, 300-second window**

| Metric | Scan Multimeter | Stream Multimeter | Speedup |
|--------|----------------|-------------------|---------|
| Time per 20k events | 1,504,849 μs | 3,333 μs | **451×** |
| Window size impact | 57× slower (1s→300s) | 1.1× slower | Minimal |
| Best for | Infrequent reads | Frequent reads | - |

**When to choose Scan:** Event rate is low AND metrics sampled infrequently (e.g., once/second)
**When to choose Stream:** High event rate OR frequent metric sampling

---

## CSV Logging: Time, Trade ID, Price, Metrics

### Application Event Loop with CSV Output

The application polls the session, processes trade events, calculates metrics, and logs to CSV:

```cpp
auto multimeter = spl::metrics::stream::multimeter<trade_summary>(std::chrono::minutes(5));
auto csv_file = std::ofstream("trades.csv");

// CSV Header
csv_file << "timestamp,trade_id,trade_price,min_price,max_price,median_price,mean_price\n";

// Event loop
while (running) {
    session.poll([&](auto&& event) -> result<void> {
        // Type-safe event handling via std::variant pattern matching
        if constexpr (std::is_same_v<std::decay_t<decltype(event)>, trade_summary>) {
            // Calculate metrics over 5-minute window
            auto metrics = multimeter(event);

            // Log to CSV: timestamp, trade_id, price, min, max, median, mean
            csv_file << std::format("{},{},{},{},{},{},{}\n",
                event.timestamp.count(),           // nanoseconds since epoch
                event.trade_id,                    // exchange trade ID
                event.price,                       // current trade price
                metrics.minimum,                   // min in window
                metrics.maximum,                   // max in window
                metrics.median,                    // median in window
                metrics.mean);                     // mean in window
            csv_file.flush();
        }
        return success();
    });
}
```

### CSV Output Example

```csv
timestamp,trade_id,trade_price,min_price,max_price,median_price,mean_price
1697654321000000000,12345678,50000.00,49950.00,50100.00,50025.00,50037.50
1697654322000000000,12345679,50010.00,49950.00,50100.00,50030.00,50038.25
1697654323000000000,12345680,49980.00,49950.00,50100.00,50015.00,50035.80
...
```

**Field Descriptions:**
- **`timestamp`**: Trade execution time in nanoseconds since Unix epoch (parsed from ISO8601)
- **`trade_id`**: Exchange-provided unique trade identifier
- **`trade_price`**: Current trade price (fixed-point decimal from ticker.price)
- **`min_price`**: Minimum trade price in 5-minute sliding window
- **`max_price`**: Maximum trade price in 5-minute sliding window
- **`median_price`**: Median trade price in 5-minute sliding window
- **`mean_price`**: Average (arithmetic mean) trade price in 5-minute window

**Data Integrity:**
- All prices use `spl::types::price` fixed-point type
- No floating-point precision loss in calculations
- Timestamps maintain nanosecond precision from exchange

---

## Performance Analysis: Scan vs Stream Multimeter

### Benchmark Methodology

The metrics system was benchmarked using Google Benchmark with the following configuration:
- **Sample Size**: 20,000 trades per iteration
- **Event Rates**: 1, 10, and 1000 events/second
- **Window Sizes**: 1, 10, 60, 180, and 300 seconds
- **Test Data**: Synthetic BTC-USD trades with realistic price distributions
- **Measurement**: Metrics calculated **on every event** (worst case for scan)

**Important Note on Read Frequency:**
The benchmarks measure performance when metrics are read on **every event**. In scenarios where `multimeter.read()` is called less frequently (e.g., only once per second instead of per-event), the **scan implementation will perform significantly better** overall, as it amortizes the O(n log n) sorting cost over multiple events. Consider your application's metric sampling frequency when choosing an implementation.

### Benchmark Results

```
---------------------------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------------
BM_ScanMultimeter/1/1               711 us          711 us          927 events_per_sec=1 items_per_second=28.1318M/s window_size_sec=1
BM_ScanMultimeter/1/10             2469 us         2469 us          284 events_per_sec=1 items_per_second=8.10202M/s window_size_sec=10
BM_ScanMultimeter/1/60            10108 us        10108 us           69 events_per_sec=1 items_per_second=1.97864M/s window_size_sec=60
BM_ScanMultimeter/1/180           25389 us        25389 us           28 events_per_sec=1 items_per_second=787.751k/s window_size_sec=180
BM_ScanMultimeter/1/300           40526 us        40525 us           17 events_per_sec=1 items_per_second=493.523k/s window_size_sec=300
BM_ScanMultimeter/10/1             2477 us         2477 us          282 events_per_sec=10 items_per_second=8.07582M/s window_size_sec=1
BM_ScanMultimeter/10/10           15320 us        15318 us           46 events_per_sec=10 items_per_second=1.30565M/s window_size_sec=10
BM_ScanMultimeter/10/60           79717 us        79707 us            9 events_per_sec=10 items_per_second=250.919k/s window_size_sec=60
BM_ScanMultimeter/10/180         229579 us       229552 us            3 events_per_sec=10 items_per_second=87.1261k/s window_size_sec=180
BM_ScanMultimeter/10/300         368969 us       368933 us            2 events_per_sec=10 items_per_second=54.2103k/s window_size_sec=300
BM_ScanMultimeter/1000/1         131447 us       131444 us            5 events_per_sec=1k items_per_second=152.155k/s window_size_sec=1
BM_ScanMultimeter/1000/10       1077951 us      1077772 us            1 events_per_sec=1k items_per_second=18.5568k/s window_size_sec=10
BM_ScanMultimeter/1000/60       1494507 us      1494319 us            1 events_per_sec=1k items_per_second=13.384k/s window_size_sec=60
BM_ScanMultimeter/1000/180      1497440 us      1497370 us            1 events_per_sec=1k items_per_second=13.3568k/s window_size_sec=180
BM_ScanMultimeter/1000/300      1504849 us      1504602 us            1 events_per_sec=1k items_per_second=13.2926k/s window_size_sec=300
BM_StreamMultimeter/1/1            3424 us         3424 us          204 events_per_sec=1 items_per_second=5.84134M/s window_size_sec=1
BM_StreamMultimeter/1/10           3416 us         3416 us          204 events_per_sec=1 items_per_second=5.85511M/s window_size_sec=10
BM_StreamMultimeter/1/60           3789 us         3789 us          184 events_per_sec=1 items_per_second=5.27832M/s window_size_sec=60
BM_StreamMultimeter/1/180          3790 us         3790 us          185 events_per_sec=1 items_per_second=5.27713M/s window_size_sec=180
BM_StreamMultimeter/1/300          3817 us         3817 us          184 events_per_sec=1 items_per_second=5.23963M/s window_size_sec=300
BM_StreamMultimeter/10/1           3766 us         3766 us          185 events_per_sec=10 items_per_second=5.31054M/s window_size_sec=1
BM_StreamMultimeter/10/10          3793 us         3793 us          184 events_per_sec=10 items_per_second=5.27239M/s window_size_sec=10
BM_StreamMultimeter/10/60          3822 us         3821 us          183 events_per_sec=10 items_per_second=5.23434M/s window_size_sec=60
BM_StreamMultimeter/10/180         3821 us         3821 us          179 events_per_sec=10 items_per_second=5.23417M/s window_size_sec=180
BM_StreamMultimeter/10/300         3809 us         3809 us          184 events_per_sec=10 items_per_second=5.2509M/s window_size_sec=300
BM_StreamMultimeter/1000/1         3688 us         3688 us          189 events_per_sec=1k items_per_second=5.4235M/s window_size_sec=1
BM_StreamMultimeter/1000/10        3808 us         3808 us          185 events_per_sec=1k items_per_second=5.25264M/s window_size_sec=10
BM_StreamMultimeter/1000/60        3378 us         3378 us          209 events_per_sec=1k items_per_second=5.92131M/s window_size_sec=60
BM_StreamMultimeter/1000/180       3346 us         3346 us          203 events_per_sec=1k items_per_second=5.97775M/s window_size_sec=180
BM_StreamMultimeter/1000/300       3333 us         3333 us          210 events_per_sec=1k items_per_second=6.0008M/s window_size_sec=300
```

### Algorithmic Complexity Analysis

#### Scan Multimeter Implementation

**Time Complexity:** O(n log n) per event
- **Data Structure:** `std::vector<trade_summary>` storing all events within the time window
- **Operation:** For each new event:
  1. Insert event: O(1)
  2. Remove expired events: O(k) where k is number of expired events
  3. Calculate median: O(n log n) via sorting
  4. Calculate min/max/mean: O(n) via linear scan

**Space Complexity:** O(n) where n is the number of events in the time window

**Performance Characteristics:**
- **Window Size Impact:** Linear to quadratic degradation as window size increases
- **Event Rate Impact:** Severe performance degradation with higher event rates
- **Memory Usage:** Proportional to window size × event rate

#### Stream Multimeter Implementation

**Time Complexity:** O(log n) per event
- **Data Structures:**
  - `std::priority_queue<event>` for median calculation
  - Running statistics for min/max/mean
  - Timeline-based event queue for expiration
- **Operation:** For each new event:
  1. Insert into priority queue: O(log n)
  2. Update running statistics: O(1)
  3. Remove expired events: O(log n) per expired event
  4. Median access: O(1) via iterator maintenance

**Space Complexity:** O(n) where n is the number of events in the time window

**Performance Characteristics:**
- **Window Size Impact:** Minimal impact (logarithmic scaling)
- **Event Rate Impact:** Consistent performance across all event rates
- **Memory Usage:** Optimized data structures with minimal overhead

### Performance Comparison

#### Low Event Rate Scenarios (1-10 events/sec)

At low event rates, the **scan approach can outperform stream** for small windows:
- **1 event/sec, 1s window:** Scan = 711μs vs Stream = 3424μs (4.8× faster)
- **Reason:** The overhead of maintaining complex data structures (priority queue, iterators) exceeds the cost of simple vector operations when the dataset is small

#### High Event Rate Scenarios (1000 events/sec)

As event rates increase, the **stream approach demonstrates superior scalability**:
- **1000 events/sec, 300s window:** Scan = 1,504,849μs vs Stream = 3,333μs (**451× faster**)
- **Reason:** O(n log n) sorting becomes prohibitively expensive as the window contains more events

#### Window Size Scaling

**Scan Performance Degradation:**
- 1s window: 711μs → 300s window: 40,526μs (57× slower)
- Performance deteriorates dramatically with larger windows due to increased sorting overhead

**Stream Performance Consistency:**
- 1s window: 3,424μs → 300s window: 3,817μs (1.1× slower)
- Minimal performance impact regardless of window size due to logarithmic operations

### Performance Recommendations

**Choose Scan Multimeter when:**
- Event rate is low (< 10 events/second)
- Metrics are sampled infrequently (e.g., once per second or less)
- Window size is small (< 10 seconds)
- Memory footprint needs to be minimized (single timeline vs. per-metric timelines)
- Code simplicity is valued

**Choose Stream Multimeter when:**
- Event rate is high (> 100 events/second)
- Metrics are read frequently (e.g., on every event)
- Window size is large (> 60 seconds)
- Consistent latency is critical
- You need predictable O(log n) worst-case performance

**Decision Matrix:**
| Scenario | Event Rate | Window Size | Read Frequency | Recommendation |
|----------|-----------|-------------|----------------|----------------|
| Live trading dashboard | 1000/s | 300s | Every event | **Stream** (451× faster) |
| Hourly reporting | 10/s | 3600s | Once/minute | **Scan** (amortized cost) |
| Real-time monitoring | 100/s | 60s | Every event | **Stream** (consistent) |
| Low-frequency analysis | 1/s | 60s | Once/10s | **Scan** (simpler) |

---

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

### Build Commands

```bash
# Configure and build release
xmake config -m release
xmake build

# Build with debug symbols
xmake config -m releasedbg
xmake build

# Run Coinbase session tests
xmake build coinbase_tests
xmake run coinbase_tests

# Run metrics benchmarks
xmake build multimeter_benchmark
xmake run multimeter_benchmark
```

### Running the Application

Example session test (subscribes to Coinbase BTC-USD ticker):
```bash
xmake build session_test
xmake run session_test
```

The test establishes a WebSocket connection, subscribes to trade data, and logs incoming events for 10 seconds.

### Unit Tests

The metrics system includes comprehensive unit tests:
```bash
xmake build metrics_tests
xmake run metrics_tests
```

Tests cover:
- Sliding window expiration logic
- Median calculation accuracy
- Min/max/mean correctness
- Edge cases (empty windows, single events, etc.)
- Both scan and stream implementations

### Project Structure

```
sparkland/
├── codec/              # JSON encoding/decoding
├── components/         # Reusable session and scheduler
├── core/               # Utilities (assert, result monad)
├── exchange/           # Exchange-specific implementations
│   ├── coinbase/       # Coinbase connector, transformer, session
│   ├── bybit/          # Bybit implementation
│   └── factory/        # Compile-time exchange factory
├── metrics/            # **Sliding window metrics**
│   ├── benchmark/      # Performance benchmarks
│   ├── include/spl/metrics/
│   │   ├── scan/       # O(n log n) recalculative metrics
│   │   └── stream/     # O(log n) incremental metrics
│   └── test/           # Unit tests
├── network/            # WebSocket/TLS/TCP layer
├── protocol/           # Protocol definitions and codecs
│   ├── coinbase/       # Coinbase WebSocket protocol
│   ├── feeder/         # Generic market data protocol
│   └── common/         # Shared types (exchange_id, price, etc.)
├── types/              # Strong types (price, quantity)
└── xmake.lua           # Build configuration
```

---

## References

- **Assignment**: [task.pdf](task.pdf) - HFT Quants Developer home assignment
- **Coinbase API**: [Advanced Trade WebSocket API](https://docs.cdp.coinbase.com/advanced-trade/docs/)
- **Author CV**: [cv.pdf](cv.pdf)

---

## License

This project was created as a technical assessment for Sparkland Trading.

**Author**: Mohammed Boujemaoui Boulaghmoudi
**Contact**: mohabouje@gmail.com
