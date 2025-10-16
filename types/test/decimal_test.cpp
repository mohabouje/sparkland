#include "spl/types/decimal.hpp"

using dec6 = spl::types::decimal<6>;
using dec8 = spl::types::decimal<6>;

// Conversion tests
static_assert(dec6::from(0.1).to<dec8>() == dec8::from(0.1));
static_assert(dec8::from(0.1).to<dec6>() == dec6::from(0.1));
static_assert(dec8::from(0.1).to<dec6>() * dec6::from(10.0) == dec8::from(1.0));

// Multiplication tests
static_assert((dec6::from(0.1) * dec6::from(0.2)) == dec6::from(0.02));
static_assert((dec6::from(0.3) * dec6::from(0.3)) == dec6::from(0.09));
static_assert((dec6::from(1.234567) * dec6::from(7.654321)) == dec6::from(9.449772));
static_assert((dec6::from(99999.999999) * dec6::from(0.000001)) == dec6::from(0.099999));
static_assert((dec6::from(0.000001) * dec6::from(0.000001)) == dec6::from(0.0));
static_assert((dec6::from(123456.789123) * dec6::from(0.000009)) == dec6::from(1.111111));

// Division tests
static_assert((dec6::from(0.1) / dec6::from(0.2)) == dec6::from(0.5));
static_assert((dec6::from(0.09) / dec6::from(0.3)) == dec6::from(0.3));
static_assert((dec6::from(9.449772) / dec6::from(7.654321)) == dec6::from(1.234566));
static_assert((dec6::from(1.0) / dec6::from(3.0)) == dec6::from(0.333333));
static_assert((dec6::from(1.0) / dec6::from(7.0)) == dec6::from(0.142857));
static_assert((dec6::from(0.000001) / dec6::from(99999.999999)) == dec6::from(0.0));

static_assert(dec6::from("0.000001") == dec6::from(0.000001));
static_assert(dec6::from("999999.999999") == dec6::from(999999.999999));
static_assert(dec6::from("-999999.999999") == dec6::from(-999999.999999));
static_assert(dec6::from("123456.789123") == dec6::from(123456.789123));
static_assert(dec6::from("-123456.789123") == dec6::from(-123456.789123));
static_assert(dec6::from("1.0") == dec6::from(1.0));
static_assert(dec6::from("-1.0") == dec6::from(-1.0));
static_assert(dec6::from("0.0") == dec6::from(0.0));
static_assert(dec6::from("-0.0") == dec6::from(0.0)); // Neg-zero is still zero
static_assert(dec6::from("42.") == dec6::from(42.0));
static_assert(dec6::from("-42.") == dec6::from(-42.0));
static_assert(dec6::from(".123456") == dec6::from(0.123456));
static_assert(dec6::from("-.123456") == dec6::from(-0.123456));
static_assert(dec6::from("1.") == dec6::from(1.0));
static_assert(dec6::from("-1.") == dec6::from(-1.0));
static_assert(dec6::from("0.999999") == dec6::from(0.999999));
static_assert(dec6::from("-0.999999") == dec6::from(-0.999999));
static_assert(dec6::from("42") == dec6::from(42.0));
static_assert(dec6::from("-42") == dec6::from(-42.0));
static_assert(dec6::from("-42.150000") == dec6::from(-42.15));

static_assert(dec6::from("0.0").padded() == "0.000000");
static_assert(dec6::from("1.0").padded() == "1.000000");
static_assert(dec6::from("1.000001").padded() == "1.000001");
static_assert(dec6::from("1.000010").padded() == "1.000010");
static_assert(dec6::from("1.000100").padded() == "1.000100");
static_assert(dec6::from("1.001000").padded() == "1.001000");
static_assert(dec6::from("1.010000").padded() == "1.010000");
static_assert(dec6::from("1.100000").padded() == "1.100000");

static_assert(dec6::from("-0.0").padded() == "0.000000");
static_assert(dec6::from("-1.0").padded() == "-1.000000");
static_assert(dec6::from("-1.000001").padded() == "-1.000001");
static_assert(dec6::from("-1.100000").padded() == "-1.100000");

static_assert(dec6::from("1050.0000015482160").padded() == "1050.000001");
static_assert(dec6::from("-1050.000001548216").padded() == "-1050.000001");
static_assert(dec6::from("-1050.154821600000").padded() == "-1050.154821");
static_assert(dec6::from("999.999999").padded() == "999.999999");
static_assert(dec6::from("999.999990").padded() == "999.999990");
static_assert(dec6::from("999.999900").padded() == "999.999900");

static_assert(dec6::from("0.0").to_string() == "0.0");
static_assert(dec6::from("1.0").to_string() == "1.0");
static_assert(dec6::from("1.000001").to_string() == "1.000001");
static_assert(dec6::from("1.000010").to_string() == "1.00001");
static_assert(dec6::from("1.000100").to_string() == "1.0001");
static_assert(dec6::from("1.001000").to_string() == "1.001");
static_assert(dec6::from("1.010000").to_string() == "1.01");
static_assert(dec6::from("1.100000").to_string() == "1.1");
static_assert(dec6::zero().to_string() == "0.0");
static_assert(dec6::one().to_string() == "1.0");

auto main() -> int {
    return 0;
}
