## 2026q1 Homework2 (stdc)
contributed by < `polun15532` >

{%hackmd NrmQUGbRQWemgwPfhzXj6g %}

## 細讀〈[你所不知道的 C 語言：數值系統篇](https://hackmd.io/@sysprog/c-numerics)〉
- [ ] 教材以 `0.1 + 0.2 ≠ 0.3` 作為引言，說明電腦的數值表示問題，援引 IEEE 754 規格和你在電腦上的實驗，充分回答以下:
    * 為何十進位的 `0.1` 無法在二進位浮點數中精確表示？用 Graphviz 或類似的向量繪圖表示法，清晰展現數值表示的過程和限制
    * IEEE-754 單精度或雙精度為例，說明 sign, exponent, mantissa 在表示 `0.1` 時會發生什麼情況。
    * 文中指出浮點數加法不具有結合律，從 Linux 核心的原始程式碼 (事實上使用定點數，但具備浮點數運算的特性) git log 找出對應的浮點數運算考量並充分討論

1. IEEE 754-1985 §3.1 Sets of Values 寫道：
>Within each format only the following entities shall be provided:
>Numbers of the form $(-1)^s 2^E (b_0 \cdot b_1 b_2 \ldots b_{p-1})$
因此，任何可被某個 binary floating-point number 精確表示的數字，都可寫成 $\frac{N}{2^k}$，必然存在整數 $k$ 使得 $x \times 2^k$ 為整數。

以 $0.1 = \frac{1}{10}$ 為例，若它可以被精確表示，則存在整數 $k$ 使得 $\frac{2^k}{10}$ 為整數。但 $10 = 2 \times 5$，且 $\gcd(2, 5) = 1$，因此分母中的質數 $5$ 不可能被任何 $2^k$ 消去。所以 $0.1$ 無法被任何有限位數的浮點數精確表示，只能依 IEEE 754 的捨入規則儲存為最接近的可表示值。

同理，$0.2 = \frac{1}{5}$ 與 $0.3 = \frac{3}{10}$ 也都無法被有限浮點數精確表示。
TODO:思考怎麼畫圖表達

2. IEEE 754-1985 §3.2 Basic Formats：
>Numbers in the single and double formats are composed of the following three fields:
>1) 1-bit sign s
>2) Biased exponent e = E+bias
>3) Fraction f = . b_1b_2 ... b_{p-1}

我使用 `gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0` 列印 $0.1$ 在 `float` 與 `double` 下的實際表示：

```text
float  0.1f
  hex      : 0x3dcccccd
  bits     : 0 01111011 10011001100110011001101
  sign     : 0
  exponent : 123 (actual -4)
  fraction : 0x4ccccd

double 0.1
  hex      : 0x3fb999999999999a
  bits     : 0 01111111011 1001100110011001100110011001100110011001100110011010
  sign     : 0
  exponent : 1019 (actual -4)
  fraction : 0x999999999999a
```

由輸出可直接看出：`0.1` 的 `sign` 為 $0$，實際 exponent 為 `-4`；單精度與雙精度的差別主要在於 fraction 欄位長度不同，因此雙精度只能把近似值做得更接近，仍然不是精確表示。

IEEE 754-1985 §4.1 Round to Nearest：
>An implementation of this standard shall provide round to nearest as the default rounding mode.
>In this mode the representable value nearest to the infinitely precise result shall be delivered;
>if the two nearest representable values are equally near, the one with its least significant bit zero shall be delivered.

假設一個數字無法被浮點數精確表示，預設規則會先選擇最接近的可表示值，這是為了避免固定向上或向下捨入造成單向誤差。當兩個可表示值同樣接近時，才進一步選擇最低有效位元為 $0$ 的那一個。
[IEEE Standard 754 for Binary Floating-Point Arithmetic](https://people.eecs.berkeley.edu/~wkahan/ieee754status/IEEE754.PDF) 指出：
>Half-way cases are rounded to Nearest Even, which means that the neighbor with last digit 0 is chosen.
>Besides its lack of statistical bias, this choice has a subtle advantage; it prevents prolonged drift during slowly convergent iterations containing steps like these$
>While(...) do { y:=x+z; ...; x:=y-z}

至於平手時最低有效位為什麼選擇 $0$ 而非 $1$，我目前沒有答案。
選 $0$ 是否代表在某些統計上 $0$ 比較穩定較不容易 drift？

- [ ] 針對 balanced ternary 和 radix economy
    * 電腦科學家 Donald E. Knuth 在《The Art of Computer Programming》第 2 卷說 "Perhaps the prettiest number system of all is the balanced ternary notation"，其背景考量是什麼？Knuth 是否有對應的著作進一步探討？
    * 為何 balanced ternary 中求負數只需「符號反轉」？與二補數 (two's complement) 表示法相比，分析計算成本、硬體實作，和數值對稱性。建立數學模型並討論
    * 說明為什麼低位元量化 (例如 ternary / 1-bit LLM) 在 AI 硬體中具有潛在優勢。參照提及的論文，描述其關鍵考量 $\to$ 歡迎協作 [BitMamba.c](https://github.com/jserv/bitmamba.c)
- [ ]  算術運算可完全以數位邏輯實作，分析 $x + y = (x \oplus y) + ((x \& y) << 1)$ 並回答：
    * 參閱數位邏輯教科書 (善用圖書館資源)，在硬體加法器中 `x ^ y` 和 `x & y` 各代表什麼訊號？並摘錄書中對應的描述，探討其應用場景
    * 為何 `(x+y)/2` 可能造成 overflow？又為何 $(x \& y) + ((x \oplus y) >> 1)$ 可避免 overflow
    * 在 Linux 核心中，為何這類 bit-level reasoning 對於效能與正確性非常重要？在 git log 找出對應的改進和修正
- [ ]  `x & (x - 1)` 可用來檢測什麼數值性質？給出數學推導，說明為何此技巧成立。Linux 核心中有哪些場景會利用 power-of-two (2 的冪，[不是「冪次」](https://hackmd.io/@sysprog/it-vocabulary)) 性質？為何 power-of-two 對於系統效能特別重要？
- [ ] `((X) - 0x01010101) & ~(X) & 0x80808080` 技巧可用於 `strlen()` 的實作，回答：
    * 該巨集偵測的是什麼資料？為何該運算可一次檢測 4 個位元組？為何這比逐 byte 檢查更有效率？
    * 在 Linux 核心原始程式碼中找出類似 word-at-a-time 手法的案例，並充分進行效能分析
- [ ]  教材提及若干真實案例，以 Boeing 787 的[軟體缺失案例](https://hackmd.io/@sysprog/software-failure)來說，為何 32 位元計數器在約 248 天會 overflow？參照 FAA (Federal Aviation Administration ) 和相關官方事故分析報告進行探討
    * 在 Linux 核心的 git log 找出類似的 integer overflow 案例並探討
    * 在 C 語言規格書列舉相關整數範圍的規範和實作考量


## Reference
[IEEE Standard for Binary Floating-Point
Arithmetic](https://www.ime.unicamp.br/~biloti/download/ieee_754-1985.pdf)
[IEEE Standard 754 for Binary Floating-Point Arithmetic](https://people.eecs.berkeley.edu/~wkahan/ieee754status/IEEE754.PDF)
