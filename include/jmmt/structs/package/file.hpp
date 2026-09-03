#pragma once
#include <jmmt/structs/lzss.hpp>
#include <mco/fourcc.hpp>

namespace jmmt::structs {

	struct PackageFileChunk {
		constexpr static auto CHUNK_ID = mco::FourCCGenerator<>::generate<"PFIL">();

		mco::FourCC ckId;

		// This is a seperate struct in the official game.
		u8 versionMajor;
		u8 versionMinor;
		i16 versionBuild;

		u32 dayCreated;

		i16 chunkNumber;
		i16 chunkCount;

		u32 indexName;
		u32 indexSourceName;
		u32 indexSourceConvertName;
		u32 indexSourceCompressName;

		// the type of this;
		u32 indexType;

		u32 padBytes;

		u32 flags;

		u32 data; // PS2 pointer, we don't need to care about this

		u32 chunkSize;
		u32 chunkOffset;

		u32 dataSize;
		u32 dataOffset;

		u32 totalFileSize;
		LzssHeader lzssHeader; // Unused by the game
	};

	mcoAssertSize(PackageFileChunk, 0x64);
} // namespace jmmt::structs
