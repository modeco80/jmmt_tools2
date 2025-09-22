#pragma once

#include <mco/base_types.hpp>

namespace jmmt {

	/// Compile-time fixed string.
	template <usize N>
	struct FixedString {
		char buf[N + 1] {};

		constexpr FixedString() = default;

		constexpr FixedString(const char* pString) {
			for(usize i = 0; i < N; ++i)
				buf[i] = pString[i];
		}

		constexpr FixedString(const char* pString, usize length) {
			for(usize i = 0; i < N; ++i)
				buf[i] = pString[i];
		}

		constexpr FixedString(const char* pString, usize length, char padCharacter) {
			// copy string
			for(usize i = 0; i < length; ++i)
				buf[i] = pString[i];

			// pad string
			if(length < N) {
				for(usize i = length; i < N; ++i)
					buf[i] = padCharacter;
			}
		}

		[[nodiscard]] constexpr operator const char*() const {
			return buf;
		}

		[[nodiscard]] constexpr usize length() const {
			return N;
		}

		/// Truncate this string to a new length.
		template <usize newLength>
		consteval auto truncate() const {
			if(newLength < N) {
				// string is larger, truncate it
				return FixedString<newLength>(this->buf, N);
			} else {
				// string is smaller. than desired length, pad with zeroes.
				return FixedString<newLength>(this->buf, length());
			}
		}
	};

	template <usize N>
	FixedString(char const (&)[N]) -> FixedString<N - 1>;

} // namespace jmmt
