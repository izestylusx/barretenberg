// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#include "barretenberg/common/thread.hpp"
#include "gemini_impl.hpp"
namespace bb {
template class GeminiProver_<curve::BN254>;
template class GeminiProver_<curve::Grumpkin>;
}; // namespace bb