#pragma once
#include <iostream>
#include <thread>
#include <chrono>
using u32 = uint32_t;

#ifdef _WIN32
    #include <windows.h>
    DWORD oldInputMode, oldOutputMode;
    UINT oldCP;
#else
    #include <termios.h>
    #include <unistd.h>
    struct termios oldt;
#endif

namespace _ansi {

template <typename T>
inline std::ostream& apply(std::ostream& os, T v) {
    return os << "\x1b[" << (int)v << "m";
}

}

namespace rang {

// 樣式設定
enum class style {
    reset     = 0,
    bold      = 1,
    dim       = 2,
    italic    = 3,
    underline = 4,
    blink     = 5,
    reversed  = 7,
    conceal   = 8,
    crossed   = 9
};

// 前景色 (Standard)
enum class fg {
    black   = 30, red     = 31, green   = 32, yellow  = 33,
    blue    = 34, magenta = 35, cyan    = 36, gray    = 37,
    reset   = 39
};

// 背景色 (Standard)
enum class bg {
    black   = 40, red     = 41, green   = 42, yellow  = 43,
    blue    = 44, magenta = 45, cyan    = 46, gray    = 47,
    reset   = 49
};

// 高亮度前景色 (Bright)
enum class fgB {
    black   = 90, red     = 91, green   = 92, yellow  = 93,
    blue    = 94, magenta = 95, cyan    = 96, gray    = 97
};

// 高亮度背景色 (Bright)
enum class bgB {
    black   = 100, red     = 101, green   = 102, yellow  = 103,
    blue    = 104, magenta = 105, cyan    = 106, gray    = 107
};

inline std::ostream& operator<<(std::ostream& os, style v) { return _ansi::apply(os, v); }
inline std::ostream& operator<<(std::ostream& os, fg v)    { return _ansi::apply(os, v); }
inline std::ostream& operator<<(std::ostream& os, bg v)    { return _ansi::apply(os, v); }
inline std::ostream& operator<<(std::ostream& os, fgB v)   { return _ansi::apply(os, v); }
inline std::ostream& operator<<(std::ostream& os, bgB v)   { return _ansi::apply(os, v); }

inline std::ostream& reset(std::ostream& os) {
    return os << style::reset;
}

} // namespace rang

namespace ter {

void sleep_ms(unsigned long long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void gotoxy(int x, int y) {
    std::cout << "\x1b[" << y << ";" << x << "H" << std::flush;
}

enum class cursor_move_dir {
    up = 0,    // A
    down = 1,  // B
    right = 2, // C
    left = 3,  // D,
    nxtline = 4, // E
    prvline = 5, // F
};

struct cursor_move {
    cursor_move_dir dir;
    u32 count;
};

namespace move {
    inline cursor_move up(u32 n)    { return {cursor_move_dir::up, n}; }
    inline cursor_move down(u32 n)  { return {cursor_move_dir::down, n}; }
    inline cursor_move left(u32 n)  { return {cursor_move_dir::left, n}; }
    inline cursor_move right(u32 n) { return {cursor_move_dir::right, n}; }
    inline cursor_move prvline(u32 n) { return {cursor_move_dir::prvline, n}; }
    inline cursor_move nxtline(u32 n) { return {cursor_move_dir::nxtline, n}; }
}

enum class clear {
    current_line = 2,
    until_new_line = 0,
};

inline std::ostream& operator<<(std::ostream& os, const cursor_move& v) {
    if (v.count <= 0) return os;
    return os << "\x1b[" << v.count << (char)('A' + (int)v.dir);
}

inline std::ostream& operator<<(std::ostream& os, const clear& v) {
    return os << "\x1b[" << (int)v << 'K';
}

inline std::ostream& hide(std::ostream& os) {
    return os << "\x1b[?25l";
}

inline std::ostream& show(std::ostream& os) {
    return os << "\x1b[?25h";
}

inline std::ostream& save_pos(std::ostream& os) {
    return os << "\x1b[s";
}

inline std::ostream& restore_pos(std::ostream& os) {
    return os << "\x1b[u";
}

} // namespace ter