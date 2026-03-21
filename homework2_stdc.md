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

運算式 `x & (x-1)` 可用來消除最低位的 $1$，也可用來檢測整數 $x$ 是否為 $2$ 的冪。設 $x>0$，令 $k$ 為 $x$ 的最低位 $1$ 所在位置，則 $x$ 可表示為

$$x = m \cdot 2^{k+1} + 2^k \quad (m \ge 0,\; k \ge 0)$$

其中 $2^k$ 對應最低位的 $1$，而其右側有 $k$ 個 $0$。當 $x$ 減去 $1$ 時，該位會變為 $0$，而其右側的 $0$ 會全部變為 $1$，因此

$$x-1 = m \cdot 2^{k+1} + (2^k - 1)$$

因此在二進位表示下，運算 $x \& (x-1)$ 會清除 $x$ 的最低位 $1$。當 $m=0$ 時，

$$x \& (x-1)=0$$

故可利用此條件判斷 $x$ 是否為 $2$ 的冪。

power-of-two 的性質可將昂貴的算術運算轉換為位元運算。當 $N=2^k$ 時，整數 $x$ 的最低 $k$ 個 bit 即對應於 $x$ 除以 $2^k$ 的餘數，因此

$$x \bmod 2^k = x \& (2^k-1)$$

同理，除以 $2^k$ 等價於將二進位表示右移 $k$ 位：

$$x / 2^k = x >> k \quad x \ge 0$$

在許多需要除法或模運算的操作，可以改為位元運算來降低計算成本。
Linux kernel 在 `include/linux/log2.h` 中提供函式 `is_power_of_2`，用於判斷整數是否為 $2^k$：
```
static __always_inline __attribute__((const))
bool is_power_of_2(unsigned long n)
{
	return n - 1 < (n ^ (n - 1));
}
```
這個寫法雖然不同於常見的 `(n & (n-1)) == 0`，但判斷的仍是同一個數值性質。  
`n-1` 的作用在於：對任意整數減 $1$ 時，會翻轉其最低位的 `1` 以及其右側所有 bit。若 $n = 2^k$，則

$$n \oplus (n-1) = 2^{k+1}-1$$

因此

$$n-1 = 2^k-1 < 2^{k+1}-1 = n \oplus (n-1)$$

此性質在 $k=0$ 時亦成立。當 $n=1$ 時：

$$1-1 = 0 < 1\oplus0 = 1$$

反之，若 $n>0$ 且 $n$ 不是 $2^k$，則其二進位中至少包含兩個 `1`。做 $\oplus $ 時高位的 `1` 會被消去，使得xxx

從 commit `4cc67b048459`(linux/log2.h: reduce instruction count for is_power_of_2()) 中也可看出這個 helper 的改寫也有效能上的考量。
```
    Follow an observation that (n ^ (n - 1)) will only ever retain the most
    significant bit set in the word operated on if that is the only bit set in
    the first place, and use it to determine whether a number is a whole power
    of 2, avoiding the need for an explicit check for nonzero.
    
    This reduces the sequence produced to 3 instructions only across Alpha,
    MIPS, and RISC-V targets, down from 4, 5, and 4 respectively, removing a
    branch in the two latter cases.  And it's 5 instructions on POWER and
    x86-64 vs 8 and 9 respectively.  There are no branches now emitted here
    for targets that have a suitable conditional set operation, although an
    inline expansion will often end with one, depending on what code a call to
    this function is used in.
```
透過對 $n \oplus (n-1)$ 的觀察，可以直接判斷整數是否為 $2$ 的冪，避免額外的非零檢查，同時移除分支判斷。


- [ ] `((X) - 0x01010101) & ~(X) & 0x80808080` 技巧可用於 `strlen()` 的實作，回答：
    * 該巨集偵測的是什麼資料？為何該運算可一次檢測 4 個位元組？為何這比逐 byte 檢查更有效率？
    * 在 Linux 核心原始程式碼中找出類似 word-at-a-time 手法的案例，並充分進行效能分析

1. 此巨集用於檢測一個 32 位元變數中，是否存在某個位元組等於 $0x00$。  
依據教材的引導，可先單一位元組的情況。令 $b$ 為一個 8-bit 整數，考慮：

$$(b - 1) \& \sim b \& 0x80$$

其中常數 $0x80$ 提示我們只須關心最高位 (bit 7)。問題可轉換為，$(b-1)$ 的 bit 7 與 $\sim b$ 的 bit 7 在什麼狀況下同時為 1？

若 $\sim b$ 的 bit 7 為 1，表示 $b$ 的 bit 7 原本為 0，因此：

$$\sim b \text{ 的 bit 7 為 1} \iff b < 0x80$$

若 $(b-1)$ 的 bit 7 為 1，表示減 1 後最高位為 1。對 8-bit 整數而言，滿足此條件的情況為：

$$b = 0x00 \quad \text{或} \quad b \ge 0x81$$

而滿足這兩個條件的交集為：

$$(b < 0x80) \cap (b = 0x00 \text{ 或 } b \ge 0x81) = \{0x00\}$$

可以得到：

$$(b - 1) \& \sim b \& 0x80 \neq 0 \iff b = 0x00$$

因此，將此判斷套用到 32 位元變數的後，只要其中任一 byte 等於 $0x00$，則

$$((X) - 0x01010101) \& \sim(X) \& 0x80808080 > 0x0$$

2. 在 `arch/x86/include/asm/word-at-a-time.h` 中，有與題目相同的判斷：

```c
struct word_at_a_time {
	const unsigned long one_bits, high_bits;
};

#define WORD_AT_A_TIME_CONSTANTS { REPEAT_BYTE(0x01), REPEAT_BYTE(0x80) }

/* Return nonzero if it has a zero */
static inline unsigned long has_zero(unsigned long a, unsigned long *bits,
				     const struct word_at_a_time *c)
{
	unsigned long mask = ((a - c->one_bits) & ~a) & c->high_bits;
	*bits = mask;
	return mask;
}
```
段程式碼的邏輯與題目中的巨集完全相同，只是將 `0x01010101` 與 `0x80808080` 抽象成 `REPEAT_BYTE(0x01)` 與 `REPEAT_BYTE(0x80)`，使同一套判斷可直接套用到 unsigned long。因此在 32 位元架構可一次檢查 4 位元組，在 64 位元架構可一次檢查 8 個 byte。

若想進一步知道第 1 個滿足條件的位元組位置，x86 版本還提供：

```c
#define find_zero(bits) (__ffs(bits) >> 3)
```
由於函式 `has_zero` 產生的 mask 只會在 bit 7、15、23、... 位置設為 1，因此使用 `__ffs(bits)` 找出最低的 set bit，再右移 3 位，即可轉成對應的 byte index。
另外這個方法依賴 little-endian 下 byte 與低位 bit 的表示關係，不可直接套用到 big-endian。對於 big-endian 在 `include/asm-generic/word-at-a-time.h` 有提供不同的 `find_zero` 實作。
```c
static inline long find_zero(unsigned long mask)
{
	long byte = 0;
#ifdef CONFIG_64BIT
	if (mask >> 32)
		mask >>= 32;
	else
		byte = 4;
#endif
	if (mask >> 16)
		mask >>= 16;
	else
		byte += 2;
	return (mask >> 8) ? byte : byte + 1;
}
```
在 big-endian 的實作中，則依據位元組順序逐步縮小範圍，判斷第一個 zero byte 的位置。

3. 為了驗證 word-at-a-time 的效果，設計一個 strlen benchmark。程式會根據輸入參數 length 建立一條長度為 length 的字串，內容全部填入 'A'，並在最後補上一個 '\0'。接著分別量測兩種 strlen 實作：

`single_byte_strlen`：逐 byte 掃描，逐一判斷 s[i] == '\0'
`four_byte_strlen`：先處理起始位址未對齊的前綴區段，之後改為每次檢查 4 bytes，並利用
  $$((x - 0x01010101) & ~x & 0x80808080)$$
  判斷其中是否存在 `0x00`

為避免編譯器將 single_byte_strlen 優化為 libc 的 strlen 而影響結果，編譯時加入 -fno-optimize-strlen 與 -fno-builtin 等選項。記憶體透過 posix_memalign(..., 64, ...) 配置，使量測主要反映對齊存取下的效能表現，同時程式透過 `clock_gettime` 取得執行前與執行後的時間。
以下為量測結果：

| 長度 | single_byte ns/call | four_byte ns/call | single_byte ns/byte | four_byte ns/byte |
| --- | ---: | ---: | ---: | ---: |
| 64 B | 110.37 | 55.82 | 1.7245 | 0.8722 |
| 256 B | 253.95 | 113.84 | 0.9920 | 0.4447 |
| 1 KiB | 983.90 | 309.40 | 0.9608 | 0.3021 |
| 4 KiB | 3611.63 | 973.55 | 0.8817 | 0.2377 |
| 64 KiB | 65234.17 | 15168.37 | 0.9954 | 0.2315 |
| 1 MiB | 1209283.11 | 305303.62 | 1.1533 | 0.2912 |
| 16 MiB | 24303767.00 | 6971473.75 | 1.4486 | 0.4155 |

由這組量測可見，`four_byte_strlen` 的執行時間低於 `single_byte_strlen`。



- [ ]  教材提及若干真實案例，以 Boeing 787 的[軟體缺失案例](https://hackmd.io/@sysprog/software-failure)來說，為何 32 位元計數器在約 248 天會 overflow？參照 FAA (Federal Aviation Administration ) 和相關官方事故分析報告進行探討
    * 在 Linux 核心的 git log 找出類似的 integer overflow 案例並探討
    * 在 C 語言規格書列舉相關整數範圍的規範和實作考量


## Reference
1. IEEE Standards Board, *IEEE Standard for Binary Floating-Point Arithmetic*, ANSI/IEEE Std 754-1985, 1985. Available: [PDF](https://ieeemilestones.ethw.org/File%3AIeee754-1985.pdf)
2. N. J. Higham, *Accuracy and Stability of Numerical Algorithms*, 2nd ed. Philadelphia, PA: SIAM, 2002.
