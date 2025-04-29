# `euler_tour_beam_search`

A generic **beam‑search** routine that combines a traditional fixed‑width beam with an _Euler tour_ representation of the search tree.

> **Update (2025‑04‑29)** — the interface of the user‑supplied `State` type has changed:  
> * `make_initial_node()` now returns **`std::tuple<Action, Evaluator, Hash>`** instead of a pair.  
> * `expand()` now receives the *incoming* `Action` as its first argument:  
>   `expand(const Action& parent_action, const Evaluator& eval, const Hash& h, Push push)`.

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
| `initial_state` | Instance of a user‑defined type that satisfies the **`BeamState`** concept (see below). |
| `max_turn` | Upper bound on the number of turns (depth). |
| `beam_width` | Maximum number of partial states kept at each depth. |
| `hash_map_capacity` | Capacity of the internal hash table. If `0`, it defaults to `16 × 3 × beam_width`. |

### Return value
* **First finished path** found (if any leaf is pushed with `finished == true`), or  
* **Best cost path** when the depth limit is reached, or  
* Empty vector if no expansion was possible.

---

## Requirements – the `BeamState` concept (updated)

```cpp
template<class State>
concept BeamState =
    Evaluator<typename State::Evaluator> &&
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
    typename State::Action;
    typename State::Evaluator;
    typename State::Hash;

    /*  Root node ------------------------------------------------------------*/
    { cs.make_initial_node() }
        -> std::same_as<std::tuple<typename State::Action,
                                   typename State::Evaluator,
                                   typename State::Hash>>;

    /*  Frontier expansion ---------------------------------------------------*/
    { cs.expand(a, e, h, push) } -> std::same_as<void>;
      // 'a' is the action that led to the current state (can be ignored).

    /*  State mutation -------------------------------------------------------*/
    { s.apply(a)   } -> std::same_as<void>;
    { s.rollback(a)} -> std::same_as<void>;
};
```

### `Evaluator` concept
Unchanged — the beam minimises `Evaluator::Cost`.

---

## Algorithm outline

(unchanged – see previous version for details)

---

## Migration guide

| Old API | New API | Notes |
|---------|---------|-------|
| `auto [eval, hash] = make_initial_node();` | `auto [root_action, eval, hash] = make_initial_node();` | `root_action` is typically a default‑constructed `Action` (e.g. `Action{}`). |
| `expand(eval, hash, push);` | `expand(parent_action, eval, hash, push);` | The first argument is the action that moved **into** the current node; many problems can simply ignore it. |

---

All other behaviour, complexity bounds and tuning advice remain identical.  
See the Japanese document for a localized explanation.

