#ifndef __OAZ_BITBOARD_HELPERS_HPP__
#define __OAZ_BITBOARD_HELPERS_HPP__

namespace oaz::bitboard {
template <size_t NROWS, size_t NCOLS>
struct ColumnMask {
  static constexpr size_t value =
      ((ColumnMask<NROWS - 1, NCOLS>::value) << NCOLS) | 1ll;
};
template <size_t NCOLS>
struct ColumnMask<1, NCOLS> {
  static constexpr size_t value = 1ll;
};

inline size_t popcount_ll(uint64_t bits) {
  return std::bitset<64>(bits).count();
}

inline size_t locount_ll(uint64_t bits) { return __builtin_clzll(~bits); }

inline size_t tocount_ll(uint64_t bits) { return __builtin_ctzll(~bits); }
}  // namespace oaz::bitboard
#endif
