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
pair<typename State::Action, typename State::Cost> minimax(State &state, const int depth) {
  using Action = typename State::Action;
  using Evaluator = typename State::Evaluator;
  using Cost = typename Evaluator::Cost;
  if (depth == 0 or state.is_finished()) {
    return state.eval.evaluate();
  }
  vector<Action> candidates;
  state.expand([&](const Action &a) { candidates.emplace_back(a); });
  Cost best_score = numeric_limits<Cost>::min();
  Action best_action;
  for (const auto &action : candidates) {
    state.apply(action);
    auto score = -minimax(state, depth - 1).second;
    if (score > best_score) {
      best_score = score;
      best_action = action;
    }
    state.rollback(action);
  }
  return {best_action, best_score};
}
}
