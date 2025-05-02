# Hill‑Climbing Utility (C++20)

## Overview
This header‑only helper implements a simple **hill‑climbing loop** that repeatedly calls a user‑supplied `State::update()` method.  
It is intended as a fast, deterministic baseline for local‑search style optimizers.

*Key traits*

* **Type‑safe:** uses C++20 Concepts (`HCState`) to ensure the state object exposes the required interface.
* **Time‑boxed:** the loop ends after a wall‑clock time budget (milliseconds) rather than counting iterations.
* **Customisable granularity:** the `step` parameter controls how many `update()` calls occur between successive time checks.

## Requirements
* C++20 compliant compiler (GCC 11+, Clang 14+, MSVC 19.30+).
* Timer implementation providing `get_milliseconds()` returning `int64_t` or similar.

## API Reference

### Concept `HCState`
```cpp
template<class State>
concept HCState = requires(State s) {
    { s.update() } -> std::same_as<void>;
};
```
An **`HCState`** type must supply
```cpp
void update();
```
which performs *one* search step – e.g. mutating internal state if an improvement is found.

### Function `HillClimbing::hill_climbing`
```cpp
template<HCState State>
void hill_climbing(State& state,
                   int   end_milliseconds,
                   int   step = 256);
```
| Parameter | Description |
|-----------|-------------|
| `state`            | The user‑defined search state, modified in‑place. |
| `end_milliseconds` | Wall‑clock time budget in **milliseconds**. |
| `step`             | Number of `state.update()` calls between clock checks (default 256). |

#### Behaviour
```txt
loop {
    if (elapsed >= budget) break;
    repeat `step` times:
        state.update();
}
```
*Lower `step`* → finer granularity (less overrun) but more `Timer` overhead.  
*Higher `step`* → fewer system calls but possible budget overshoot.

## Minimal Example
```cpp
#include "hill_climbing.hpp"
#include <iostream>
#include <cstdlib>

struct MyState {
    int    x    = 0;
    double best = 1e9;

    // minimise f(x) = (x-42)^2
    void update() {
        int candidate = x + ((rand() & 1) ? 1 : -1);   // ±1 step
        double score  = (candidate - 42.0) * (candidate - 42.0);

        if (score < best) {
            x    = candidate;
            best = score;
        }
    }
};

int main() {
    MyState st;
    HillClimbing::hill_climbing(st, 500); // 500 ms
    std::cout << "Solution: " << st.x << ", error = " << st.best << '\n';
}
```

## Tips & Extensions
* **Adaptive step size:** call `hill_climbing()` twice – once with small `step` early for exploration, later with large `step` for exploitation.
* **Restart strategy:** run multiple independent hill‑climbers with random initial conditions and keep the best outcome.
* **Thread safety:** the function itself is thread‑safe when each thread owns its own `State` instance.
* **Pausing / resuming:** expose `State::serialize()` / `deserialize()` to checkpoint long runs.

## License
MIT License – see `LICENSE` for the full text.
