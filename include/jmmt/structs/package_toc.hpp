#pragma once

#include <base/types.hpp>

namespace jmmt::structs {

	struct PackageTocHeader {
		char fileName[0x40];
		u32 filenameHash;
		u32 tocStartOffset;
		u32 tocSize;
		u32 tocFileCount;
	};

} // namespace jmmt::structs
