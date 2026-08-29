#pragma once
#include <mco/fourcc.hpp>

namespace jmmt::structs {
	struct PackageGroupChunk {
		constexpr static auto CHUNK_ID = mco::FourCCGenerator<>::generate<"PGRP">();
		mco::FourCC ckId;

		/// Hash name of the name of this group.
		u32 indexName;

		/// The amount of entries in this group.
		u32 nEntries;

		/// Flags.
		u32 flagsMask;
	};

	mcoAssertSize(PackageGroupChunk, 0x10);
} // namespace jmmt::structs
