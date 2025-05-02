namespace SimulatedAnnealing {
template<typename State>
concept SAState = requires(State s, double delta, double progress)
{
  { s.update(delta, progress) } -> std::same_as<void>;
};
template<SAState State>
void simulated_annealing(State &state,
                         const double start_temp,
                         const double end_temp,
                         const int end_milliseconds,
                         const int step) {
  const Timer timer;
  XorShift rng;
  const auto start_time = timer.get_milliseconds();
  const auto end_time = start_time + end_milliseconds;
  while (true) {
    const auto now = timer.get_milliseconds();
    if (now >= end_time) {
      break;
    }
    double progress = static_cast<double>(now - start_time) / end_milliseconds;
    const double temp = start_temp + (end_temp - start_temp) * progress;
    for (int i = 0; i < step; i++) {
      state.update(temp * log(rng.probability()), progress);
    }
  }
}
}
