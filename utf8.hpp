#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <string_view>
#include <cassert>
#include <cstdint> 

#define _utf8_impl_concat(t) \
Utf8String& operator<<(const t x) { \
    push_str(x); \
    return *this; \
} \
Utf8String& operator+=(const t x) { \
    push_str(x); \
    return *this; \
}

using namespace std;
using u8 = uint8_t;
using u32 = uint32_t;

namespace utf8util {
    int expect_len(unsigned char ch) {
        if (ch < 128) return 1;
        if ((ch >> 5) == 0b110) return 2;
        if ((ch >> 4) == 0b1110) return 3;
        if ((ch >> 3) == 0b11110) return 4;
        return -1;
    }

    bool is_ascii_space(char ch) {
        return ch == ' ' || (ch >= '\x09' && ch <= '\x0d' );
    }

    inline constexpr u8 WHITESPACE_MAP[256] = {
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    bool lookup_space(u32 ch) {
        assert(ch > 127);
        switch (ch >> 8) {
            case 0: return (WHITESPACE_MAP[ch & 0xff] & 1) != 0;
            case 22: return ch == 0x1680;
            case 32: return (WHITESPACE_MAP[ch & 0xff] & 2) != 0;
            case 48: return ch == 0x3000;
            default: return false;
        };
    }
};

class Utf8String {
    string s;
    vector<int> seg;

    void parse(int i) {
        if(i) seg.pop_back();
        while (i < s.size()) {
            seg.push_back(i);
            int len = utf8util::expect_len(s[i]);
            // NOTE: invalid utf8
            if (len == -1) len = 1;
            i += len;
        }
        seg.push_back(s.size());
    }
public:
    class Iter;

    class Slice {
        const Utf8String& str;
        int s, e;
        Slice(const Utf8String& str, int s, int e) : str(str), s(s), e(e) {}

        friend class Utf8String;
        friend class Iter;

        public:
        Slice slice(int start, int end) const {
            return str.slice(s+start, s+end);
        }
        Slice operator[] (int i) const {
            return slice(i, i+1);
        }
        string_view as_view() const {
            return string_view(
                str.s.data() + str.seg[s],
                bytes_len()
            );
        }
        
        friend ostream& operator<<(ostream& os, const Slice& slice) {
            return os << slice.as_view();
        }
        
        int bytes_len() const {
            return str.seg[e] - str.seg[s];
        }

        int len() const {
            return e - s;
        }
 
        Iter begin() const {
            return Iter(str, s);
        }
        Iter end() const {
            return Iter(str, e);
        }

        bool operator==(const Slice& other) const { 
            return as_view() == other.as_view();
        }

        bool operator!=(const Slice& other) const {
            return !(*this == other);
        } 

        u32 first_as_char() const {
            auto p = (const unsigned char*)&str.s[str.seg[s]];
            int len = utf8util::expect_len(*p);
            if (len == 1) return p[0];
            // NOTE: invalid utf8
            if (len == -1) return 0;
            u32 x = p[0] & (0xFF >> len);
            for (int i = 1; i < len; ++i) {
                x = (x << 6) | (p[i] & ((1 << 6) - 1));
            }
            return x;
        }

        bool first_is_whitespace() const {
            if ((u8)str.s[str.seg[s]] < 128) return utf8util::is_ascii_space(str.s[str.seg[s]]);
            return utf8util::lookup_space(first_as_char());
        }

        Slice trim_start() {
            for(int i=0; i<len(); i++) {
                if (!(*this)[i].first_is_whitespace()) {
                    return slice(i, len());
                }
            }
            return Slice(str, 0, 0);
        }

        Slice trim_end() {
            for(int i=len()-1; i>=0; i--) {
                if (!(*this)[i].first_is_whitespace()) {
                    return slice(0, i + 1);
                }
            }
            return Slice(str, 0, 0);
        }

        Slice trim() {
            return trim_start().trim_end();
        }
    };

    class Iter {
        const Utf8String& s;
        int i;
        Iter(const Utf8String& s, int i) : s(s), i(i) {}

        friend class Utf8String;
        friend class Slice;
    public:
        Slice operator*() {
            return Slice(s, i, i+1);
        }
        Iter& operator++() {
            i++;
            return *this;
        }
        bool operator!=(const Iter& other) const {
            return i != other.i;
        }
    };

    Utf8String() : seg({0}) {}

    Utf8String(const char* input) : s(input) {
        parse(0);
    }

    Utf8String(string input) : s(move(input)) {
        parse(0);
    }

    void push_str(const char* input) {
        int i = s.size();
        s += input;
        parse(i);
    }

    void push_str(const string& x) {
        int i = s.size();
        s += x;
        parse(i);
    }

    void push_str(const Slice x) {
        int start_byte = x.str.seg[x.s];
        int offset = s.size();
        seg.reserve(seg.size() + x.len());
        for (int i = x.s + 1; i <= x.e; i++) {
            seg.push_back(offset + (x.str.seg[i] - start_byte));
        }
        s.append(x.str.s, start_byte, x.bytes_len());
    }

    void push_str(const Utf8String& x) {
        push_str(x.as_slice());
    }

    void pop() {
        seg.pop_back();
        s.erase(seg.back());
    }

    int bytes_len() const {
        return s.size();
    }

    int len() const {
        return seg.size() - 1;
    }

    void clear() {
        s.clear();
        seg.resize(1);
        seg[0] = 0;
    }

    Slice slice(int start, int end) const {
        return Slice(*this, start, end);
    }

    Slice as_slice() const {
        return slice(0, len());
    }

    Slice operator[] (int i) const {
        return slice(i, i+1);
    }

    Iter begin() const {
        return Iter(*this, 0);
    }

    Iter end() const {
        return Iter(*this, len());
    }

    bool operator==(const Utf8String& other) const {
        return s == other.s;
    }
    bool operator!=(const Utf8String& other) const {
        return s != other.s;
    }
    bool operator==(const Slice& other) const {
        return slice(0, len()) == other;
    }
    bool operator!=(const Slice& other) const {
        return !(*this == other);
    }

    friend istream& operator>>(istream& is, Utf8String& str) {
        is>>str.s;
        str.seg.clear();
        str.parse(0);
        return is;
    }

    istream& getline(istream& is) {
        std::getline(is, s);
        seg.clear();
        parse(0);
        return is;
    }

    _utf8_impl_concat(char*)
    _utf8_impl_concat(string&)
    _utf8_impl_concat(Utf8String&)
    _utf8_impl_concat(Slice)

    friend ostream& operator<<(ostream& os, const Utf8String& str) {
        return os<<str.s;
    }

    template <typename T>
    Utf8String& operator<<(const T& value) {
        ostringstream oss;
        oss << value;
        push_str(oss.str());
        return *this;
    }
};
