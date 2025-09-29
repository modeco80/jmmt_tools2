#pragma once
#include <mco/base_types.hpp>

namespace jmmt::fs {

	/// Public-facing package metadata
	struct PackageMetadata {
		/// The amount of files in the package.
		u32 nrPackageFiles;

		/// The offset where the pgrp/pfil chunks start.
		u32 chunksStartOffset;
	};

} // namespace jmmt::fs
