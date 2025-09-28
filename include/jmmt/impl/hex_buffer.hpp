#pragma once
#include <mco/base_types.hpp>
#include <array>
#include <jmmt/fixed_string.hpp>

namespace jmmt::impl {
	/// Converts a hex string into a fixed buffer at compile time.
	template<FixedString inputHexString>
	consteval auto hexToBuffer() {
		if(auto evenTest = inputHexString.length() % 2; evenTest != 0) {
			throw "invalid input";
		}

		std::array<u8, inputHexString.length()/2> buf;

		auto parseHexChar = [](char c) {
			if (c >= '0' && c <= '9') {
				return c - '0';
			} else if (c >= 'A' && c <= 'F') {
				return c - 'A' + 10;
			} else if (c >= 'a' && c <= 'f') {
				return c - 'a' + 10;
			}
			throw "its over";
		};

		for(auto i = 0; i < inputHexString.length(); i += 2) {
			u8 upperNibble = parseHexChar(inputHexString[i]);
			u8 lowerNibble = parseHexChar(inputHexString[i+1]);
			buf[i/2] = (upperNibble << 4) | lowerNibble;
		}

		return buf;
	}

	static_assert(hexToBuffer<"fefa">()[0] == 0xfe, "its done");
	static_assert(hexToBuffer<"fefa">()[1] == 0xfa, "its done");
}
