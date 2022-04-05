#ifndef _FRN_LIB_PRIMITIVES_HASH_H
#define _FRN_LIB_PRIMITIVES_HASH_H

#include <array>
#include <cstdint>
#include <vector>

namespace frn::lib {
namespace primitives {

static constexpr unsigned int compute_capacity(int bit_size) {
  return 2 * bit_size / (8 * sizeof(uint64_t));
}

/**
 * @brief Tag for SHA-3 with 256 bit output.
 */
struct SHA3_256 {
  //! type of a digest
  using DigestType = std::array<unsigned char, 256 / 8>;
  //! the capacity in words
  static constexpr unsigned int kCapacity = compute_capacity(256);
};

/**
 * @brief Tag for SHA-3 with 384 bit output.
 */
struct SHA3_384 {
  //! type of a digest
  using DigestType = std::array<unsigned char, 384 / 8>;
  //! the capacity in words
  static constexpr unsigned int kCapacity = compute_capacity(384);
};

/**
 * @brief Tag for SHA-3 with 512 bit output.
 */
struct SHA3_512 {
  //! type of a digest
  using DigestType = std::array<unsigned char, 512 / 8>;
  //! the capacity in words
  static constexpr unsigned int kCapacity = compute_capacity(512);
};

/**
 * @brief A hash function.
 *
 * Hash defines an Initialize-Update-Finalize style interface for a hash
 * function (concretely, SHA-3). The overloads to update mimic those in PRG.
 *
 * The implementation is based on SHA-3 reference implementation which can be
 * found at https://github.com/brainhub/SHA3IUF.
 *
 * @tparam tag a Tag specifying the type of SHA-3 use.
 */
template <typename Tag>
class Hash {
 public:
  /**
   * @brief The type of the final digest. Will be an STL array of a some size.
   */
  using DigestType = typename Tag::DigestType;

  /**
   * @brief Initialize the hash function.
   */
  Hash(){};

  /**
   * @brief Update the hash function with a set of bytes.
   *
   * @param bytes a pointer to a number of bytes.
   * @param nbytes the number of bytes.
   * @return the updated Hash object.
   */
  Hash &Update(const unsigned char *bytes, std::size_t nbytes);

  /**
   * @brief Update the hash function with the content from a byte vector.
   *
   * @param bytes a vector of bytes.
   * @return the updated Hash object.
   */
  Hash &Update(const std::vector<unsigned char> &bytes) {
    return Update(bytes.data(), bytes.size());
  };

  /**
   * @brief Finalize and return the digest.
   */
  DigestType Finalize();

 private:
  static const std::size_t kStateSize = 25;
  static const std::size_t kCuttoff =
      kStateSize - (Tag::kCapacity & (~0x80000000));

  uint64_t mState[kStateSize] = {0};
  unsigned char mStateBytes[kStateSize * 8] = {0};
  uint64_t mSaved = 0;
  unsigned int mByteIndex = 0;
  unsigned int mWordIndex = 0;
};

static const uint64_t keccakf_rndc[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};

static const unsigned int keccakf_rotc[24] = {1,  3,  6,  10, 15, 21, 28, 36,
                                              45, 55, 2,  14, 27, 41, 56, 8,
                                              25, 43, 62, 18, 39, 61, 20, 44};

static const unsigned int keccakf_piln[24] = {10, 7,  11, 17, 18, 3,  5,  16,
                                              8,  21, 24, 4,  15, 23, 19, 13,
                                              12, 2,  20, 14, 22, 9,  6,  1};

static inline uint64_t rotl64(uint64_t x, uint64_t y) {
  return (x << y) | (x >> ((sizeof(uint64_t) * 8) - y));
}

static inline void keccakf(uint64_t state[25]) {
  uint64_t t;
  uint64_t bc[5];

  for (std::size_t round = 0; round < 24; ++round) {
    for (std::size_t i = 0; i < 5; ++i)
      bc[i] = state[i] ^ state[i + 5] ^ state[i + 10] ^ state[i + 15] ^
              state[i + 20];

    for (std::size_t i = 0; i < 5; ++i) {
      t = bc[(i + 4) % 5] ^ rotl64(bc[(i + 1) % 5], 1);
      for (std::size_t j = 0; j < 25; j += 5) state[j + i] ^= t;
    }

    t = state[1];
    for (std::size_t i = 0; i < 24; ++i) {
      const uint64_t v = keccakf_piln[i];
      bc[0] = state[v];
      state[v] = rotl64(t, keccakf_rotc[i]);
      t = bc[0];
    }

    for (std::size_t j = 0; j < 25; j += 5) {
      for (std::size_t i = 0; i < 5; ++i) bc[i] = state[j + i];
      for (std::size_t i = 0; i < 5; ++i)
        state[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
    }

    state[0] ^= keccakf_rndc[round];
  }
}

template <typename Tag>
Hash<Tag> &Hash<Tag>::Update(const unsigned char *bytes, std::size_t nbytes) {
  unsigned int old_tail = (8 - mByteIndex) & 7;
  const unsigned char *p = bytes;

  if (nbytes < old_tail) {
    while (nbytes--) mSaved |= (uint64_t)(*(p++)) << ((mByteIndex++) * 8);
    return *this;
  }

  if (old_tail) {
    nbytes -= old_tail;
    while (old_tail--) mSaved |= (uint64_t)(*(p++)) << ((mByteIndex++) * 8);

    mState[mWordIndex] ^= mSaved;
    mByteIndex = 0;
    mSaved = 0;

    if (++mWordIndex == kCuttoff) {
      keccakf(mState);
      mWordIndex = 0;
    }
  }

  std::size_t words = nbytes / sizeof(uint64_t);
  unsigned int tail = nbytes - words * sizeof(uint64_t);

  for (std::size_t i = 0; i < words; ++i) {
    const uint64_t t =
        (uint64_t)(p[0]) | ((uint64_t)(p[1]) << 8 * 1) |
        ((uint64_t)(p[1]) << 8 * 2) | ((uint64_t)(p[1]) << 8 * 3) |
        ((uint64_t)(p[1]) << 8 * 4) | ((uint64_t)(p[1]) << 8 * 5) |
        ((uint64_t)(p[1]) << 8 * 6) | ((uint64_t)(p[1]) << 8 * 7);

    mState[mWordIndex] ^= t;

    if (++mWordIndex == kCuttoff) {
      keccakf(mState);
      mWordIndex = 0;
    }
    p += sizeof(uint64_t);
  }

  while (tail--) mSaved |= (uint64_t)(*(p++)) << ((mByteIndex++) * 8);

  return *this;
}

template <typename Tag>
auto Hash<Tag>::Finalize() -> DigestType {
  uint64_t t = (uint64_t)(((uint64_t)(0x02 | (1 << 2))) << ((mByteIndex)*8));
  mState[mWordIndex] ^= mSaved ^ t;
  mState[kCuttoff - 1] ^= 0x8000000000000000ULL;
  keccakf(mState);

  for (std::size_t i = 0; i < kStateSize; ++i) {
    const unsigned int t1 = (uint32_t)mState[i];
    const unsigned int t2 = (uint32_t)((mState[i] >> 16) >> 16);
    mStateBytes[i * 8 + 0] = (unsigned char)t1;
    mStateBytes[i * 8 + 1] = (unsigned char)(t1 >> 8);
    mStateBytes[i * 8 + 2] = (unsigned char)(t1 >> 16);
    mStateBytes[i * 8 + 3] = (unsigned char)(t1 >> 24);
    mStateBytes[i * 8 + 4] = (unsigned char)t2;
    mStateBytes[i * 8 + 5] = (unsigned char)(t2 >> 8);
    mStateBytes[i * 8 + 6] = (unsigned char)(t2 >> 16);
    mStateBytes[i * 8 + 7] = (unsigned char)(t2 >> 24);
  }

  // truncate
  DigestType digest = {0};
  for (std::size_t i = 0; i < digest.size(); ++i) digest[i] = mStateBytes[i];

  return digest;
}

}  // namespace primitives
}  // namespace frn::lib

#endif  // _FRN_LIB_PRIMITIVES_HASH_H
