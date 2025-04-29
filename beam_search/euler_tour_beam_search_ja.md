# `euler_tour_beam_search` 日本語ドキュメント（更新版）

固定幅ビームサーチに **Euler ツアー木** を組み合わせた汎用探索アルゴリズムです。

> **変更点 (2025‑04‑29)**  
> * `make_initial_node()` の戻り値が **`std::tuple<Action, Evaluator, Hash>`** になりました。  
> * `expand()` の第一引数に「親からこの状態へ到達した **Action**」が渡されます。  
>   ```cpp
>   void expand(const Action& parent_action,
>               const Evaluator& eval,
>               const Hash& h,
>               Push push) const;
>   ```

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
| `initial_state` | **`BeamState`** コンセプトを満たすルート状態 |
| `max_turn` | 最大手数（深さ上限） |
| `beam_width` | 各深さで保持する候補数 |
| `hash_map_capacity` | 重複検出ハッシュ表の容量（`0` で自動設定） |

---

## `BeamState` コンセプト（更新）

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
    /* 型定義 */
    typename State::Action;
    typename State::Evaluator;
    typename State::Hash;

    /* ルートノード */
    { cs.make_initial_node() }
        -> std::same_as<std::tuple<typename State::Action,
                                   typename State::Evaluator,
                                   typename State::Hash>>;

    /* 子ノード列挙 */
    { cs.expand(a, e, h, push) } -> std::same_as<void>;
      // a は「親→現在」への Action。不要なら無視してよい。

    /* 状態遷移／巻き戻し */
    { s.apply(a)    } -> std::same_as<void>;
    { s.rollback(a) } -> std::same_as<void>;
};
```

`Evaluator` の要件は従来と同じ（コストの最小化）。

---

## 移行ガイド

| 旧実装 | 新実装 | 補足 |
|--------|--------|------|
| `auto [eval, hash] = make_initial_node();` | `auto [root_action, eval, hash] = make_initial_node();` | `root_action` は多くのケースでデフォルト値 (`Action{}`) となります。 |
| `expand(eval, hash, push);` | `expand(parent_action, eval, hash, push);` | 第一引数は「この状態に来るのに使った Action」。不要なら無視可。 |

---

アルゴリズムの流れ・計算量・チューニング方法は変更ありません。  
英語版ドキュメントも併せてご覧ください。
