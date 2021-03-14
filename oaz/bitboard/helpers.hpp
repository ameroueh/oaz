#ifndef OAZ_BITBOARD_HELPERS_HPP_
  #define OAZ_BITBOARD_HELPERS_HPP

  #include <stdint.h>

  #include <bitset>

namespace oaz::bitboard {
template <uint64_t NROWS, uint64_t NCOLS>
struct ColumnMask {
  static constexpr uint64_t value =
      ((ColumnMask<NROWS - 1, NCOLS>::value) << NCOLS) | 1ll;
};
template <uint64_t NCOLS>
struct ColumnMask<1, NCOLS> {
  static constexpr uint64_t value = 1ll;
};

inline uint64_t popcount_ll(uint64_t bits) {
  return std::bitset<64>(bits).count();
}

inline uint64_t locount_ll(uint64_t bits) { return __builtin_clzll(~bits); }

inline uint64_t tocount_ll(uint64_t bits) { return __builtin_ctzll(~bits); }
}  // namespace oaz::bitboard
#endif  // OAZ_BITBOARD_HELPERS_HPP_
