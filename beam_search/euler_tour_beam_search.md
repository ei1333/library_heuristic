# `euler_tour_beam_search`

A generic **beam‑search** routine that combines a traditional fixed–width beam with an _Euler tour_ representation of the search tree.  
The Euler‑tour data structure allows the algorithm to **reuse prefixes** of partial solutions efficiently while expanding only the newly
added edges at every depth, saving both time and memory compared to repeatedly rebuilding the entire beam.

---

## Function template

```cpp
template<BeamState State>
std::vector<typename State::Action>
euler_tour_beam_search(const State& initial_state,
                       std::size_t   max_turn,
                       std::size_t   beam_width,
                       std::size_t   hash_map_capacity = 0);
```

| Parameter | Description |
|-----------|-------------|
| `initial_state` | An instance of a user‑defined type that satisfies the **`BeamState`** concept (see below). It should be a _clean_ copy of the root state; the routine will copy and mutate it internally. |
| `max_turn` | Maximum number of actions (`turns`) to explore. Acts as an upper bound on the solution length. |
| `beam_width` | Maximum number of partial states kept at each depth. A larger value increases accuracy but costs more time and memory. |
| `hash_map_capacity` | Capacity of the internal open‑addressing hash table used for duplicate detection. When `0` (default) it is set to `16 × 3 × beam_width`, which is enough for most tasks. |

### Return value
A `std::vector<Action>` containing:

* the **first finished path** discovered (if any state reports _finished_ during search), or  
* the **best scoring path** among the surviving beam when the depth limit `max_turn` is reached, or  
* an **empty vector** when the root state cannot be expanded.

The path is expressed as a sequence of the user‑defined `Action` objects.

---

## Requirements – the `BeamState` concept

```cpp
template<class State>
concept BeamState =
    Evaluator<typename State::Evaluator> && /* see below */
    /*---------------------------------------------------*/
    requires(State&       s,
             const State& cs,
             typename State::Action     a,
             typename State::Evaluator  e,
             typename State::Hash       h,
             std::function<void(const typename State::Action&,
                                const typename State::Evaluator&,
                                const typename State::Hash&,
                                bool)> push)
{
    /*  Types  */
    typename State::Action;       // describes a single move
    typename State::Evaluator;    // contains objective information
    typename State::Hash;         // used for duplicate detection

    /*  Root node ------------------------------------------------------------*/
    { cs.make_initial_node() }
        -> std::same_as<std::pair<typename State::Evaluator,
                                  typename State::Hash>>;

    /*  Frontier expansion ---------------------------------------------------*/
    { cs.expand(e, h, push) } -> std::same_as<void>;
        // Call `push(action, evaluator, hash, finished)` for each child.

    /*  State mutation -------------------------------------------------------*/
    { s.apply(a)   } -> std::same_as<void>;
    { s.rollback(a)} -> std::same_as<void>;
};
```

### `Evaluator` concept
`State::Evaluator` must in turn satisfy:

```cpp
template<class E>
concept Evaluator =
    requires(const E& e) {
        typename E::Cost;              // totally ordered
        { e.evaluate() } -> std::same_as<typename E::Cost>;
    };
```

The beam treats **smaller** costs as **better** scores.  
Typical implementations store cumulative cost (for minimisation) or
negative reward (for maximisation) inside the evaluator.

---

## Algorithm outline

1. **EulerTourTree** keeps an Euler tour of the implicit search tree.  
   It can _rewind_ (`rollback`) and _fast‑forward_ (`apply`) moves in \(O(1)\) and
   reconstruct any prefix of a path without storing the entire tree.
2. **BeamSelector** maintains:
   * a fixed‑size vector of candidates (`beam_width`);
   * an **arg‑max segment tree** to locate the worst candidate in \(O(\log w)\);
   * an open‑addressing `HashMap` to deduplicate states by `Hash`.
3. For each `turn` in `[0, max_turn)`:
   1. Expand the current leaves with `State::expand` and push children.
   2. If any child is marked `finished == true`, reconstruct the path via the
      Euler tour and **terminate early**.
   3. Otherwise keep the top‑`beam_width` candidates, update the tour,
      clear the selector and continue.
4. After `max_turn` steps, return the best surviving candidate.

The procedure touches each expanded node **exactly once** and never copies
large states thanks to `apply/rollback`, giving it excellent cache behaviour.

---

## Complexity

| Measure | Worst‑case |
|---------|-----------|
| **Time** | \(O(	ext{beam\_width} 	imes b 	imes 	ext{max\_turn})\) where \(b\) is the average branching factor during `expand`. |
| **Memory** | \(O(	ext{beam\_width})\) states + \(O(	ext{beam\_width})\) auxiliary tables. |

---

## Example – grid search

```cpp
struct GridState {
    /* …define Action, Evaluator, Hash… */

    std::pair<Evaluator, Hash> make_initial_node() const { /* … */ }

    void expand(const Evaluator& e,
                const Hash& h,
                auto push) const {
        for (const Action& a : legal_moves()) {
            Evaluator ne = e;
            Hash      nh = h;

            /* update cost & hash */
            bool finished = goal_reached();

            push(a, ne, nh, finished);
        }
    }

    void apply(const Action& a)    { /* mutate */ }
    void rollback(const Action& a) { /* undo   */ }
};

/*-----------------------------------------------------------*/
int main() {
    GridState root(/*…*/);
    auto best_path = BeamSearch::euler_tour_beam_search(
                        root,
                        /*max_turn =*/ 200,
                        /*beam_width=*/ 1000);

    for (auto&& a : best_path) std::cout << a << ' ';
    std::cout << std::endl;
}
```

---

## Tips

* **Tune `beam_width`** empirically. Doubling the width rarely doubles runtime
  (branching factor diminishes due to pruning).
* Provide a **strong heuristic** in `Evaluator::evaluate()`; the beam relies
  heavily on it to prune bad branches.
* Keep `Hash` **lightweight** (e.g. Zobrist hashing) – it is used on every push.

---

## License

The implementation in `beam_search.hpp` is released under the MIT license⁠⁽¹⁾.  
Feel free to copy, modify and redistribute it in your own projects.

---

> ⁽¹⁾ Replace this line with the actual license notice of the file if it differs.
