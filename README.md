# Sparkland - Cryptocurrency Market Data Aggregation System

**[Mohammed Boujemaoui Boulaghmoudi](cv.pdf)**  
C++ Senior Software Engineer - Quant Developer  
- Github: [mohabouje](https://github.com/mohabouje)
- Web: [mohabouje.github.io](https://mohabouje.github.io)
- Email: mohabouje@gmail.com  
- Phone: (+34) 603 260 806  
- Phone: (+971) 583 084 195

## Performance Analysis: Scan vs Stream Multimeter Implementations

The metrics system provides two distinct implementations for calculating statistical aggregates (min, max, median, mean) over time-windowed data:

1. **Scan Multimeter** (`spl::metrics::scan::multimeter`) - Recalculative approach
2. **Stream Multimeter** (`spl::metrics::stream::multimeter`) - Incremental approach

The following benchmark results demonstrate the performance characteristics across different event rates and window sizes (20,000 samples per test):

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
- **1000 events/sec, 300s window:** Scan = 1,504,849μs vs Stream = 3,333μs (451× faster)
- **Reason:** O(n log n) sorting becomes prohibitively expensive as the window contains more events

#### Window Size Scaling

**Scan Performance Degradation:**
- 1s window: 711μs → 300s window: 40,526μs (57× slower)
- Performance deteriorates dramatically with larger windows due to increased sorting overhead

**Stream Performance Consistency:**
- 1s window: 3,424μs → 300s window: 3,817μs (1.1× slower)
- Minimal performance impact regardless of window size due to logarithmic operations

#### Timeline Implementation

Both implementations utilize `spl::core::timeline<Event>` which provides:
- Automatic expiration of events outside the time window
- Efficient event storage and retrieval
- Thread-safe operations (when configured)
