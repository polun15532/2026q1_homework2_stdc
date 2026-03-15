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
至於平手時最低有效位為什麼選擇 $0$ 而非 $1$，Nicholas J. Higham 在《Accuracy and Stability of Numerical Algorithms》第 2 章（p.54）給出答案：
>For bases 2 and 10 rounding to even is preferred to rounding to odd. After rounding to even a subsequent rounding to one less place does not involve a tie.

選擇 rounding to even 時當精度在縮掉一位，不會再次遇到平手，Higham 給了以下例子：
```
rounding to even:
2.445 -> 2.44 -> 2.4

rounding to odd:
2.445 -> 2.45 -> 2.5
```
可觀查到在這個例子中，平手時向 even 捨入遇到 1 次平手，向 odd 捨入則遇到兩次。

3.在 scheduler 中，多個負載相關數值 `weight`、`load`、`load_avg`、`util_avg`、`capacity` 等，使用不同 fixed-point 精度。這使得運算時容易出現錯誤或 overflow。這一點在 commit `6ecdd74962f2` (sched/fair: Generalize the load/util averages resolution definition) 中直接被指出：
>weight, load, load_avg, util_avg, freq, and capacity may have different fixed point ranges, which makes their update and usage error-prone.

表面上看，統一尺度確實可避免不同單位混用所帶來的錯誤。然而，在 scheduler 的設計中，這並不是理想的解法。原因在於，一旦統一尺度，就必須同時決定一個共同的解析度與可表示範圍，而 scheduler 的數值運算必須在精度、overflow 風險與計算成本之間取得平衡。

這種取捨可以從 kernel/sched/sched.h 的註解中看出：
```
/*
 * Increase resolution of nice-level calculations for 64-bit architectures.
 * The extra resolution improves shares distribution and load balancing of
 * low-weight task groups (eg. nice +19 on an autogroup), deeper task-group
 * hierarchies, especially on larger systems. This is not a user-visible change
 * and does not change the user-interface for setting shares/weights.
 *
 * We increase resolution only if we have enough bits to allow this increased
 * resolution (i.e. 64-bit). The costs for increasing resolution when 32-bit
 * are pretty high and the returns do not justify the increased costs.
 *
 * Really only required when CONFIG_FAIR_GROUP_SCHED=y is also set, but to
 * increase coverage and consistency always enable it on 64-bit platforms.
 */
```
提高數值解析度可以改善 shares distribution 與 load balancing，但同時也需要更多位元與運算成本，因此僅在 64 位元架構上提高解析度，而 32 位元架構則維持較低解析度。因此 scheduler 的數值表示並不是單純追求統一，而是需要在解析度與實際成本之間取得平衡。
在這種設計背景下，scheduler 內部不同指標就可能帶有不同的尺度。當這些具有不同尺度的數值被混合運算時，就容易出現 `6ecdd74962f2` 所指出的問題。
因此，該 commit 的做法並不是強制所有指標採用相同的表式方法，而是建立共同的 fixed-point 標準，並透過 `scale_load` 與 `scale_load_down` 在不同尺度之間進行轉換，使不同來源的數值在跨指標運算前能夠先被正規化，從而降低更新與使用時出錯的風險。

不同尺度造成的問題在 scheduler 的實作中曾經發生過。例如 `ea1dc6fc6242` (sched/fair: Fix calc_cfs_shares() fixed point arithmetics width confusion) 指出在提高 load resolution 後，scheduler 中兩個參與計算的變數使用了不同單位：
>after which tg->load_avg and cfs_rq->load.weight had different units (10 bit fixed point and 20 bit fixed point resp.)

為了解決這個單位不一致的問題，該 commit 在相關運算中加入轉換機制：
>Add a comment to explain the use of cfs_rq->load.weight over the 'natural' cfs_rq->avg.load_avg and add scale_load_down() to correct for the difference in unit.

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
1. IEEE Standards Board, *IEEE Standard for Binary Floating-Point Arithmetic*, ANSI/IEEE Std 754-1985, 1985. Available: [PDF](https://www.ime.unicamp.br/~biloti/download/ieee_754-1985.pdf)
2. N. J. Higham, *Accuracy and Stability of Numerical Algorithms*, 2nd ed. Philadelphia, PA: SIAM, 2002.
