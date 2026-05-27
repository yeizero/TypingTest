#include <fstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <functional>
#include <chrono>
#include "utils.hpp"
using namespace std;

const int LINES_DISPLAY = 5;
const int BREAKLINE_LEN = 80;
const auto FOCUS_TEXT_COLOR = rang::fgB::magenta;
const auto WRONG_TEXT_COLOR = rang::fgB::red;
const int INITIAL_TIME = 60;
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
    long long elapsed_ms() const {
        return chrono::duration_cast<chrono::milliseconds>(
            now() - start
        ).count();
    }
};

struct ShareInfo {
    mutex mtx;
    Instant time;
    int lines_left;
};

void timer_task(ShareInfo& info) {
  //thread::
}

// int main() {
//     mutex mtx;
//     int lines_left = 0;
//     thread timer_t(timer_task, ref(mtx), ref(lines_left));
//     timer_t.join();
// }

Utf8String styled_compare(Utf8String& input, Utf8String& ans, int& wrong_cnt) {
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

int main() {
    init();
    
    vector<Utf8String> lines;

    while(1) {
        cout << "輸入檔案位置：";
        string path;
        getline(cin, path);
        std::ifstream file(path);
        if (file.is_open()) {
            lines.assign(1, Utf8String());
            while(lines.back().getline(file)) {
                if(lines.back().len() == 0) continue;
                lines.emplace_back();
            }
            lines.pop_back();
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
    int typed_cnt = 0;
    int wrong_cnt = 0;

    for(int i = 0; i < lines.size(); i++) {
        int lines_cnt = 0;
        int line_start = max(0, i - 1);
        bool has_prev = i != line_start;
        cout << ter::hide;
        for(int j = line_start; j < line_start + LINES_DISPLAY; j++) {
            if(j != line_start) {
                lines_cnt += 2;
                if (j == i) { // prev input
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
        cout << ter::show;

        if(lines_cnt == 0) cout << ter::move::nxtline(1);
        else if (has_prev) cout << ter::move::prvline(lines_cnt - 3);
        else cout << ter::move::prvline(lines_cnt - 1);
 
        Utf8String raw_input;
        raw_input.getline(cin);
        typed_cnt += max(raw_input.len(), lines[i].len());
        inputs[i] = styled_compare(raw_input, lines[i], wrong_cnt);

        if (i == line_start) cout << ter::move::prvline(2);
        else cout << ter::move::prvline(4);
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
    // cout << "速度";
    double accuracy = (double)(typed_cnt - wrong_cnt) * 100 / typed_cnt;
    cout << "正確率: " 
         << fixed
         << setprecision(2)
         << accuracy 
         << "% (錯 "
         << wrong_cnt
         << " 字)";
}
