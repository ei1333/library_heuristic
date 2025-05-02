struct XorShift {
  private:
    constexpr static double R = 1.0 / 0xffffffff;
    uint64_t x;

  public:
    explicit XorShift(const uint64_t seed = 88172645463325252ull) : x(seed) {
    }

    template<typename T = uint64_t>
    [[nodiscard]] T get() {
      // [0, 2^64)
      x ^= x << 7ull;
      x ^= x >> 9ull;
      return x;
    }

    [[nodiscard]] uint32_t get(const uint32_t r) {
      // [0, r)
      return (static_cast<uint64_t>(get<uint32_t>()) * r) >> 32ull;
    }

    [[nodiscard]] uint32_t get(const uint32_t l, const uint32_t r) {
      // [l, r)
      return l + get(r - l);
    }

    [[nodiscard]] double probability() {
      // [0.0, 1.0]
      return get<uint32_t>() * R;
    }
};
