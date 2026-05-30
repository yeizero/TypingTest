# TypingTest

期末專題，簡易打字遊戲。中英文皆支援。

## 操作方式

支援版本: 至少 C++ 17

編譯 `main.cpp` -> 執行程式 -> 輸入文字檔案的絕對路徑 -> 開始打字 -> 打完自動計算成績

## 測試資料

`data1.txt`: 簡短英文，同時展示trim末空格

`data2.txt`: 英文文章

`data3.txt`: 中文文章

`data4.bin`: 非UTF-8數據，預計拒絕選擇

## 使用技術

### UTF-8 解析

由於C++標準庫無UTF-8相關功能，在 `utf8.hpp` 實作需要用到的功能，包括utf8驗證、空格字元判斷、迭代器、無複製引用。部分實作參考rust標準庫。

### ANSI跳脫序列

在 `ansi.cpp` 包含ansi跳脫序列相關程式，rang是顏色相關，修改於[rang](https://github.com/agauniyal/rang/blob/master/include/rang.hpp)，移除了向後兼容；
ter用於控制游標以及畫面控制。

### 時間測量
class Instant使用steady_clock計時。

### 多執行緒
由於cin只有同步函數，使用thread在另一個執行緒計時，透過mutex避免競態條件和保護cout順序正確。

