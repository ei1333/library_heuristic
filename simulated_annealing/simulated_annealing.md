# Simulated Annealing Utility (C++20)

## Overview
This header‑only utility provides a generic simulated‑annealing loop that can be paired with any *state* object that fulfils the **`SAState`** concept.

*Key features*  
- **Type‑safe:** leverages C++20 Concepts to catch interface mismatches at compile time.  
- **Header‑only:** simply include the header in your project, no separate build step required.  
- **Deterministic duration:** runtime is limited by a wall‑clock timeout rather than an iteration counter.

## Requirements
- A C++20 compliant compiler (e.g. GCC 11+, Clang 14+, MSVC 19.30+).
- `<chrono>`, `<cmath>`, and `<concepts>` headers.  
  (Timer and RNG utilities are assumed to be implemented elsewhere.)

## API Reference

### Concept `SAState`
```cpp
template<class T>
concept SAState = requires(T s, double delta, double progress) {
    { s.update(delta, progress) } -> std::same_as<void>;
};
```
An *`SAState`* type **must** provide  
```cpp
void update(double delta, double progress);
```  
where

| Parameter | Meaning |
|-----------|---------|
| `delta`   | Candidate energy change (already multiplied by the current temperature). |
| `progress`| Normalised elapsed time in the range **[0 … 1]**. |

### Function `SimulatedAnnealing::simulated_annealing`
```cpp
template<SAState State>
void simulated_annealing(State& state,
                         double start_temp,
                         double end_temp,
                         int    end_milliseconds);
```
| Parameter | Meaning |
|-----------|---------|
| `state`            | Stateful object updated in‑place during the search. |
| `start_temp`       | Initial temperature. |
| `end_temp`         | Final temperature. |
| `end_milliseconds` | Wall‑clock time budget in **milliseconds**. |

Internally the loop  
1. tracks elapsed time with a `Timer`,  
2. linearly interpolates the temperature between `start_temp` and `end_temp`,  
3. calls `state.update()` 256 times per outer iteration using  
   `delta = temp * log(rng.probability())`.

#### Expected behaviour of `update`
Your `update` implementation is responsible for **accepting or rejecting** the candidate move according to your cost function.  
A typical pattern is the classic Metropolis–Hastings acceptance test.

## Minimal Example
```cpp
#include "simulated_annealing.hpp"
#include <iostream>

struct MyState {
    double energy = 0.0;

    void update(double delta, double /*progress*/) {
        // Simple downhill‑only move:
        if (delta < 0) energy += delta;
    }
};

int main() {
    MyState st;
    SimulatedAnnealing::simulated_annealing(st, 1.0, 0.01, 1000);
    std::cout << "Final energy: " << st.energy << '\n';
}
```

## Building
```bash
g++ -std=c++20 -O2 -pipe main.cpp -o sa_example
```

## Extending the Interface
To enforce additional operations (e.g. `double score() const;`) simply extend the `requires` clause:

```cpp
concept SAState = requires(T s, double delta, double progress) {
    { s.update(delta, progress) } -> std::same_as<void>;
    { s.score() } -> std::convertible_to<double>;
};
```

## License
MIT License – see `LICENSE` for full text.
