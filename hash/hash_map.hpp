template<class Key, class T>
struct HashMap {
  explicit HashMap(size_t n) : n(n), generation(1), valid(n), data(n) {
  }

  [[nodiscard]] pair<bool, int> get_index(Key key) const {
    Key i = key % n;
    while (valid[i] == generation) {
      if (data[i].first == key) {
        return {true, i};
      }
      if (++i == n) {
        i = 0;
      }
    }
    return {false, i};
  }

  void set(int i, Key key, T value) {
    valid[i] = generation;
    data[i] = {key, value};
  }

  [[nodiscard]] T get(int i) const {
    return data[i].second;
  }

  void clear() {
    ++generation;
  }

  private:
    size_t n;
    size_t generation;
    vector<int> valid;
    vector<pair<Key, T> > data;
};
