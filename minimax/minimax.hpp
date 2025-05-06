namespace MiniMax {
template<class State>
concept MiniMaxState =
    requires(State &s,
             const State &cs,
             typename State::Action &a,
             function<void(const typename State::Action &)> push)
{
  typename State::Action;
  typename State::Cost;

  { cs.expand(push) } -> same_as<void>;
  { cs.is_finished() } -> same_as<bool>;
  { cs.evaluate() } -> same_as<typename State::Cost>;
  { s.apply(a) } -> same_as<void>;
  { s.rollback(a) } -> same_as<void>;
} &&
totally_ordered<typename State::Cost>;

template<MiniMaxState State>
typename State::Cost get_best_score(State &state, const size_t depth) {
  using Action = typename State::Action;
  using Cost = typename State::Cost;

  if (depth == 0 or state.is_finished()) {
    return state.evaluate();
  }

  vector<Action> candidates;
  state.expand([&](const Action &a) { candidates.emplace_back(a); });

  if (candidates.empty()) {
    return state.evaluate();
  }

  Cost best_score = numeric_limits<Cost>::min();
  for (const auto &action : candidates) {
    state.apply(action);
    Cost score = -get_best_score(state, depth - 1);
    if (score > best_score) {
      best_score = score;
    }
    state.rollback(action);
  }
  return best_score;
}

template<MiniMaxState State>
typename State::Action get_best_action(State &state, const size_t depth) {
  using Action = typename State::Action;
  using Cost = typename State::Cost;
  assert(depth > 0 and not state.is_finished());
  vector<Action> candidates;
  state.expand([&](const Action &a) { candidates.emplace_back(a); });
  assert(not candidates.empty());
  Cost best_score = numeric_limits<Cost>::min();
  Action best_action;
  for (const auto &action : candidates) {
    state.apply(action);
    Cost score = -get_best_score(state, depth - 1);
    if (score > best_score) {
      best_score = score;
      best_action = action;
    }
    state.rollback(action);
  }
  return best_action;
}
}
