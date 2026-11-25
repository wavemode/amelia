#include "utility.hpp"

#include <cstring>

#include "util/data/runtime_error.hpp"

#define INTRINHASH_HEADER_ONLY
#include "intrinhash.h"

namespace amelia {

namespace {

constexpr uint64_t SEED = 0xc0ffee;

static_assert(sizeof(intrinhash_state) <= internal::HASH_STATE_SIZE);

} // namespace

[[noreturn]] void throw_runtime_error(const char *message) {
  throw RuntimeError(String::from(message));
}

uint64_t hash_str_64(const char *str, size_t len) {
  uint64_t result = 0;
  intrinhash(str, len, SEED, &result, sizeof(result), intrinhash_auto());
  return result;
}

uint32_t hash_str_32(const char *str, size_t len) {
  uint32_t result = 0;
  intrinhash(str, len, SEED, &result, sizeof(result), intrinhash_auto());
  return result;
}

HashState::HashState() noexcept
    : m_data(), m_state(), m_initialized(false), m_length(0) {
  new (&m_state) intrinhash_state{};
}

void HashState::init() noexcept {
  intrinhash_init(
      reinterpret_cast<intrinhash_state *>(&m_state), SEED, intrinhash_auto()
  );
  m_initialized = true;
  m_length = 0;
}

void HashState::update(const char *data, size_t len) {
  if (!m_initialized) {
    throw_runtime_error("HashState not initialized");
  }
  while (m_length + len > internal::HASH_BLOCK_SIZE) {
    if (m_length > 0) {
      size_t copy_amount = internal::HASH_BLOCK_SIZE - m_length;
      std::memcpy(m_data, data, copy_amount);
      intrinhash_update(
          reinterpret_cast<intrinhash_state *>(&m_state),
          m_data,
          internal::HASH_BLOCK_SIZE
      );
      data += copy_amount;
      len -= copy_amount;
      m_length = 0;
    } else {
      intrinhash_update(
          reinterpret_cast<intrinhash_state *>(&m_state),
          data,
          internal::HASH_BLOCK_SIZE
      );
      data += internal::HASH_BLOCK_SIZE;
      len -= internal::HASH_BLOCK_SIZE;
    }
  }
  std::memcpy(m_data + m_length, data, len);
  m_length += len;
}

uint64_t HashState::finalize() {
  if (!m_initialized) {
    throw_runtime_error("HashState not initialized");
  }
  if (m_length) {
    intrinhash_update(
        reinterpret_cast<intrinhash_state *>(&m_state), m_data, m_length
    );
  }
  uint64_t result = 0;
  intrinhash_finalize(
      reinterpret_cast<intrinhash_state *>(&m_state), &result, sizeof(result)
  );
  m_initialized = false;
  return result;
}

} // namespace amelia
