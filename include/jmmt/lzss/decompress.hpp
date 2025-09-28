#pragma once
#include <jmmt/structs/lzss.hpp>

namespace jmmt::lzss {

	int lzssDecompress(structs::LzssHeader* header, const u8* compressedInput, i32 compressedLength, u8* destBuffer);

}
