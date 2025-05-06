namespace AlphaBeta {
template<class State>
concept AlphaBeta =
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

template<AlphaBeta State>
typename State::Cost get_best_score(State &state, typename State::Cost alpha, typename State::Cost beta, const size_t depth) {
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

  for (const auto &action : candidates) {
    state.apply(action);
    Cost score = -get_best_score(state, -beta, -alpha, depth - 1);
    if (score > alpha) {
      alpha = score;
    }
    state.rollback(action);
    if (alpha >= beta) {
      return alpha;
    }
  }
  return alpha;
}

template<AlphaBeta State>
typename State::Action get_best_action(State &state, const size_t depth) {
  using Action = typename State::Action;
  using Cost = typename State::Cost;
  assert(depth > 0 and not state.is_finished());
  vector<Action> candidates;
  state.expand([&](const Action &a) { candidates.emplace_back(a); });
  assert(not candidates.empty());
  Cost alpha = -numeric_limits<Cost>::max();
  Cost beta = numeric_limits<Cost>::max();
  Action best_action;
  for (const auto &action : candidates) {
    state.apply(action);
    Cost score = -get_best_score(state, -beta, -alpha, depth - 1);
    if (score > alpha) {
      alpha = score;
      best_action = action;
    }
    state.rollback(action);
  }
  return best_action;
}
}
