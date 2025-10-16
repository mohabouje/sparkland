#include "spl/candlestick/generator/quantity.hpp"
#include "spl/candlestick/generator/nominal.hpp"
#include "spl/candlestick/generator/tick.hpp"
#include "spl/candlestick/generator/time.hpp"

#include <gtest/gtest.h>

constexpr auto trades = std::array{
    spl::protocol::feeder::trade::trade_summary{
                                                .instrument_id = spl::protocol::common::instrument_id(),
                                                .exchange_id   = spl::protocol::common::exchange_id::binance,
                                                .side          = spl::protocol::common::aggressor_side::sell,
                                                .price         = 102411.46_p,
                                                .quantity      = 0.00266_q,
                                                .condition     = spl::protocol::common::trade_condition::regular,
                                                .timestamp     = spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368007812294400)),
                                                },
    spl::protocol::feeder::trade::trade_summary{
                                                .instrument_id = spl::protocol::common::instrument_id(),
                                                .exchange_id   = spl::protocol::common::exchange_id::binance,
                                                .side          = spl::protocol::common::aggressor_side::buy,
                                                .price         = 102411.47_p,
                                                .quantity      = 0.00179_q,
                                                .condition     = spl::protocol::common::trade_condition::regular,
                                                .timestamp     = spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368007904142336)),
                                                },
    spl::protocol::feeder::trade::trade_summary{
                                                .instrument_id = spl::protocol::common::instrument_id(),
                                                .exchange_id   = spl::protocol::common::exchange_id::binance,
                                                .side          = spl::protocol::common::aggressor_side::sell,
                                                .price         = 102411.46_p,
                                                .quantity      = 0.00017_q,
                                                .condition     = spl::protocol::common::trade_condition::regular,
                                                .timestamp     = spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368008812308480)),
                                                },
    spl::protocol::feeder::trade::trade_summary{
                                                .instrument_id = spl::protocol::common::instrument_id(),
                                                .exchange_id   = spl::protocol::common::exchange_id::binance,
                                                .side          = spl::protocol::common::aggressor_side::sell,
                                                .price         = 102411.46_p,
                                                .quantity      = 0.00266_q,
                                                .condition     = spl::protocol::common::trade_condition::regular,
                                                .timestamp     = spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009812308480)),
                                                },
    spl::protocol::feeder::trade::trade_summary{
                                                .instrument_id = spl::protocol::common::instrument_id(),
                                                .exchange_id   = spl::protocol::common::exchange_id::binance,
                                                .side          = spl::protocol::common::aggressor_side::buy,
                                                .price         = 102411.47_p,
                                                .quantity      = 0.00633_q,
                                                .condition     = spl::protocol::common::trade_condition::regular,
                                                .timestamp     = spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009822308480)),
                                                },
    spl::protocol::feeder::trade::trade_summary{
                                                .instrument_id = spl::protocol::common::instrument_id(),
                                                .exchange_id   = spl::protocol::common::exchange_id::binance,
                                                .side          = spl::protocol::common::aggressor_side::buy,
                                                .price         = 102411.47_p,
                                                .quantity      = 0.00098_q,
                                                .condition     = spl::protocol::common::trade_condition::regular,
                                                .timestamp     = spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009832308480)),
                                                },
};

TEST(CandlestickGeneratorTest, TimeBasedGeneratorRealData) {
    auto const period  = std::chrono::nanoseconds(1'000'000'000);
    auto generator     = spl::candlestick::generator<spl::candlestick::type::time>(period);
    auto collection    = std::vector<spl::protocol::feeder::candlestick::candlestick>{};
    auto const functor = [&](auto&& object) -> spl::result<void> {
        spl::logger::info("{}", object);
        collection.emplace_back(object);
        return spl::success();
    };
    for (auto const& trade : trades) {
        auto const result = generator(trade, functor);
        EXPECT_TRUE(result) << result.error().message().data();
    }

    EXPECT_EQ(collection.size(), 2);

    EXPECT_EQ(collection[0].exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(collection[0].instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(collection[0].open, 102411.46_p);
    EXPECT_EQ(collection[0].high, 102411.47_p);
    EXPECT_EQ(collection[0].low, 102411.46_p);
    EXPECT_EQ(collection[0].close, 102411.47_p);
    EXPECT_NEAR(collection[0].volume, 0.00445_q, spl::math::EPSILON);
    EXPECT_EQ(collection[0].starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368007812294400)));
    EXPECT_EQ(collection[0].ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368007904142336)));
    EXPECT_TRUE(collection[0].complete);

    EXPECT_EQ(collection[1].exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(collection[1].instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(collection[1].open, 102411.46_p);
    EXPECT_EQ(collection[1].high, 102411.46_p);
    EXPECT_EQ(collection[1].low, 102411.46_p);
    EXPECT_EQ(collection[1].close, 102411.46_p);
    EXPECT_EQ(collection[1].volume, 0.00017000000000000001_q);
    EXPECT_EQ(collection[1].starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368008812308480)));
    EXPECT_EQ(collection[1].ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368008812308480)));
    EXPECT_TRUE(collection[1].complete);

    auto const& current = *generator.get();
    EXPECT_EQ(current.exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(current.instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(current.open, 102411.46_p);
    EXPECT_EQ(current.high, 102411.47_p);
    EXPECT_EQ(current.low, 102411.46_p);
    EXPECT_EQ(current.close, 102411.47_p);
    EXPECT_EQ(current.volume, 0.0099699999999999997_q);
    EXPECT_EQ(current.starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009812308480)));
    EXPECT_EQ(current.ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009832308480)));
    EXPECT_FALSE(current.complete);
}

TEST(CandlestickGeneratorTest, VolumeBasedGeneratorRealData) {
    auto generator     = spl::candlestick::generator<spl::candlestick::type::quantity>(0.006_q);
    auto collection    = std::vector<spl::protocol::feeder::candlestick::candlestick>{};
    auto const functor = [&](auto&& object) -> spl::result<void> {
        spl::logger::info("{}", object);
        collection.emplace_back(object);
        return spl::success();
    };
    for (auto const& trade : trades) {
        auto const result = generator(trade, functor);
        EXPECT_TRUE(result) << result.error().message().data();
    }

    EXPECT_EQ(collection.size(), 2);

    EXPECT_EQ(collection[0].exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(collection[0].instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(collection[0].open, 102411.46_p);
    EXPECT_EQ(collection[0].high, 102411.47_p);
    EXPECT_EQ(collection[0].low, 102411.46_p);
    EXPECT_EQ(collection[0].close, 102411.46_p);
    EXPECT_EQ(collection[0].volume, 0.006_q);
    EXPECT_EQ(collection[0].starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368007812294400)));
    EXPECT_EQ(collection[0].ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009812308480)));
    EXPECT_TRUE(collection[0].complete);

    EXPECT_EQ(collection[1].exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(collection[1].instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(collection[1].open, 102411.46_p);
    EXPECT_EQ(collection[1].high, 102411.47_p);
    EXPECT_EQ(collection[1].low, 102411.46_p);
    EXPECT_EQ(collection[1].close, 102411.47_p);
    EXPECT_EQ(collection[1].volume, 0.006_q);
    EXPECT_EQ(collection[1].starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009812308480)));
    EXPECT_EQ(collection[1].ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009822308480)));
    EXPECT_TRUE(collection[1].complete);

    auto const& current = *generator.get();
    EXPECT_EQ(current.exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(current.instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(current.open, 102411.47_p);
    EXPECT_EQ(current.high, 102411.47_p);
    EXPECT_EQ(current.low, 102411.47_p);
    EXPECT_EQ(current.close, 102411.47_p);
    EXPECT_EQ(current.volume, 0.0025899999999999994_q);
    EXPECT_EQ(current.starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009822308480)));
    EXPECT_EQ(current.ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009832308480)));
    EXPECT_FALSE(current.complete);
}

TEST(CandlestickGeneratorTest, NominalBasedGeneratorRealData) {
    auto generator     = spl::candlestick::generator<spl::candlestick::type::nominal>(615.00_p);
    auto collection    = std::vector<spl::protocol::feeder::candlestick::candlestick>{};
    auto const functor = [&](auto&& object) -> spl::result<void> {
        spl::logger::info("{}", object);
        collection.emplace_back(object);
        return spl::success();
    };
    for (auto const& trade : trades) {
        auto const result = generator(trade, functor);
        EXPECT_TRUE(result) << result.error().message().data();
    }

    EXPECT_EQ(collection.size(), 2);

    EXPECT_EQ(collection[0].exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(collection[0].instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(collection[0].open, 102411.46_p);
    EXPECT_EQ(collection[0].high, 102411.47_p);
    EXPECT_EQ(collection[0].low, 102411.46_p);
    EXPECT_EQ(collection[0].close, 102411.46_p);
    EXPECT_NEAR(collection[0].volume, 0.0060051871353069275_q, spl::math::EPSILON);
    EXPECT_EQ(collection[0].starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368007812294400)));
    EXPECT_EQ(collection[0].ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009812308480)));
    EXPECT_TRUE(collection[0].complete);

    EXPECT_EQ(collection[1].exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(collection[1].instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(collection[1].open, 102411.46_p);
    EXPECT_EQ(collection[1].high, 102411.47_p);
    EXPECT_EQ(collection[1].low, 102411.46_p);
    EXPECT_EQ(collection[1].close, 102411.47_p);
    EXPECT_EQ(collection[1].volume, 0.0060051868481931624_q);
    EXPECT_EQ(collection[1].starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009812308480)));
    EXPECT_EQ(collection[1].ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009822308480)));
    EXPECT_TRUE(collection[1].complete);

    auto const& current = *generator.get();
    EXPECT_EQ(current.exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(current.instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(current.open, 102411.47_p);
    EXPECT_EQ(current.high, 102411.47_p);
    EXPECT_EQ(current.low, 102411.47_p);
    EXPECT_EQ(current.close, 102411.47_p);
    EXPECT_EQ(current.volume, 0.0025796260164999097_q);
    EXPECT_EQ(current.starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009822308480)));
    EXPECT_EQ(current.ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009832308480)));
    EXPECT_FALSE(current.complete);
}

TEST(CandlestickGeneratorTest, TickBasedGeneratorRealData) {
    auto generator     = spl::candlestick::generator<spl::candlestick::type::tick>(3);
    auto collection    = std::vector<spl::protocol::feeder::candlestick::candlestick>{};
    auto const functor = [&](auto&& object) -> spl::result<void> {
        spl::logger::info("{}", object);
        collection.emplace_back(object);
        return spl::success();
    };
    for (auto const& trade : trades) {
        auto const result = generator(trade, functor);
        EXPECT_TRUE(result) << result.error().message().data();
    }

    EXPECT_EQ(collection.size(), 2);

    EXPECT_EQ(collection[0].exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(collection[0].instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(collection[0].open, 102411.46_p);
    EXPECT_EQ(collection[0].high, 102411.47_p);
    EXPECT_EQ(collection[0].low, 102411.46_p);
    EXPECT_EQ(collection[0].close, 102411.46_p);
    EXPECT_NEAR(collection[0].volume, 0.00462_q, spl::math::EPSILON);
    EXPECT_EQ(collection[0].starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368007812294400)));
    EXPECT_EQ(collection[0].ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368008812308480)));
    EXPECT_TRUE(collection[0].complete);

    EXPECT_EQ(collection[1].exchange_id, spl::protocol::common::exchange_id::binance);
    EXPECT_EQ(collection[1].instrument_id, spl::protocol::common::instrument_id());
    EXPECT_EQ(collection[1].open, 102411.46_p);
    EXPECT_EQ(collection[1].high, 102411.47_p);
    EXPECT_EQ(collection[1].low, 102411.46_p);
    EXPECT_EQ(collection[1].close, 102411.47_p);
    EXPECT_EQ(collection[1].volume, 0.0099699999999999997_q);
    EXPECT_EQ(collection[1].starting, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009812308480)));
    EXPECT_EQ(collection[1].ending, spl::protocol::common::timestamp(std::chrono::nanoseconds(1738368009832308480)));
    EXPECT_TRUE(collection[1].complete);

    EXPECT_FALSE(generator.get());
}
