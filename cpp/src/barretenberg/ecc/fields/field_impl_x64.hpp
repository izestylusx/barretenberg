// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once

#if (BBERG_NO_ASM == 0)
#include "./field_impl.hpp"
#include "asm_macros.hpp"
namespace bb {

template <class T> field<T> field<T>::asm_mul_with_coarse_reduction(const field& a, const field& b) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_mul_with_coarse_reduction");

    field r;
    constexpr uint64_t r_inv = T::r_inv;
    constexpr uint64_t modulus_0 = modulus.data[0];
    constexpr uint64_t modulus_1 = modulus.data[1];
    constexpr uint64_t modulus_2 = modulus.data[2];
    constexpr uint64_t modulus_3 = modulus.data[3];
    constexpr uint64_t zero_ref = 0;

    /**
     * Registers: rax:rdx = multiplication accumulator
     *            %r12, %r13, %r14, %r15, %rax: work registers for `r`
     *            %r8, %r9, %rdi, %rsi: scratch registers for multiplication results
     *            %r10: zero register
     *            %0: pointer to `a`
     *            %1: pointer to `b`
     *            %2: pointer to `r`
     **/
    __asm__(MUL("0(%0)", "8(%0)", "16(%0)", "24(%0)", "%1")
                STORE_FIELD_ELEMENT("%2", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "%r"(&a),
              "%r"(&b),
              "r"(&r),
              [modulus_0] "m"(modulus_0),
              [modulus_1] "m"(modulus_1),
              [modulus_2] "m"(modulus_2),
              [modulus_3] "m"(modulus_3),
              [r_inv] "m"(r_inv),
              [zero_reference] "m"(zero_ref)
            : "%rdx", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
    return r;
}

template <class T> void field<T>::asm_self_mul_with_coarse_reduction(const field& a, const field& b) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_self_mul_with_coarse_reduction");

    constexpr uint64_t r_inv = T::r_inv;
    constexpr uint64_t modulus_0 = modulus.data[0];
    constexpr uint64_t modulus_1 = modulus.data[1];
    constexpr uint64_t modulus_2 = modulus.data[2];
    constexpr uint64_t modulus_3 = modulus.data[3];
    constexpr uint64_t zero_ref = 0;
    /**
     * Registers: rax:rdx = multiplication accumulator
     *            %r12, %r13, %r14, %r15, %rax: work registers for `r`
     *            %r8, %r9, %rdi, %rsi: scratch registers for multiplication results
     *            %r10: zero register
     *            %0: pointer to `a`
     *            %1: pointer to `b`
     *            %2: pointer to `r`
     **/
    __asm__(MUL("0(%0)", "8(%0)", "16(%0)", "24(%0)", "%1")
                STORE_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "r"(&a),
              "r"(&b),
              [modulus_0] "m"(modulus_0),
              [modulus_1] "m"(modulus_1),
              [modulus_2] "m"(modulus_2),
              [modulus_3] "m"(modulus_3),
              [r_inv] "m"(r_inv),
              [zero_reference] "m"(zero_ref)
            : "%rdx", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
}

template <class T> field<T> field<T>::asm_sqr_with_coarse_reduction(const field& a) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_sqr_with_coarse_reduction");

    field r;
    constexpr uint64_t r_inv = T::r_inv;
    constexpr uint64_t modulus_0 = modulus.data[0];
    constexpr uint64_t modulus_1 = modulus.data[1];
    constexpr uint64_t modulus_2 = modulus.data[2];
    constexpr uint64_t modulus_3 = modulus.data[3];
    constexpr uint64_t zero_ref = 0;

// Our SQR implementation with BMI2 but without ADX has a bug.
// The case is extremely rare so fixing it is a bit of a waste of time.
// We'll use MUL instead.
#if !defined(__ADX__) || defined(DISABLE_ADX)
    /**
     * Registers: rax:rdx = multiplication accumulator
     *            %r12, %r13, %r14, %r15, %rax: work registers for `r`
     *            %r8, %r9, %rdi, %rsi: scratch registers for multiplication results
     *            %r10: zero register
     *            %0: pointer to `a`
     *            %1: pointer to `b`
     *            %2: pointer to `r`
     **/
    __asm__(MUL("0(%0)", "8(%0)", "16(%0)", "24(%0)", "%1")
                STORE_FIELD_ELEMENT("%2", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "%r"(&a),
              "%r"(&a),
              "r"(&r),
              [modulus_0] "m"(modulus_0),
              [modulus_1] "m"(modulus_1),
              [modulus_2] "m"(modulus_2),
              [modulus_3] "m"(modulus_3),
              [r_inv] "m"(r_inv),
              [zero_reference] "m"(zero_ref)
            : "%rdx", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");

#else

    /**
     * Registers: rax:rdx = multiplication accumulator
     *            %r12, %r13, %r14, %r15, %rax: work registers for `r`
     *            %r8, %r9, %rdi, %rsi: scratch registers for multiplication results
     *            %[zero_reference]: memory location of zero value
     *            %0: pointer to `a`
     *            %[r_ptr]: memory location of pointer to `r`
     **/
    __asm__(SQR("%0")
            // "movq %[r_ptr], %%rsi                   \n\t"
            STORE_FIELD_ELEMENT("%1", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "r"(&a),
              "r"(&r),
              [zero_reference] "m"(zero_ref),
              [modulus_0] "m"(modulus_0),
              [modulus_1] "m"(modulus_1),
              [modulus_2] "m"(modulus_2),
              [modulus_3] "m"(modulus_3),
              [r_inv] "m"(r_inv)
            : "%rcx", "%rdx", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
#endif
    return r;
}

template <class T> void field<T>::asm_self_sqr_with_coarse_reduction(const field& a) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_self_sqr_with_coarse_reduction");

    constexpr uint64_t r_inv = T::r_inv;
    constexpr uint64_t modulus_0 = modulus.data[0];
    constexpr uint64_t modulus_1 = modulus.data[1];
    constexpr uint64_t modulus_2 = modulus.data[2];
    constexpr uint64_t modulus_3 = modulus.data[3];
    constexpr uint64_t zero_ref = 0;

// Our SQR implementation with BMI2 but without ADX has a bug.
// The case is extremely rare so fixing it is a bit of a waste of time.
// We'll use MUL instead.
#if !defined(__ADX__) || defined(DISABLE_ADX)
    /**
     * Registers: rax:rdx = multiplication accumulator
     *            %r12, %r13, %r14, %r15, %rax: work registers for `r`
     *            %r8, %r9, %rdi, %rsi: scratch registers for multiplication results
     *            %r10: zero register
     *            %0: pointer to `a`
     *            %1: pointer to `b`
     *            %2: pointer to `r`
     **/
    __asm__(MUL("0(%0)", "8(%0)", "16(%0)", "24(%0)", "%1")
                STORE_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "r"(&a),
              "r"(&a),
              [modulus_0] "m"(modulus_0),
              [modulus_1] "m"(modulus_1),
              [modulus_2] "m"(modulus_2),
              [modulus_3] "m"(modulus_3),
              [r_inv] "m"(r_inv),
              [zero_reference] "m"(zero_ref)
            : "%rdx", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");

#else
    /**
     * Registers: rax:rdx = multiplication accumulator
     *            %r12, %r13, %r14, %r15, %rax: work registers for `r`
     *            %r8, %r9, %rdi, %rsi: scratch registers for multiplication results
     *            %[zero_reference]: memory location of zero value
     *            %0: pointer to `a`
     *            %[r_ptr]: memory location of pointer to `r`
     **/
    __asm__(SQR("%0")
            // "movq %[r_ptr], %%rsi                   \n\t"
            STORE_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "r"(&a),
              [zero_reference] "m"(zero_ref),
              [modulus_0] "m"(modulus_0),
              [modulus_1] "m"(modulus_1),
              [modulus_2] "m"(modulus_2),
              [modulus_3] "m"(modulus_3),
              [r_inv] "m"(r_inv)
            : "%rcx", "%rdx", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
#endif
}

template <class T> field<T> field<T>::asm_add_with_coarse_reduction(const field& a, const field& b) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_add_with_coarse_reduction");

    field r;

    constexpr uint64_t twice_not_modulus_0 = twice_not_modulus.data[0];
    constexpr uint64_t twice_not_modulus_1 = twice_not_modulus.data[1];
    constexpr uint64_t twice_not_modulus_2 = twice_not_modulus.data[2];
    constexpr uint64_t twice_not_modulus_3 = twice_not_modulus.data[3];

    __asm__(CLEAR_FLAGS("%%r12") LOAD_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
                ADD_REDUCE("%1",
                           "%[twice_not_modulus_0]",
                           "%[twice_not_modulus_1]",
                           "%[twice_not_modulus_2]",
                           "%[twice_not_modulus_3]") STORE_FIELD_ELEMENT("%2", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "%r"(&a),
              "%r"(&b),
              "r"(&r),
              [twice_not_modulus_0] "m"(twice_not_modulus_0),
              [twice_not_modulus_1] "m"(twice_not_modulus_1),
              [twice_not_modulus_2] "m"(twice_not_modulus_2),
              [twice_not_modulus_3] "m"(twice_not_modulus_3)
            : "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
    return r;
}

template <class T> void field<T>::asm_self_add_with_coarse_reduction(const field& a, const field& b) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_self_add_with_coarse_reduction");

    constexpr uint64_t twice_not_modulus_0 = twice_not_modulus.data[0];
    constexpr uint64_t twice_not_modulus_1 = twice_not_modulus.data[1];
    constexpr uint64_t twice_not_modulus_2 = twice_not_modulus.data[2];
    constexpr uint64_t twice_not_modulus_3 = twice_not_modulus.data[3];

    __asm__(CLEAR_FLAGS("%%r12") LOAD_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
                ADD_REDUCE("%1",
                           "%[twice_not_modulus_0]",
                           "%[twice_not_modulus_1]",
                           "%[twice_not_modulus_2]",
                           "%[twice_not_modulus_3]") STORE_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "r"(&a),
              "r"(&b),
              [twice_not_modulus_0] "m"(twice_not_modulus_0),
              [twice_not_modulus_1] "m"(twice_not_modulus_1),
              [twice_not_modulus_2] "m"(twice_not_modulus_2),
              [twice_not_modulus_3] "m"(twice_not_modulus_3)
            : "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
}

template <class T> field<T> field<T>::asm_sub_with_coarse_reduction(const field& a, const field& b) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_sub_with_coarse_reduction");

    field r;

    constexpr uint64_t twice_modulus_0 = twice_modulus.data[0];
    constexpr uint64_t twice_modulus_1 = twice_modulus.data[1];
    constexpr uint64_t twice_modulus_2 = twice_modulus.data[2];
    constexpr uint64_t twice_modulus_3 = twice_modulus.data[3];

    __asm__(
        CLEAR_FLAGS("%%r12") LOAD_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15") SUB("%1")
            REDUCE_FIELD_ELEMENT("%[twice_modulus_0]", "%[twice_modulus_1]", "%[twice_modulus_2]", "%[twice_modulus_3]")
                STORE_FIELD_ELEMENT("%2", "%%r12", "%%r13", "%%r14", "%%r15")
        :
        : "r"(&a),
          "r"(&b),
          "r"(&r),
          [twice_modulus_0] "m"(twice_modulus_0),
          [twice_modulus_1] "m"(twice_modulus_1),
          [twice_modulus_2] "m"(twice_modulus_2),
          [twice_modulus_3] "m"(twice_modulus_3)
        : "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
    return r;
}

template <class T> void field<T>::asm_self_sub_with_coarse_reduction(const field& a, const field& b) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_self_sub_with_coarse_reduction");

    constexpr uint64_t twice_modulus_0 = twice_modulus.data[0];
    constexpr uint64_t twice_modulus_1 = twice_modulus.data[1];
    constexpr uint64_t twice_modulus_2 = twice_modulus.data[2];
    constexpr uint64_t twice_modulus_3 = twice_modulus.data[3];

    __asm__(
        CLEAR_FLAGS("%%r12") LOAD_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15") SUB("%1")
            REDUCE_FIELD_ELEMENT("%[twice_modulus_0]", "%[twice_modulus_1]", "%[twice_modulus_2]", "%[twice_modulus_3]")
                STORE_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
        :
        : "r"(&a),
          "r"(&b),
          [twice_modulus_0] "m"(twice_modulus_0),
          [twice_modulus_1] "m"(twice_modulus_1),
          [twice_modulus_2] "m"(twice_modulus_2),
          [twice_modulus_3] "m"(twice_modulus_3)
        : "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
}

template <class T> void field<T>::asm_conditional_negate(field& r, const uint64_t predicate) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_conditional_negate");

    constexpr uint64_t twice_modulus_0 = twice_modulus.data[0];
    constexpr uint64_t twice_modulus_1 = twice_modulus.data[1];
    constexpr uint64_t twice_modulus_2 = twice_modulus.data[2];
    constexpr uint64_t twice_modulus_3 = twice_modulus.data[3];

    __asm__(CLEAR_FLAGS("%%r8") LOAD_FIELD_ELEMENT(
                "%1", "%%r8", "%%r9", "%%r10", "%%r11") "movq %[twice_modulus_0], %%r12 \n\t"
                                                        "movq %[twice_modulus_1], %%r13 \n\t"
                                                        "movq %[twice_modulus_2], %%r14 \n\t"
                                                        "movq %[twice_modulus_3], %%r15 \n\t"
                                                        "subq %%r8, %%r12 \n\t"
                                                        "sbbq %%r9, %%r13 \n\t"
                                                        "sbbq %%r10, %%r14 \n\t"
                                                        "sbbq %%r11, %%r15 \n\t"
                                                        "testq %0, %0 \n\t"
                                                        "cmovnzq %%r12, %%r8 \n\t"
                                                        "cmovnzq %%r13, %%r9 \n\t"
                                                        "cmovnzq %%r14, %%r10 \n\t"
                                                        "cmovnzq %%r15, %%r11 \n\t" STORE_FIELD_ELEMENT(
                                                            "%1", "%%r8", "%%r9", "%%r10", "%%r11")
            :
            : "r"(predicate),
              "r"(&r),
              [twice_modulus_0] "i"(twice_modulus_0),
              [twice_modulus_1] "i"(twice_modulus_1),
              [twice_modulus_2] "i"(twice_modulus_2),
              [twice_modulus_3] "i"(twice_modulus_3)
            : "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
}

template <class T> field<T> field<T>::asm_reduce_once(const field& a) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_reduce_once");

    field r;

    constexpr uint64_t not_modulus_0 = not_modulus.data[0];
    constexpr uint64_t not_modulus_1 = not_modulus.data[1];
    constexpr uint64_t not_modulus_2 = not_modulus.data[2];
    constexpr uint64_t not_modulus_3 = not_modulus.data[3];

    __asm__(CLEAR_FLAGS("%%r12") LOAD_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
                REDUCE_FIELD_ELEMENT("%[not_modulus_0]", "%[not_modulus_1]", "%[not_modulus_2]", "%[not_modulus_3]")
                    STORE_FIELD_ELEMENT("%1", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "r"(&a),
              "r"(&r),
              [not_modulus_0] "m"(not_modulus_0),
              [not_modulus_1] "m"(not_modulus_1),
              [not_modulus_2] "m"(not_modulus_2),
              [not_modulus_3] "m"(not_modulus_3)
            : "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
    return r;
}

template <class T> void field<T>::asm_self_reduce_once(const field& a) noexcept
{
    BB_OP_COUNT_TRACK_NAME("fr::asm_self_reduce_once");

    constexpr uint64_t not_modulus_0 = not_modulus.data[0];
    constexpr uint64_t not_modulus_1 = not_modulus.data[1];
    constexpr uint64_t not_modulus_2 = not_modulus.data[2];
    constexpr uint64_t not_modulus_3 = not_modulus.data[3];

    __asm__(CLEAR_FLAGS("%%r12") LOAD_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
                REDUCE_FIELD_ELEMENT("%[not_modulus_0]", "%[not_modulus_1]", "%[not_modulus_2]", "%[not_modulus_3]")
                    STORE_FIELD_ELEMENT("%0", "%%r12", "%%r13", "%%r14", "%%r15")
            :
            : "r"(&a),
              [not_modulus_0] "m"(not_modulus_0),
              [not_modulus_1] "m"(not_modulus_1),
              [not_modulus_2] "m"(not_modulus_2),
              [not_modulus_3] "m"(not_modulus_3)
            : "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
}
} // namespace bb
#endif