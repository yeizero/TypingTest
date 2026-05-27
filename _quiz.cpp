#pragma once
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Utf8String {
    string s;
    vector<int> seg;

    void parse(int i) {
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
    }
public:
    class Iter;

    class Slice {
        Utf8String& str;
        int s, e;
        Slice(Utf8String& str, int s, int e) : str(str), s(s), e(e) {}

        friend class Utf8String;
        friend class Iter;

    public:
        Slice slice(int start, int end) {
            return str.slice(s+start, s+end);
        }
        Slice operator[] (int i) {
            return slice(i, i+1);
        }

        friend ostream& operator<<(ostream& os, const Slice& slice) {
            auto& str = slice.str;
            os.write(str.s.data() + str.seg[slice.s],
                     str.seg[slice.e] - str.seg[slice.s]);
            return os;
        }

        int len() {
            return e - s;
        }
 
        Iter begin() {
            return Iter(str, s);
        }
        Iter end() {
            return Iter(str, e);
        }
    };

    class Iter {
        Utf8String& s;
        int i;
        Iter(Utf8String& s, int i) : s(s), i(i) {}

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

    Utf8String() {}

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

    void push_str(const Utf8String& x) {
        seg.reserve(seg.size()+x.seg.size());
        for(int num : x.seg) {
            seg.push_back(s.size()+num);
        }
        s += x.s;
    }

    void pop() {
        s.erase(seg.back());
        seg.pop_back();
    }

    int len() const {
        return seg.size();
    }

    void clear() {
        s.clear();
        seg.clear();
    }

    Slice slice(int start, int end) {
        return Slice(*this, start, end);
    }

    Slice operator[] (int i) {
        return slice(i, i+1);
    }

    Iter begin() {
        return Iter(*this, 0);
    }

    Iter end() {
        return Iter(*this, len());
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

    friend ostream& operator<<(ostream& os, const Utf8String& str) {
        return os<<str.s;
    }
};