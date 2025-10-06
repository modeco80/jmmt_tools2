#pragma once
#include <mco/base_types.hpp>

namespace jmmt::structs {

	struct LzssHeader {
		u32 next; // This is actually a pointer.
		u8 cByteId;
		u8 cHdrSize = sizeof(LzssHeader); // should be sizeof(LzssHeader)
		u8 nMaxMatch;
		u8 nFillByte;
		u16 nRingSize;
		u16 nErrorId;
		u32 nUnCompressedBytes;
		u32 nCompressedBytes;
		u32 nCRC;
		u32 nFileId;
		u32 nCompressedDataCRC;
	};

	mcoAssertSize(LzssHeader, 0x20);
} // namespace jmmt::structs
