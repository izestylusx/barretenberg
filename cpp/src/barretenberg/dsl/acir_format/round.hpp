// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#include <cstdint>

namespace acir_format {

// Rounds a number to the nearest multiple of 8
uint32_t round_to_nearest_mul_8(uint32_t num_bits);

// Rounds the number of bits to the nearest byte
uint32_t round_to_nearest_byte(uint32_t num_bits);

} // namespace acir_format
