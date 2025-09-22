#include <jmmt/ps2/vif.hpp>

namespace jmmt::ps2 {

	VIF_INSTRUCTION(unpack) {
		constexpr static const char* elementNameTable[] = {
			"s",
			"v2",
			"v3",
			"v4"
		};

		printf("TODO: unpack %s.%d\n", elementNameTable[static_cast<usize>(instr.getUnpackElementType())], instr.getUnpackLength());

		advanceInput(instr.getUnpackByteLength());
	}

} // namespace jmmt::ps2
