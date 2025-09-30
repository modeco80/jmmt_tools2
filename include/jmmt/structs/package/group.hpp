#pragma once
#include <jmmt/fourcc.hpp>

namespace jmmt::structs {
	struct PackageGroupHeader {
		constexpr static auto MAGIC = FourCCGenerator<>::generate<"PGRP">();
		FourCC magic;

		/// Hash name of the name of this group.
		u32 indexName;

		/// The amount of entries in this group.
		u32 nEntries;

		/// Flags.
		u32 flagsMask;
	};

	mcoAssertSize(PackageGroupHeader, 0x10);
}
