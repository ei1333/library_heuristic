template<class Key, class T>
struct HashMap {
  explicit HashMap(size_t n_) {
    if (n_ % 2 == 0) {
      ++n_;
    }
    n = n_;
    valid.resize(n, false);
    data.resize(n);
  }

  [[nodiscard]] pair<bool, int> get_index(Key key) const {
    Key i = key % n;
    while (valid[i]) {
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
    valid[i] = true;
    data[i] = {key, value};
  }

  [[nodiscard]] T get(int i) const {
    assert(valid[i]);
    return data[i].second;
  }

  void clear() {
    fill(valid.begin(), valid.end(), false);
  }

  private:
    size_t n;
    vector<bool> valid;
    vector<pair<Key, T> > data;
};
