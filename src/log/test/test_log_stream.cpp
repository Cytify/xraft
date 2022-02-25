#include <assert.h>
#include <limits>
#include <stdint.h>

#include "src/log/log_stream.h"

void test_bool() {
    xlog::LogStream s;
    const util::Buffer<util::kLargeBuffer>& buf = s.get_buffer();
    assert(buf.to_string() == "");
    s << true;
    assert(buf.to_string() == "1");
    s << '\n';
    assert(buf.to_string() == "1\n");
    s << false;
    assert(buf.to_string() == "1\n0");
}

void test_integer() {
    xlog::LogStream s;
    const util::Buffer<util::kLargeBuffer>& buf = s.get_buffer();
    assert(buf.to_string() == "");
    s << 1;
    assert(buf.to_string() == "1");
    s << 0;
    assert(buf.to_string() == "10");
    s << -1;
    assert(buf.to_string() == "10-1");
    s.reset_buffer();
    assert(buf.to_string() == "");

    s << 1 << " " << 999 << " " << 65;
    assert(buf.to_string() == "1 999 65");
}

void test_integer_limit() {
    xlog::LogStream s;
    const util::Buffer<util::kLargeBuffer>& buf = s.get_buffer();
    assert(buf.to_string() == "");

    s << -2147483647;
    assert(buf.to_string() == "-2147483647");

    s << static_cast<int>(-2147483647 - 1);
    assert(buf.to_string() == "-2147483647-2147483648");
    s << ' ';
    s << 2147483647;
    assert(buf.to_string() == ("-2147483647-2147483648 2147483647"));
    s.reset_buffer();

    s << std::numeric_limits<int16_t>::min();
    assert(buf.to_string() == ("-32768"));
    s.reset_buffer();

    s << std::numeric_limits<int16_t>::max();
    assert(buf.to_string() == ("32767"));
    s.reset_buffer();

    s << std::numeric_limits<uint16_t>::min();
    assert(buf.to_string() == ("0"));
    s.reset_buffer();

    s << std::numeric_limits<uint16_t>::max();
    assert(buf.to_string() == ("65535"));
    s.reset_buffer();

    s << std::numeric_limits<int32_t>::min();
    assert(buf.to_string() == ("-2147483648"));
    s.reset_buffer();

    s << std::numeric_limits<int32_t>::max();
    assert(buf.to_string() == ("2147483647"));
    s.reset_buffer();

    s << std::numeric_limits<uint32_t>::min();
    assert(buf.to_string() == ("0"));
    s.reset_buffer();

    s << std::numeric_limits<uint32_t>::max();
    assert(buf.to_string() == ("4294967295"));
    s.reset_buffer();

    s << std::numeric_limits<int64_t>::min();
    assert(buf.to_string() == ("-9223372036854775808"));
    s.reset_buffer();

    s << std::numeric_limits<int64_t>::max();
    assert(buf.to_string() == ("9223372036854775807"));
    s.reset_buffer();

    s << std::numeric_limits<uint64_t>::min();
    assert(buf.to_string() == ("0"));
    s.reset_buffer();

    s << std::numeric_limits<uint64_t>::max();
    assert(buf.to_string() == ("18446744073709551615"));
    s.reset_buffer();

    int16_t a = 0;
    int32_t b = 0;
    int64_t c = 0;
    s << a;
    s << b;
    s << c;
    assert(buf.to_string() == ("000"));
}

void test_float() {
    xlog::LogStream s;
    const util::Buffer<util::kLargeBuffer>& buf = s.get_buffer();
    assert(buf.to_string() == "");

    s << 0.0;
    assert(buf.to_string() == ("0"));
    s.reset_buffer();

    s << 1.0;
    assert(buf.to_string() == ("1"));
    s.reset_buffer();

    s << 0.1;
    assert(buf.to_string() == ("0.1"));
    s.reset_buffer();

    s << 0.05;
    assert(buf.to_string() == ("0.05"));
    s.reset_buffer();

    s << 0.15;
    assert(buf.to_string() == ("0.15"));
    s.reset_buffer();

    double a = 0.1;
    s << a;
    assert(buf.to_string() == ("0.1"));
    s.reset_buffer();

    double b = 0.05;
    s << b;
    assert(buf.to_string() == ("0.05"));
    s.reset_buffer();

    double c = 0.15;
    s << c;
    assert(buf.to_string() == ("0.15"));
    s.reset_buffer();

    s << a + b;
    assert(buf.to_string() == ("0.15"));
    s.reset_buffer();

    assert(a + b != c);

    s << 1.23456789;
    assert(buf.to_string() == ("1.23456789"));
    s.reset_buffer();

    s << 1.234567;
    assert(buf.to_string() == ("1.234567"));
    s.reset_buffer();

    s << -123.456;
    assert(buf.to_string() == ("-123.456"));
    s.reset_buffer();
}

void test_string() {
    xlog::LogStream s;
    const util::Buffer<util::kLargeBuffer>& buf = s.get_buffer();
    assert(buf.to_string() == "");

    s << "Hello ";
    assert(buf.to_string() == ("Hello "));

    std::string str = "asdasdasd";
    s << str;
    assert(buf.to_string() == ("Hello asdasdasd"));
}

void test_long() {
    xlog::LogStream s;
    const util::Buffer<util::kLargeBuffer>& buf = s.get_buffer();
    assert(buf.to_string() == "");

    for (int i = 0; i < 399 * 1000; ++i) {
        s << "123456789 ";
        assert(buf.get_length() == 10 * (i + 1));
        assert(buf.avail() == 4000000 - 10 * (i + 1));
    }

    s << "abcdefghi ";
    assert(buf.get_length() == 3990010);
    assert(buf.avail() == 9990);

    s << "abcdefghi";
    assert(buf.get_length() == 3990019);
    assert(buf.avail() == 9981);
}

int main() {
    test_bool();
    test_integer();
    test_integer_limit();
    test_float();
    test_string();
    test_long();
}