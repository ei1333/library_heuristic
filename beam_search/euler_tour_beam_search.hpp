namespace BeamSearch {
template<class E>
concept Evaluator = requires(const E &e)
    {
      typename E::Cost;
      { e.evaluate() } -> same_as<typename E::Cost>;
    } &&
    totally_ordered<typename E::Cost>;

template<class State>
concept BeamState =
    Evaluator<typename State::Evaluator> &&
    requires(State& s, const State& cs,
             typename State::Action& a,
             typename State::Evaluator& e,
             typename State::Hash& h,
             function<void(const typename State::Action &,
                           const typename State::Evaluator &,
                           const typename State::Hash &,
                           bool)> push)
    {
      typename State::Action;
      typename State::Evaluator;
      typename State::Hash;

      { cs.make_initial_node() } -> same_as<pair<typename State::Evaluator, typename State::Hash> >;
      { cs.expand(e, h, push) } -> same_as<void>;
      { s.apply(a) } -> same_as<void>;
      { s.rollback(a) } -> same_as<void>;
    } &&
    integral<typename State::Hash>;

template<typename StateType>
struct BeamSelector {
  using Action = typename StateType::Action;
  using Evaluator = typename StateType::Evaluator;
  using Cost = typename Evaluator::Cost;
  using Hash = typename StateType::Hash;

  struct Candidate {
    int parent;
    Action action;
    Evaluator eval;
    Hash hash;
  };

  vector<Candidate> finished_candidates, candidates;
  using T = pair<Cost, int>;

  vector<T> costs;
  bool full;

  struct monoid {
    using S = T;
    static constexpr S op(const S &a, const S &b) {
      if (a.first < b.first) return b;
      return a;
    }
    static constexpr S e() {
      return {numeric_limits<Cost>::min(), -1};
    }
  };
  SegmentTree<monoid> seg;
  HashMap<Hash, int> hash_to_index;
  size_t beam_width;

  explicit BeamSelector(size_t beam_width, size_t hash_map_capacity)
    : full(false),
      seg(monoid(), beam_width),
      hash_to_index(hash_map_capacity),
      beam_width(beam_width) {
    candidates.reserve(beam_width);
    costs.reserve(beam_width);
  }

  void push(const Action &action, const Evaluator &eval, const Hash &hash, int parent, const bool finished) {
    auto cost = eval.evaluate();
    if (finished) {
      finished_candidates.emplace_back((Candidate){parent, action, eval, hash});
      return;
    }
    if (full and cost >= seg.all_prod().first) {
      return;
    }
    auto [valid, i] = hash_to_index.get_index(hash);
    if (valid) {
      int j = hash_to_index.get(i);
      if (hash == candidates[j].hash) {
        if (cost < costs[j].first) {
          candidates[j] = (Candidate){parent, action, eval, hash};
          costs[j].first = cost;
          if (full) {
            seg.set(j, costs[j]);
          }
        }
        return;
      }
    }
    if (full) {
      int j = seg.all_prod().second;
      hash_to_index.set(i, hash, j);
      candidates[j] = (Candidate){parent, action, eval, hash};
      costs[j].first = cost;
      seg.set(j, costs[j]);
    } else {
      hash_to_index.set(i, hash, static_cast<int>(candidates.size()));
      costs.emplace_back(cost, static_cast<int>(candidates.size()));
      candidates.emplace_back((Candidate){parent, action, eval, hash});
      if (candidates.size() == beam_width) {
        seg.build(costs);
        full = true;
      }
    }
  }

  [[nodiscard]] const vector<Candidate> &get_candidates() {
    return candidates;
  }

  [[nodiscard]] vector<Candidate> get_finished_candidate() const {
    return finished_candidates;
  }

  [[nodiscard]] Candidate get_best_candidate() const {
    assert(not candidates.empty());
    return candidates[ranges::min_element(costs) - costs.begin()];
  }

  [[nodiscard]] bool is_finished() const {
    return not finished_candidates.empty();
  }

  void clear() {
    candidates.clear();
    hash_to_index.clear();
    costs.clear();
    full = false;
  }
};

template<typename StateType>
struct EulerTourTree {
  using Action = typename StateType::Action;
  using Evaluator = typename StateType::Evaluator;
  using Hash = typename StateType::Hash;
  using Candidate = typename BeamSelector<StateType>::Candidate;

  using Edge = pair<int, Action>;
  using Vertex = pair<Evaluator, Hash>;

  explicit EulerTourTree(StateType state, const int beam_width) : state(move(state)), buckets(beam_width) {
  }

  void dfs(BeamSelector<StateType> &selector) {
    if (curr_tour.empty()) {
      const auto &[eval, hash] = state.make_initial_node();
      state.expand(eval,
                   hash,
                   [&](const Action &a, const Evaluator &e, const Hash &h, bool f) {
                     return selector.push(a, e, h, 0, f);
                   });
      return;
    }

    for (const auto &[i, action] : curr_tour) {
      if (i >= 0) {
        state.apply(action);
        const auto &[eval, hash] = leaves[i];
        state.expand(eval,
                     hash,
                     [&](const Action &a, const Evaluator &e, const Hash &h, bool f) {
                       return selector.push(a, e, h, i, f);
                     });
        state.rollback(action);
      } else if (i == -1) {
        state.apply(action);
      } else {
        state.rollback(action);
      }
    }
  }

  void update(const vector<Candidate> &candidates) {
    leaves.clear();

    if (curr_tour.empty()) {
      for (const auto &candidate : candidates) {
        curr_tour.emplace_back(static_cast<int>(leaves.size()), candidate.action);
        leaves.emplace_back(candidate.eval, candidate.hash);
      }
      return;
    }

    for (int i = 0; i < static_cast<int>(candidates.size()); i++) {
      buckets[candidates[i].parent].emplace_back(i);
    }

    auto it = curr_tour.begin();

    while (it->first == -1 and it->second == curr_tour.back().second) {
      const auto &action = it++->second;
      state.apply(action);
      road.emplace_back(action);
      curr_tour.pop_back();
    }

    while (it != curr_tour.end()) {
      const auto &[leaf_index, action] = *it++;
      if (leaf_index >= 0) {
        if (buckets[leaf_index].empty()) {
          continue;
        }
        next_tour.emplace_back(-1, action);
        for (int i : buckets[leaf_index]) {
          int j = static_cast<int>(leaves.size());
          next_tour.emplace_back(j, candidates[i].action);
          leaves.emplace_back(candidates[i].eval, candidates[i].hash);
        }
        buckets[leaf_index].clear();
        next_tour.emplace_back(-2, action);
      } else if (leaf_index == -1) {
        next_tour.emplace_back(-1, action);
      } else if (next_tour.back().first == -1) {
        next_tour.pop_back();
      } else {
        next_tour.emplace_back(-2, action);
      }
    }
    curr_tour.swap(next_tour);
    next_tour.clear();
  }

  [[nodiscard]] vector<Action> restore(int parent, int turn) const {
    vector<Action> ret = road;
    ret.reserve(turn);
    for (const auto &[leaf_index, action] : curr_tour) {
      if (leaf_index >= 0) {
        if (leaf_index == parent) {
          ret.push_back(action);
          return ret;
        }
      } else if (leaf_index == -1) {
        ret.push_back(action);
      } else {
        ret.pop_back();
      }
    }
    return {};
  }

  StateType state;
  vector<Action> road;
  vector<Edge> curr_tour, next_tour;
  vector<vector<int> > buckets;
  vector<Vertex> leaves;
};

template<BeamState State>
vector<typename State::Action> beam_search(const State &state,
                                           const size_t max_turn,
                                           size_t beam_width,
                                           size_t hash_map_capacity = 0) {
  if (hash_map_capacity == 0) {
    hash_map_capacity = 16 * 3 * beam_width;
  }

  EulerTourTree<State> tree(state, beam_width);
  BeamSelector<State> selector(beam_width, hash_map_capacity);

  for (size_t turn = 0; turn < max_turn; turn++) {
    tree.dfs(selector);

    if (selector.is_finished()) {
      auto finished = selector.get_finished_candidate()[0];
      auto path = tree.restore(finished.parent, turn + 1);
      path.emplace_back(finished.action);
      return path;
    }

    const auto &candidates = selector.get_candidates();
    if (candidates.empty()) {
      return {};
    }

    if (turn + 1 == max_turn) {
      auto best = selector.get_best_candidate();
      auto path = tree.restore(best.parent, turn + 1);
      path.emplace_back(best.action);
      return path;
    }

    tree.update(candidates);
    selector.clear();
  }
  return {};
}
}
