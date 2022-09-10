#pragma once

#include <cstddef>
#include <array>
#include <type_traits>

namespace usdk {

template <typename T, size_t N>
struct array {
    T array[N];

    using type = T;
    static constexpr size_t length = N;

    constexpr size_t size() const { return N; }
    constexpr T& operator[](size_t i) { return array[i]; }
    constexpr const T& operator[](size_t i) const { return array[i]; }
    constexpr T * begin() { return &array[0]; }
    constexpr const T * begin() const { return &array[0]; }
    constexpr T * end() { return &array[N]; }
    constexpr const T * end() const { return &array[N]; }

    template <typename... Us>
    constexpr auto append(Us... us);

    template <typename... Us>
    constexpr auto concat(Us... us);
};

template <typename InputIt, typename OutputIt>
constexpr OutputIt copy(InputIt in, InputIt in_end, OutputIt out) {
    while (in != in_end) {
        *out = *in;
        ++in;
        ++out;
    }

    return out;
};

template <typename InputIt, typename Size, typename OutputIt>
constexpr OutputIt copy_n(InputIt in, Size size, OutputIt out) {
    for (Size i = 0; i < size; i++) {
        *out = *in;
        ++in;
        ++out;
    }

    return out;
};

namespace detail {
    template <typename T, size_t N>
    T array_type_helper(T(&)[N]);
    template <typename T, size_t N>
    T array_type_helper(const array<T, N>&);
    template <typename T, size_t N>
    T array_type_helper(const std::array<T, N>&);
}

template <typename T>
using array_type = decltype(detail::array_type_helper(std::declval<T>()));

template <typename T, size_t N>
constexpr size_t array_length(T(&)[N]) {
    return N;
}

template <typename T, size_t N>
constexpr size_t array_length(const array<T, N>&) {
    return N;
}

template <typename T, size_t N>
constexpr size_t array_length(const std::array<T, N>&) {
    return N;
}

template <typename T, size_t N>
constexpr auto to_array(const T(&in)[N]) {
    array<T, N> out = {};
    copy_n(std::begin(in), N, std::begin(out));
    return out;
}

template <typename T, size_t N>
constexpr auto to_array(const std::array<T, N>& in) {
    array<T, N> out = {};
    copy_n(std::begin(in), N, std::begin(out));
    return out;
}

template <typename U, typename T, size_t N>
constexpr auto array_convert(array<T, N> in) {
    array<U, N> out = {};
    copy_n(std::begin(in), N, std::begin(out));
    return out;
}

template <size_t From, size_t To, typename T, size_t N>
constexpr auto array_span(array<T, N> in) {
    array<T, To - From> out = {};
    copy_n(std::begin(in) + From, To - From, std::begin(out));
    return out;
}

namespace detail {
    template <typename T, typename... Ts>
    T first_helper(T, Ts...);
    template <typename... Ts>
    using first = decltype(first_helper(std::declval<Ts>()...));
}

template <typename... Array>
constexpr auto array_concat(Array... arrays) {
    using T = detail::first<array_type<Array>...>;
    constexpr size_t N = (array_length(arrays) + ...);
    array<T, N> out = {};

    auto it = std::begin(out);
    ((it = copy_n(std::begin(arrays), array_length(arrays), it)), ...);

    return out;
}

template <typename T, size_t N>
template <typename... Us>
constexpr auto array<T, N>::concat(Us... us) {
    return array_concat(*this, us...);
}

template <typename Array, typename... Ts>
constexpr auto array_append(Array array, Ts... ts) {
    auto append_data = to_array({ ts... });
    return array_concat(array, append_data);
}

template <typename T, size_t N>
template <typename... Us>
constexpr auto array<T, N>::append(Us... us) {
    return array_append(*this, us...);
}

}
