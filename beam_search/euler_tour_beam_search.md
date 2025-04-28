# Euler Tour Beam Search — ドキュメント

<!-- TOC -->
- [概要](#概要)
- [テンプレートパラメータ](#テンプレートパラメータ)
- [関数シグネチャ](#関数シグネチャ)
- [引数](#引引数)
- [戻り値](#戻り値)
- [例外](#例外)
- [アルゴリズム概要](#アルゴリズム概要)
- [計算量](#計算量)
- [StateType に求められるインターフェース](#statetype-に求められるインターフェース)
- [使用例](#使用例)
- [拡張ポイント](#拡張ポイント)
- [参考文献](#参考文献)
<!-- /TOC -->

## 概要
`euler_tour_beam_search` は *Euler Tour* 技法で探索木を線形化し、ビームサーチ (Beam Search) と組み合わせて経路探索を高速化する汎用アルゴリズムです。ユーザは `StateType` が定めるインターフェースを実装するだけで、幅優先探索と深さ優先探索の利点を兼ね備えた近似的経路探索を簡潔に利用できます。

## テンプレートパラメータ
| パラメータ | 説明 |
|-----------|------|
| `StateType` | 探索空間を表現するユーザ定義型。後述のインターフェースを実装する必要があります。 |

## 関数シグネチャ
```cpp
template <typename StateType>
std::vector<typename StateType::Action> euler_tour_beam_search(
    const StateType &state,
    size_t           max_turn,
    size_t           beam_width,
    size_t           hash_map_capacity = 0);
```

## 引数
| 名前 | 型 | 説明 |
|------|----|------|
| `state` | `const StateType &` | 初期状態を保持するオブジェクト。参照で受け取り、内部で必要に応じてコピーされます。 |
| `max_turn` | `size_t` | 探索を行う最大ターン数。0 から始まり、`max_turn` 回展開した時点で探索を打ち切ります。 |
| `beam_width` | `size_t` | ビーム幅。各ターンで保持する候補ノード数の上限です。 |
| `hash_map_capacity` | `size_t` (省略可) | ハッシュテーブルの初期容量。`0` を指定すると `16 * 3 * beam_width` が自動設定されます。|

## 戻り値
`std::vector<typename StateType::Action>` — 発見された行動列 (パス)。

* 終端ノードが見つかった場合: そのターンまでの行動列を返します。
* `max_turn` 到達などで探索を打ち切った場合: 現在の候補の中で評価値が最良のノードまでの行動列を返します。
* 候補が存在しない場合: 空のベクタを返します。

## 例外
本関数は直接例外を送出しません。ただし `StateType` の実装やユーザ定義コンテナが例外を送出した場合はそのまま伝搬します。また、`assert` によりデバッグ時に不正使用を検出します。(例: ビームが空の状態で `get_best_candidate` を呼び出すなど)

## アルゴリズム概要
1. **EulerTourTree** — 探索木を *Euler Tour* (行きがけ/帰りがけ) で線形表現し、パス復元と局所更新を高速化します。
2. **BeamSelector** — セグメントツリーで *最悪コスト* の置換を \(O(\log B)\) で行い、同一ハッシュの重複を排除します (\(B = \) `beam_width`)。
3. **メインループ**
   1. `EulerTourTree::dfs` で 1 ターン展開し、`BeamSelector::push` で候補集合を更新。
   2. 終端ノードが存在すれば即座にパスを復元して返却。
   3. 候補が空の場合は失敗 (空ベクタ) を返却。
   4. `turn + 1 == max_turn` なら評価値が最良のノードを用いてパスを返却。
   5. `EulerTourTree::update` でツリーを更新して次ターンへ。

## 計算量
- **時間**: \(O(\text{max\_turn} \times (B \times C_{expand} + \log B))\)
  - \(C_{expand}\) は `StateType::expand` 1 回あたりの候補数と評価計算コストに依存します。
- **空間**: \(O(B)\) — ビーム幅に比例。ハッシュ表と Euler Tour 配列も同オーダーです。

## StateType に求められるインターフェース
```cpp
struct StateType {
  using Action;    // 行動の型
  using Evaluator; // 評価器の型
  using Hash;      // ハッシュ値の型
  struct Candidate { int parent; Action action; Evaluator eval; Hash hash; };

  // 初期ノードを生成
  std::tuple<Action, Evaluator, Hash> make_initial_node() const;

  // action を仮適用した状態で子ノードを列挙し push_candidate を呼ぶ
  void expand(const Action &action,
              const Evaluator &eval,
              const Hash &hash,
              const std::function<void(const Action &, const Evaluator &, const Hash &, bool)> &push_candidate) const;

  // action を状態に適用
  void apply(const Action &action);

  // 直前に apply した action をロールバック
  void rollback(const Action &action);
};
```
### Evaluator の要件
```cpp
struct Evaluator {
  using Cost = /* 符号付き算術型 */;
  Cost evaluate() const; // 小さい値ほど良いとみなす
};
```

## 使用例
```cpp
#include "beam_search.hpp"
#include <iostream>

struct MyState : BeamSearch::IBeamState<MyAction, MyEvaluator, MyHash> {
  // …StateType のインターフェースを実装…
};

int main() {
  MyState init_state(/* … */);
  size_t max_turn   = 30;
  size_t beam_width = 64;

  auto best_actions = BeamSearch::euler_tour_beam_search(init_state, max_turn, beam_width);
  for (const auto &a : best_actions) std::cout << a << "\n";
}
```

## 拡張ポイント
- **ハッシュ関数の最適化** — `StateType::Hash` と `HashMap` を置き換えることでメモリ効率や衝突率を改善できます。
- **動的ビーム幅** — `BeamSelector` を改造してビーム幅をターンごとに調整するアプローチも可能です。
- **評価関数の工夫** — タスク固有のヒューリスティックを `Evaluator::evaluate()` に実装すると探索性能が向上します。

## 参考文献
- R. Korf, “Depth-first iterative-deepening: An optimal admissible tree search,” *Artificial Intelligence*, 1985.
- Competitive Programming Library for Beam Search, 2025.
