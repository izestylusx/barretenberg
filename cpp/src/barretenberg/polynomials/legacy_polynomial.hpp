// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once
#include "barretenberg/common/mem.hpp"
#include "barretenberg/crypto/sha256/sha256.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
// for PolynomialSpan
#include "barretenberg/polynomials/polynomial.hpp"
#include "evaluation_domain.hpp"
#include "polynomial_arithmetic.hpp"
#include <fstream>

namespace bb {

/**
 * @brief Polynomial class that represents the coefficients 'a' of a_0 + a_1 x + a_n x^n of
 * a finite field polynomial equation of degree that is at most the size of some zk circuit.
 * The polynomial is used to represent the gates of our arithmetized zk programs.
 * Polynomials use the majority of the memory in proving, so caution should be used in making sure
 * unnecessary copies are avoided, both for avoiding unnecessary memory usage and performance
 * due to unnecessary allocations.
 * Note: This should not be used for new code, hence the Legacy name. This is only used in older plonk-centric code
 * as opposed to newer honk-centric code.
 *
 * @tparam Fr the finite field type.
 */
template <typename Fr> class LegacyPolynomial {
  public:
    /**
     * Implements requirements of `std::ranges::contiguous_range` and `std::ranges::sized_range`
     */
    using value_type = Fr;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    using pointer = std::shared_ptr<value_type[]>;
    using const_pointer = pointer;
    using iterator = Fr*;
    using const_iterator = Fr const*;
    using FF = Fr;
    enum class DontZeroMemory { FLAG };

    LegacyPolynomial(size_t initial_size);
    // Constructor that does not initialize values, use with caution to save time.
    LegacyPolynomial(size_t initial_size, DontZeroMemory flag);
    LegacyPolynomial(const LegacyPolynomial& other);
    LegacyPolynomial(const LegacyPolynomial& other, size_t target_size);

    LegacyPolynomial(LegacyPolynomial&& other) noexcept;

    // Create a polynomial from the given fields.
    LegacyPolynomial(std::span<const Fr> coefficients);

    // Allow polynomials to be entirely reset/dormant
    LegacyPolynomial() = default;

    /**
     * @brief Create the degree-(m-1) polynomial T(X) that interpolates the given evaluations.
     * We have T(xⱼ) = yⱼ for j=1,...,m
     *
     * @param interpolation_points (x₁,…,xₘ)
     * @param evaluations (y₁,…,yₘ)
     */
    LegacyPolynomial(std::span<const Fr> interpolation_points, std::span<const Fr> evaluations);

    // move assignment
    LegacyPolynomial& operator=(LegacyPolynomial&& other) noexcept;
    LegacyPolynomial& operator=(std::span<const Fr> coefficients) noexcept;
    LegacyPolynomial& operator=(const LegacyPolynomial& other);
    ~LegacyPolynomial() = default;

    /**
     * Return a shallow clone of the polynomial. i.e. underlying memory is shared.
     */
    LegacyPolynomial share() const;

    std::array<uint8_t, 32> hash() const { return crypto::sha256(byte_span()); }

    void clear()
    {
        // to keep the invariant that backing_memory_ can handle capacity() we do NOT reset backing_memory_
        // backing_memory_.reset();
        coefficients_ = nullptr;
        size_ = 0;
    }

    /**
     * @brief Check whether or not a polynomial is identically zero
     *
     */
    bool is_zero()
    {
        if (is_empty()) {
            ASSERT(false);
            info("Checking is_zero on an empty Polynomial!");
        }
        for (size_t i = 0; i < size(); i++) {
            if (coefficients_[i] != 0) {
                return false;
            }
        }
        return true;
    }

    bool operator==(LegacyPolynomial const& rhs) const;

    // Const and non const versions of coefficient accessors
    Fr const& operator[](const size_t i) const { return coefficients_[i]; }

    Fr& operator[](const size_t i) { return coefficients_[i]; }

    // For compatibility with Polynomial (which needs a special mutable accessor)
    Fr const& at(const size_t i) const
    {
        ASSERT(i < capacity());
        return coefficients_[i];
    }

    // For compatibility with Polynomial (which needs a special mutable accessor)
    Fr& at(const size_t i)
    {
        ASSERT(i < capacity());
        return coefficients_[i];
    }

    // For compatibility with Polynomial (which needs a special safe mutation function)
    void set_if_valid_index(size_t index, const Fr& value) { at(index) = value; }

    Fr evaluate(const Fr& z, size_t target_size) const;
    Fr evaluate(const Fr& z) const;

    Fr compute_barycentric_evaluation(const Fr& z, const EvaluationDomain<Fr>& domain)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    Fr evaluate_from_fft(const EvaluationDomain<Fr>& large_domain,
                         const Fr& z,
                         const EvaluationDomain<Fr>& small_domain)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void fft(const EvaluationDomain<Fr>& domain)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void partial_fft(const EvaluationDomain<Fr>& domain, Fr constant = 1, bool is_coset = false)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void coset_fft(const EvaluationDomain<Fr>& domain)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void coset_fft(const EvaluationDomain<Fr>& domain,
                   const EvaluationDomain<Fr>& large_domain,
                   size_t domain_extension)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void coset_fft_with_constant(const EvaluationDomain<Fr>& domain, const Fr& constant)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void coset_fft_with_generator_shift(const EvaluationDomain<Fr>& domain, const Fr& constant)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void ifft(const EvaluationDomain<Fr>& domain)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void ifft_with_constant(const EvaluationDomain<Fr>& domain, const Fr& constant)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    void coset_ifft(const EvaluationDomain<Fr>& domain)
        requires polynomial_arithmetic::SupportsFFT<Fr>;
    Fr compute_kate_opening_coefficients(const Fr& z)
        requires polynomial_arithmetic::SupportsFFT<Fr>;

    bool is_empty() const { return size_ == 0; }

    // For compatibility with polynomial.hpp
    // void set(size_t i, const Fr& value) { (*this)[i] = value; };
    /**
     * @brief Returns an std::span of the left-shift of self.
     *
     * @details If the n coefficients of self are (0, a₁, …, aₙ₋₁),
     * we returns the view of the n-1 coefficients (a₁, …, aₙ₋₁).
     */
    LegacyPolynomial shifted() const;

    /**
     * @brief Set self to the right shift of input coefficients
     * @details Set the size of self to match the input then set coefficients equal to right shift of input. Note: The
     * shifted result is constructed with its first shift-many coefficients equal to zero, so we assert that the last
     * shift-size many input coefficients are equal to zero to ensure that the relationship f(X) = f_{shift}(X)/X^m
     * holds. This is analagous to asserting the first coefficient is 0 in our left-shift-by-one method.
     *
     * @param coeffs_in
     * @param shift_size
     */
    void set_to_right_shifted(std::span<Fr> coeffs_in, size_t shift_size = 1);

    /**
     * @brief adds the polynomial q(X) 'other', multiplied by a scaling factor.
     *
     * @param other q(X)
     * @param scaling_factor scaling factor by which all coefficients of q(X) are multiplied
     */
    void add_scaled(std::span<const Fr> other, Fr scaling_factor);

    /**
     * @brief adds the polynomial q(X) 'other'.
     *
     * @param other q(X)
     */
    LegacyPolynomial& operator+=(std::span<const Fr> other);

    /**
     * @brief subtracts the polynomial q(X) 'other'.
     *
     * @param other q(X)
     */
    LegacyPolynomial& operator-=(std::span<const Fr> other);

    /**
     * @brief sets this = p(X) to s⋅p(X)
     *
     * @param scaling_factor s
     */
    LegacyPolynomial& operator*=(Fr scaling_factor);

    /**
     * @brief evaluates p(X) = ∑ᵢ aᵢ⋅Xⁱ considered as multi-linear extension p(X₀,…,Xₘ₋₁) = ∑ᵢ aᵢ⋅Lᵢ(X₀,…,Xₘ₋₁)
     * at u = (u₀,…,uₘ₋₁)
     *
     * @details this function allocates a temporary buffer of size n/2
     *
     * @param evaluation_points an MLE evaluation point u = (u₀,…,uₘ₋₁)
     * @param shift evaluates p'(X₀,…,Xₘ₋₁) = 1⋅L₀(X₀,…,Xₘ₋₁) + ∑ᵢ˲₁ aᵢ₋₁⋅Lᵢ(X₀,…,Xₘ₋₁) if true
     * @return Fr p(u₀,…,uₘ₋₁)
     */
    Fr evaluate_mle(std::span<const Fr> evaluation_points, bool shift = false) const;

    /**
     * @brief Partially evaluates in the last k variables a polynomial interpreted as a multilinear extension.
     *
     * @details Partially evaluates p(X) = (a_0, ..., a_{2^n-1}) considered as multilinear extension p(X_0,…,X_{n-1}) =
     * \sum_i a_i*L_i(X_0,…,X_{n-1}) at u = (u_0,…,u_{m-1}), m < n, in the last m variables X_n-m,…,X_{n-1}. The result
     * is a multilinear polynomial in n-m variables g(X_0,…,X_{n-m-1})) = p(X_0,…,X_{n-m-1},u_0,...u_{m-1}).
     *
     * @note Intuitively, partially evaluating in one variable collapses the hypercube in one dimension, halving the
     * number of coefficients needed to represent the result. To partially evaluate starting with the first variable (as
     * is done in evaluate_mle), the vector of coefficents is halved by combining adjacent rows in a pairwise
     * fashion (similar to what is done in Sumcheck via "edges"). To evaluate starting from the last variable, we
     * instead bisect the whole vector and combine the two halves. I.e. rather than coefficents being combined with
     * their immediate neighbor, they are combined with the coefficient that lives n/2 indices away.
     *
     * @param evaluation_points an MLE partial evaluation point u = (u_0,…,u_{m-1})
     * @return Polynomial<Fr> g(X_0,…,X_{n-m-1})) = p(X_0,…,X_{n-m-1},u_0,...u_{m-1})
     */
    LegacyPolynomial<Fr> partial_evaluate_mle(std::span<const Fr> evaluation_points) const;

    /**
     * @brief Divides p(X) by (X-r₁)⋯(X−rₘ) in-place.
     * Assumes that p(rⱼ)=0 for all j
     *
     * @details we specialize the method when only a single root is given.
     * if one of the roots is 0, then we first factor all other roots.
     * dividing by X requires only a left shift of all coefficient.
     *
     * @param roots list of roots (r₁,…,rₘ)
     */
    void factor_roots(std::span<const Fr> roots) { polynomial_arithmetic::factor_roots(std::span{ *this }, roots); };
    void factor_roots(const Fr& root) { polynomial_arithmetic::factor_roots(std::span{ *this }, root); };

    iterator begin() { return coefficients_; }
    iterator end() { return coefficients_ + size_; }
    pointer data() { return backing_memory_; }

    std::span<uint8_t> byte_span() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return { reinterpret_cast<uint8_t*>(coefficients_), size_ * sizeof(Fr) };
    }

    const_iterator begin() const { return coefficients_; }
    const_iterator end() const { return coefficients_ + size_; }
    const_pointer data() const { return backing_memory_; }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return size_ + MAXIMUM_COEFFICIENT_SHIFT; }

    static LegacyPolynomial random(const size_t num_coeffs)
    {
        LegacyPolynomial p(num_coeffs);
        std::generate_n(p.begin(), num_coeffs, []() { return Fr::random_element(); });
        return p;
    }

    /**
     * @brief Implicit conversion operator to convert Polynomial to PolynomialSpan.
     * NOTE: For LegacyPolynomial, unlike Polynomial, start index is always 0.
     * @return PolynomialSpan<Fr> A span covering the entire polynomial.
     */
    operator PolynomialSpan<Fr>() { return { 0, { coefficients_, coefficients_ + size() } }; }

    /**
     * @brief Implicit conversion operator to convert Polynomial to PolynomialSpan.
     * NOTE: For LegacyPolynomial, unlike Polynomial, start index is always 0.
     * @return PolynomialSpan<Fr> A span covering the entire polynomial.
     */
    operator PolynomialSpan<const Fr>() const { return { 0, { coefficients_, coefficients_ + size() } }; }

  private:
    // allocate a fresh memory pointer for backing memory
    // DOES NOT initialize memory
    void allocate_backing_memory(size_t n_elements);

    // safety check for in place operations
    bool in_place_operation_viable(size_t domain_size = 0) { return (size() >= domain_size); }

    void zero_memory_beyond(size_t start_position);
    // When a polynomial is instantiated from a size alone, the memory allocated corresponds to
    // input size + MAXIMUM_COEFFICIENT_SHIFT to support 'shifted' coefficients efficiently.
    const static size_t MAXIMUM_COEFFICIENT_SHIFT = 1;

    // The memory
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    std::shared_ptr<Fr[]> backing_memory_;
    // A pointer into backing_memory_ to support std::span-like functionality. This allows for coefficient subsets
    // and shifts.
    Fr* coefficients_ = nullptr;
    // The size_ effectively represents the 'usable' length of the coefficients array but may be less than the true
    // 'capacity' of the array. It is not explicitly tied to the degree and is not changed by any operations on the
    // polynomial.
    size_t size_ = 0;
};

template <typename Fr> inline std::ostream& operator<<(std::ostream& os, LegacyPolynomial<Fr> const& p)
{
    if (p.size() == 0) {
        return os << "[]";
    }
    if (p.size() == 1) {
        return os << "[ data " << p[0] << "]";
    }
    return os << "[ data\n"
              << "  " << p[0] << ",\n"
              << "  " << p[1] << ",\n"
              << "  ... ,\n"
              << "  " << p[p.size() - 2] << ",\n"
              << "  " << p[p.size() - 1] << ",\n"
              << "]";
}

using polynomial = LegacyPolynomial<bb::fr>;

} // namespace bb

/**
 * The static_assert below ensure that that our Polynomial class correctly models an `std::ranges::contiguous_range`,
 * and other requirements that allow us to convert a `Polynomial<Fr>` to a `std::span<const Fr>`.
 *
 * This also means we can now iterate over the elements in the vector using a `for(auto ...)` loop, and use various std
 * algorithms.
 *
 * static_assert(std::ranges::contiguous_range<bb::polynomial>);
 * static_assert(std::ranges::sized_range<bb::polynomial>);
 * static_assert(std::convertible_to<bb::polynomial, std::span<const bb::fr>>);
 * static_assert(std::convertible_to<bb::polynomial&, std::span<bb::fr>>);
 * // cannot convert a const polynomial to a non-const span
 * static_assert(!std::convertible_to<const bb::polynomial&, std::span<bb::fr>>);
 * static_assert(std::convertible_to<const bb::polynomial&, std::span<const bb::fr>>);
 */
