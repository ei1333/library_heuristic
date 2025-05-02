namespace HillClimbing {
template<typename State>
concept HCState = requires(State s)
{
  { s.update() } -> std::same_as<void>;
};
template<HCState State>
void hill_climbing(State &state, const int end_milliseconds, const int step = 256) {
  const Timer timer;
  const auto start_time = timer.get_milliseconds();
  const auto end_time = start_time + end_milliseconds;
  while (true) {
    if (const auto now = timer.get_milliseconds(); now >= end_time) {
      break;
    }
    for (int i = 0; i < step; i++) {
      state.update();
    }
  }
}
}
