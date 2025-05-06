// https://atcoder.jp/contests/abc025/tasks/abc025_c

struct MyState {
  using Action = pair< int, int >;
  using Cost = int;

  explicit MyState() : b{}, c{}, visited{}, turn{1} {
    for (auto & i : b) {
      for (int & j : i) {
        cin >> j;
      }
    }
    for (auto & i : c) {
      for (int & j : i) {
        cin >> j;
      }
    }
  }

  [[nodiscard]] bool is_finished() const {
    return turn == 10;
  }

  [[nodiscard]] pair< Cost, Cost > get_score() const {
    Cost takahashi = 0, naoko = 0;
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 3; j++) {
        if (visited[i][j] != 0 and visited[i + 1][j] != 0) {
          if (visited[i][j] % 2 == visited[i + 1][j] % 2) {
            takahashi += b[i][j];
          } else {
            naoko += b[i][j];
          }
        }
      }
    }
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 2; j++) {
        if (visited[i][j] != 0 and visited[i][j + 1] != 0) {
          if (visited[i][j] % 2 == visited[i][j + 1] % 2) {
            takahashi += c[i][j];
          } else {
            naoko += c[i][j];
          }
        }
      }
    }
    return {takahashi, naoko};
  }

  [[nodiscard]] Cost evaluate() const {
    auto [takahashi, naoko] = get_score();
    if (turn % 2 == 1) return takahashi - naoko;
    return naoko - takahashi;
  }

  void expand(const function<void(const Action &)>& push) const {
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (visited[i][j] == 0) {
          push({i, j});
        }
      }
    }
  }

  void apply(const Action &a) {
    visited[a.first][a.second] = turn;
    ++turn;
  }

  void rollback(const Action &a) {
    visited[a.first][a.second] = 0;
    --turn;
  }

  int b[2][3], c[3][2], visited[3][3];
  int turn;
};
