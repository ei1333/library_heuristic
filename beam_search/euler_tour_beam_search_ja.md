# `euler_tour_beam_search` 日本語ドキュメント

固定幅ビームサーチに **Euler ツアー木** を組み合わせた汎用探索ルーチンです。  
Euler ツアー表現を使うことで、部分解の **共通接頭辞** を効率よく再利用し、各ターンで追加された枝だけを拡張します。これにより、ビーム幅を大きくしても時間・メモリの増加を抑えられます。

---

## 関数テンプレート

```cpp
template<BeamState State>
std::vector<typename State::Action>
euler_tour_beam_search(const State& initial_state,
                       std::size_t  max_turn,
                       std::size_t  beam_width,
                       std::size_t  hash_map_capacity = 0);
```

| 引数 | 説明 |
|------|------|
| `initial_state` | **`BeamState`** コンセプトを満たすユーザ定義型のインスタンス（後述）。探索のルートとなるクリーンな状態を渡します。 |
| `max_turn` | 探索する最大手数。これ以上深いノードは展開しません。 |
| `beam_width` | 各深さで保持する候補数。大きくすると精度は上がりますが計算量も増えます。 |
| `hash_map_capacity` | 重複検出用ハッシュ表の容量。`0` の場合は `16 × 3 × beam_width` に自動設定されます。 |

### 戻り値
- 途中で **`finished == true`** のノードが見つかった場合 → その最短経路  
- 見つからなければ深さ `max_turn` 時点で **最も良いコスト** の経路  
- 展開可能なノードが無い場合 → 空ベクタ

---

## `BeamState` コンセプト

```cpp
template<class State>
concept BeamState = Evaluator<typename State::Evaluator> &&
                    /*--------------------------------------*/
requires(State& s,
         const State& cs,
         typename State::Action     a,
         typename State::Evaluator  e,
         typename State::Hash       h,
         std::function<void(const typename State::Action&,
                            const typename State::Evaluator&,
                            const typename State::Hash&,
                            bool)> push) {
    /* 型 */
    typename State::Action;
    typename State::Evaluator;
    typename State::Hash;

    /* ルートノード生成 */
    { cs.make_initial_node() }
        -> std::same_as<std::pair<typename State::Evaluator,
                                  typename State::Hash>>;

    /* 子ノード列挙 */
    { cs.expand(e, h, push) } -> std::same_as<void>;

    /* 状態遷移／巻き戻し */
    { s.apply(a)    } -> std::same_as<void>;
    { s.rollback(a) } -> std::same_as<void>;
};
```

### `Evaluator` コンセプト
```cpp
template<class E>
concept Evaluator = requires(const E& e) {
    typename E::Cost;                 // 全順序付き型
    { e.evaluate() } -> std::same_as<typename E::Cost>;
};
```
ビームサーチでは **値が小さいほど良い**（最小化）とみなします。  
最大化問題なら「負の報酬」をコストとして返すなどしてください。

---

## アルゴリズム概要

1. **EulerTourTree**  
   - 探索木を Euler ツアー列として保持  
   - `apply` / `rollback` で O(1) で経路を進めたり戻したり
2. **BeamSelector**  
   - 固定長ベクタで候補を管理  
   - セグメント木で **最悪候補** を O(log w) で取得  
   - オープンアドレス法ハッシュで重複削除
3. 各ターンで  
   1. 葉を展開 (`State::expand`)  
   2. `finished` が出たら経路を復元して即終了  
   3. ビーム幅まで絞り込み、ツアーを更新  
4. 深さ上限到達時は最良候補を返す

---

## 計算量

| 指標 | オーダー |
|------|---------|
| 時間 | \(O(	ext{beam\_width} × b × 	ext{max\_turn})\) （b: 平均分岐数） |
| メモリ | \(O(	ext{beam\_width})\) |

---

## ミニ例（グリッド探索）

```cpp
struct GridState {
    /* Action, Evaluator, Hash などを定義 */

    std::pair<Evaluator, Hash> make_initial_node() const { … }

    void expand(const Evaluator& e, const Hash& h, auto push) const {
        for (const Action& a : legal_moves()) {
            Evaluator ne = e;
            Hash      nh = h;
            /* コスト＆ハッシュ更新 */

            bool finished = goal_reached();
            push(a, ne, nh, finished);
        }
    }

    void apply(const Action& a)    { … }
    void rollback(const Action& a) { … }
};

int main() {
    GridState root;
    auto path = BeamSearch::euler_tour_beam_search(root, 200, 1000);

    for (auto&& a : path) std::cout << a << ' ';
    std::cout << std::endl;
}
```

---

## TIPS

* `beam_width` は実験的に調整しましょう。幅を 2 倍にしても必ずしも実行時間が 2 倍になるわけではありません。
* `Evaluator::evaluate()` には **強いヒューリスティック** を。ビームサーチの性能はここで決まります。
* `Hash` は **軽量** に（例: Zobrist ハッシュ）。毎回 `push` で計算されます。

---

## ライセンス

この実装（`beam_search.hpp`）は MIT ライセンス⁠⁽¹⁾ です。  
商用・非商用を問わず自由にご利用ください。

> ⁽¹⁾ 実際のソースに別のライセンス表記がある場合はそちらを優先してください。
