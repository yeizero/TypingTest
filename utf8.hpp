#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
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

class Utf8String {
    string s;
    vector<int> seg;

    void parse(int i) {
        if(i) seg.pop_back();
        while (i < s.size()) {
            seg.push_back(i);
            auto ch = (unsigned char)s[i];
            int len;
            if (ch >= 0xf0) len = 4;
            else if (ch >= 0xe0) len = 3;
            else if (ch >= 0xc0) len = 2;
            else len = 1;
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
