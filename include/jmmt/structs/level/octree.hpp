#pragma once
#include <mco/base_types.hpp>

namespace jmmt::structs::level {

	struct aOctree {
		float minX;
		float minY;
		float minZ;

		i16 patch; // First patch
		i16 count; // Other patches (so patches[patch..patch+count])

		float maxX;
		float maxY;
		float maxZ;

		i16 sibling; // Next
		i16 child;
	};

	mcoAssertSize(aOctree, 0x20);
}
