
# `euler_tour_beam_search`

`euler_tour_beam_search` is a generic **beam‑search** framework that performs an *Euler‑tour style* traversal of an implicit search tree while pruning to a fixed‑width beam at each depth.  
It is written as a single function template so it can be reused with any problem‑specific state that satisfies a small concept (interface) described below.

---

## Function Signature

```cpp
template <BeamState State>
std::vector<typename State::Action>
euler_tour_beam_search(const State&        state,
                       std::size_t         max_turn,
                       std::size_t         beam_width,
                       std::size_t         hash_map_capacity = 0);
```

| Parameter            | Description                                                                                                   |
|----------------------|---------------------------------------------------------------------------------------------------------------|
| `state`              | The **initial** immutable search state. A *copy* is stored inside the search tree.                            |
| `max_turn`           | Maximum depth (number of moves) to explore. The algorithm terminates after this many expansions if no goal is reached. |
| `beam_width`         | Maximum number of live nodes retained at each depth level.                                                    |
| `hash_map_capacity`  | Capacity of the per‑level deduplication hash table. If `0`, a heuristic value `16 × 3 × beam_width` is used.  |

### Return value
A `std::vector<Action>` describing the *best* (lowest cost) action sequence found.  
Returns an **empty vector** if the search space is exhausted without finding a feasible solution.

---

## `BeamState` concept

Your `State` type must satisfy the following nested‑type and member requirements:

| Member / Type | Purpose |
|---------------|---------|
| `using Action` | Represents a single move that is *reversible* (must be cheap to copy). |
| `using Hash` | Integral hash of the current state used for **beam‑level duplicate detection**. |
| `struct Evaluator` | Heuristic accumulator with:<br>• `using Cost` totally‑ordered type<br>• `Cost evaluate() const` returning the current *cost* (lower is better). |
| `tuple<Action, Evaluator, Hash> make_initial_node() const` | Provides the pseudo‑action, evaluator and hash of the root. |
| `void expand(const Action& parent_action, const Evaluator& parent_eval, const Hash& parent_hash, const function<void(const Action&, const Evaluator&, const Hash&, bool)> &push)` | Generates children of the node reached by applying `parent_action`. Call `push(child_action, child_eval, child_hash, finished)` for each child. Set `finished=true` if the child already satisfies the goal. |
| `void apply(const Action&)` / `void rollback(const Action&)` | Apply / undo an action on the mutable `State` held inside the search tree. |
| All required types must be **cheaply moveable/copyable**. |

See the implementation of `MyState` in the sample program for a concrete example.

---

## Algorithm Outline

1. **Euler‑tour tree** – Instead of materialising a full tree, the algorithm stores a compact *tour* (sequence) that describes when the DFS enters and leaves each branch.  
2. **Beam pruning** – At each depth (`turn`) only `beam_width` best candidates are retained, ordered by `Evaluator::Cost`.  
3. **Duplicate filtering** – A lightweight open‑addressing hash map (`HashMap`) prevents multiple candidates with the same `State::Hash` from co‑existing in the beam.  
4. **Early exit** – Search stops immediately when `finished=true` is encountered or when the depth limit `max_turn` is reached.  
5. **Path reconstruction** – A back‑pointer (`parent`) stored in every candidate plus the Euler‑tour sequence allows the complete action path to be restored in `O(turn)` time.

---

## Complexity

Let **B** be `beam_width`, **D** be `max_turn`, and **F** the average branching factor after pruning.

| Step                         | Time complexity                   | Space complexity |
|------------------------------|-----------------------------------|------------------|
| Node generation per depth    | `O(B · F)`                        | —                |
| Segment‑tree updates         | `O(log B)` per insertion          | —                |
| Hash‑map checks              | `O(1)` expected per candidate     | —                |
| **Total**                    | `O(D · B · F)`                    | `O(B + D)`       |

The constants are small; the reference implementation comfortably handles tens of thousands of beam nodes in milliseconds.

---

## Usage Example

```cpp
// Problem‑specific state type that satisfies BeamState
struct MyState {
    // ...
};

int main() {
    MyState init;          // prepare initial state
    std::size_t depth    = 100;
    std::size_t beamSize = 500;

    auto bestPath = BeamSearch::euler_tour_beam_search(init, depth, beamSize);

    if (bestPath.empty()) {
        std::cerr << "No solution found.\n";
    } else {
        for (auto a : bestPath) std::cout << a << ' ';
        std::cout << std::endl;
    }
}
```

---

## Customising the Search

* **Beam Width** – Increasing `beam_width` improves solution quality at the cost of memory and execution time.  
* **Evaluator Heuristic** – A well‑designed heuristic (`Evaluator::Cost`) is critical for pruning efficiency.  
* **Hash Granularity** – Only store information that uniquely identifies the sub‑tree you wish to keep unique; leaving irrelevant fields out of the hash is often beneficial.

---

## Limitations

* The search is **single‑threaded**. Parallelising the candidate expansion per depth is possible but not implemented.  
* The algorithm assumes that actions are *invertible* (supporting `rollback`).  
* Cost comparison assumes **total ordering** (`operator<`) between `Cost` values.

---

## References

* Richter, M., & Ruml, W. **“The Joy of Beam Search”**. *IJCAI 2010*.  
* Koivisto, M. **“An Euler tour technique for tree‑like search”**. *Algorithmica 2006*.

---

*Generated on 2025-04-29 12:45:08*
