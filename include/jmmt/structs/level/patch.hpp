#pragma once
#include <mco/base_types.hpp>

namespace jmmt::structs::level {

	struct BitSet {
		u8 bytes[32];
	};

	/// Patch structure in the file. Each patch is *actually* a
	/// VIFtag packed patch. Kind of evil.
	struct aVifPatch {
		u32 vifTag0; // strow
		i32 rx; // row literals
		i32 ry;
		i32 rz;
		i32 rw;
		u32 vifTag1; // stmod 01 (enable row-addition)
		u32 vifTag2; // unpack v3.16 ...
		u16 xyz0[12];
		u16 xyz1[12];
		u16 xyz2[12];
		u16 xyz3[12];
		u32 vifTag3; // stmod 00 (disable row-addition)
		u32 vifTag4; // unpack v3.16 ...
		u16 rgb0[12];
		u16 rgb1[12];
		u16 rgb2[12];
		u16 rgb3[12];
		u32 vifTag5; // unpack v2.16 ...
		u16 uv[10];
		u32 vifTag6; // mscal (does process)
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

	mcoAssertSize(aVifPatch, 0x100);

} // namespace jmmt::structs::level
