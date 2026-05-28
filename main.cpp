#include <fstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <functional>
#include <chrono>
#include "utils.hpp"
using namespace std;

const int LINES_DISPLAY = 5;
const int TIME_LINE_GAP = 1;
// const int BREAKLINE_LEN = 80;
const auto FOCUS_TEXT_COLOR = rang::fgB::magenta;
const auto WRONG_TEXT_COLOR = rang::fgB::red;
// const int INITIAL_TIME = 60;
// const auto WRONG_SPACE_COLOR = rang::bg::red;

void init() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
}

class Instant {
    using Time = chrono::time_point<chrono::steady_clock>;
    Time now() const {
        return chrono::steady_clock::now();
    }
    Time start;
public:
    Instant() : start(now()) {}
    long long elapsed_sec() const {
        return chrono::duration_cast<chrono::seconds>(
            now() - start
        ).count();
    }
    long long elapsed_ms() const {
        return chrono::duration_cast<chrono::milliseconds>(
            now() - start
        ).count();
    }
};

struct ShareInfo {
    mutex mtx;
    const Instant time;
    int end_offset;
    ShareInfo(int end_offset) : end_offset(end_offset) {}
};

void timer_task(ShareInfo& info) {
    while(1) {
        unique_lock<mutex> mtx(info.mtx);
        if(info.end_offset == 0) {
            return;
        }

        cout << ter::hide
             << ter::save_pos
             << ter::move::moveline(info.end_offset);
        for(int i=0; i<TIME_LINE_GAP; i++) 
            cout << '\n';
        cout << (info.time.elapsed_sec())
             << ter::clear::until_new_line
             << ter::restore_pos
             << ter::show
             << flush;

        mtx.unlock();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

Utf8String styled_compare(Utf8String& input, Utf8String::Slice ans, int& wrong_cnt) {
    Utf8String str;
    for(int i=0; i<input.len(); i++) {
        if (i >= ans.len()) {
            wrong_cnt += input.len() - ans.len();
            str << WRONG_TEXT_COLOR 
                << input.slice(i, input.len()) 
                << rang::reset;
            break;
        }
        if (input[i] != ans[i]) {
            wrong_cnt++;
            str << WRONG_TEXT_COLOR;
        }
        str << input[i];
        if (input[i] != ans[i]) {
            str << rang::reset;
        }
    }
    int left = ans.len() - input.len();
    if (left > 0) {
        wrong_cnt += left;
    }
    return str;
}

void word_contribute_count(Utf8String& input, Utf8String::Slice ans, int& cnt) {
    for(int i=0; i<ans.len(); i++) {
        if (i == input.len()) break;
        if (input[i] != ans[i]) continue;
        if (ans[i].bytes_len() == 1) cnt += 1;
        else cnt += 5;
    }
}

int main() {
    init();
    
    vector<Utf8String> lines_raw;
    vector<Utf8String::Slice> lines;

    while(1) {
        cout << "輸入檔案位置：";
        string path;
        getline(cin, path);
        std::ifstream file(path);
        if (file.is_open()) {
            lines_raw.assign(1, Utf8String());
            while(lines_raw.back().getline(file)) {
                lines_raw.emplace_back();
            }
            lines_raw.pop_back();
            
            for(auto& line : lines_raw) {
                auto slice = line.as_slice().trim_end();
                if (slice.len() == 0) continue;
                lines.push_back(slice);   
            }

            cout<<ter::move::prvline(1);
            break;
        } else {
            cout << rang::fg::red 
                 << "檔案無法開啟，請重試。" 
                 << rang::reset 
                 << ter::move::prvline(1) 
                 << ter::clear::current_line;
        }
    }

    vector<Utf8String> inputs(lines.size());

    ShareInfo info(LINES_DISPLAY * 2);
    unique_lock<mutex> mtx(info.mtx, defer_lock);

    thread timer_thread(timer_task, ref(info));
    
    int typed_cnt = 0;
    int wrong_cnt = 0;
    int contribute_cnt = 0;

    for(int i = 0; i < lines.size(); i++) {
        mtx.lock();
        int line_start = max(0, i - 1);
        bool has_prev = i != line_start;
        int offset = LINES_DISPLAY * 2 - 3;
        if (has_prev) offset -= 2;
        cout << ter::hide;
        for(int j = line_start; j < line_start + LINES_DISPLAY; j++) {
            if(j != line_start) {
                if (j == i) { // old input
                    cout << '\n' 
                         << inputs[j-1] 
                         << ter::clear::until_new_line 
                         << '\n';
                } else {
                    cout << '\n' 
                         << ter::clear::current_line 
                         << '\n';
                }
            }
            if (j >= lines.size()) {
                cout << ter::clear::current_line;
                continue;
            }
            if(j == i) {
                cout << FOCUS_TEXT_COLOR;
            } else {
                cout << rang::style::dim;
            }
            cout << lines[j] 
                 << rang::reset 
                 << ter::clear::until_new_line;
        }

        info.end_offset = offset;

        cout << ter::move::moveline(-offset)
             << ter::show
             << flush;

        mtx.unlock();

        Utf8String raw_input;
        raw_input.getline(cin);
        typed_cnt += max(raw_input.len(), lines[i].len());
        inputs[i] = styled_compare(raw_input, lines[i], wrong_cnt);
        word_contribute_count(raw_input, lines[i], contribute_cnt);

        mtx.lock();
        if (i == line_start) cout << ter::move::prvline(2);
        else cout << ter::move::prvline(4);
        cout << flush;
        mtx.unlock();
    }

    mtx.lock();
    info.end_offset = 0;
    mtx.unlock();
    
    int max_time_line = LINES_DISPLAY * 2 - 1 + TIME_LINE_GAP;
    if (lines.size() * 2 <= max_time_line - 1) {
        for(int i=0; i<max_time_line; i++) {
            cout << ter::clear::current_line << '\n';
        }
        cout << ter::move::prvline(max_time_line);
    }
    for(int i=0; i<lines.size(); i++) {
        cout << rang::style::dim
             << lines[i]
             << rang::reset
             << ter::clear::until_new_line
             << '\n'
             << inputs[i]
             << ter::clear::until_new_line
             << '\n';
    }

    cout << "\n[ 成績 ]\n";

    int time_usage = info.time.elapsed_sec();
    cout << "用時: " << time_usage << " 秒\n";

    double mins = info.time.elapsed_ms() / 1000.0 / 60.0;
    cout << "速度: "
         << fixed
         << setprecision(1) 
         << (contribute_cnt / 5.0 / mins) 
         << " WPM\n";

    double accuracy = (double)(typed_cnt - wrong_cnt) * 100 / typed_cnt;
    cout << "正確率: " 
         << fixed
         << setprecision(2)
         << accuracy 
         << "% (錯 "
         << wrong_cnt
         << " 字)";

    timer_thread.join();
}
