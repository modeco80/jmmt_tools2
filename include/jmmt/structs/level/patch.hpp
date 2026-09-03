#pragma once
#include <mco/base_types.hpp>

namespace jmmt::structs::level {

	struct BitSet {
		u8 bytes[32];
	};

	struct aVifPatch {
		u32 vifTag0;
		i32 rx;
		i32 ry;
		i32 rz;
		i32 rw;
	};

	struct aPatchHeader {
		float scale;
		float reserved0; // Used in retail
		float reserved1;
		float reserved2;

		u32 patchCount;
		u32 patchOffset;

		u32 sectorCount;
		u32 sectorOffset;
	};

} // namespace jmmt::structs::level
