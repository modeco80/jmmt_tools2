#pragma once
#include <jmmt/structs/lzss.hpp>

namespace jmmt::lzss {

	/// Decompress 3DO LZSS input.
	int decompress(structs::LzssHeader* header, const u8* compressedInput, i32 compressedLength, u8* destBuffer);

}
