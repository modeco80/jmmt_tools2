#pragma once
#include <mco/base_types.hpp>

namespace jmmt {

	using Crc32Result = u32;

	/// Hash a string.
	Crc32Result HashString(std::string_view str);

	/// Hash a string (case-sensitive version).
	Crc32Result HashStringCase(std::string_view str);

}
