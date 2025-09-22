//! VIF instruction implementations.
#include <jmmt/ps2/vif.hpp>

#define TODO(inst) \
	std::printf("TODO: VIF " #inst "\n");

namespace jmmt::ps2 {

	// invalid instruction handler
	VIF_INSTRUCTION(Invalid) {
		std::printf("Warning jmmt::ps2::Vif: Invalid instruction %02x\n", instr.cmd);
		exit = true;
	}

	VIF_INSTRUCTION(nop) {
		return;
	}

	VIF_INSTRUCTION(stcycl) {
		TODO(stcycl);
	}

	VIF_INSTRUCTION(offset) {
		TODO(offset);
	}

	VIF_INSTRUCTION(base) {
		TODO(base)
	}

	VIF_INSTRUCTION(itop) {
		TODO(itop)
	}

	VIF_INSTRUCTION(stmod) {
		mode = instr.immediate & 0b00000011;
	}

	VIF_INSTRUCTION(mskpath3) {
		TODO(mskpath3)
	}

	VIF_INSTRUCTION(mark) {
		TODO(mark)
	}

	VIF_INSTRUCTION(flushe) {
		TODO(flushe)
	}

	VIF_INSTRUCTION(flush) {
		TODO(flush)
	}

	VIF_INSTRUCTION(flusha) {
		TODO(flusha)

	}

	VIF_INSTRUCTION(mscal) {
		TODO(mscal)
		exit = true;
	}

	VIF_INSTRUCTION(mscalf) {
		TODO(mscalf)
		exit = true;
	}

	VIF_INSTRUCTION(mscnt) {
		TODO(mscal)
		exit = true;
	}

	VIF_INSTRUCTION(stmask) {
		if(getBytesFromInput(&mask, sizeof(mask)) != sizeof(mask))
			exit = true;
	}

	VIF_INSTRUCTION(strow) {
		if(getBytesFromInput(&row[0], sizeof(row)) != sizeof(row))
			exit = true;
	}

	VIF_INSTRUCTION(stcol) {
		if(getBytesFromInput(&col[0], sizeof(col)) != sizeof(col))
			exit = true;
	}

	VIF_INSTRUCTION(mpg) {
		TODO(mpg)
		advanceInput(instr.num * 8);
	}

	VIF_INSTRUCTION(direct) {
		TODO(direct)
		advanceInput(instr.immediate * 8);
	}

	VIF_INSTRUCTION(directhl) {
		TODO(direct)
		advanceInput(instr.immediate * 8);
	}

} // namespace jmmt::ps2
