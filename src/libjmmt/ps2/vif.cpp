#include <array>
#include <cstring>
#include <jmmt/ps2/vif.hpp>

namespace jmmt::ps2 {

	constexpr static auto gInstructionTable = VifEmulator::makeInstructionTable();

	u32 VifEmulator::advanceInput(u32 len) {
		if((inputConsumed + len) > inputLength) {
			len = inputLength - inputConsumed;
		}

		inputConsumed += len;
		return len;
	}

	u32 VifEmulator::getBytesFromInput(void* pOut, u32 len) {
		if((inputConsumed + len) > inputLength) {
			len = inputLength - inputConsumed;
		}

		// don't copy anything
		if(len == 0)
			return 0;

		memcpy(pOut, &pInput[inputConsumed], len);
		advanceInput(len);
		return len;
	}

	void VifEmulator::reset() {
		memset(this, 0, sizeof(*this));
	}

	void VifEmulator::execute(const u8* pTags, u32 tagBufferLength, u8* pVUMemory, u32 vuMemorySize) {
		this->pInput = pTags;
		this->inputLength = tagBufferLength;
		this->pOutput = pVUMemory;
		this->outputLength = vuMemorySize;

		assert(vuMemorySize >= VU0_MEMORY_SIZE && "Invalid VU0 memory buffer");

		while(true) {
			// Early-exit can be triggered by invalid conditions.
			if(exit == true) {
				exit = false;
				break;
			}

			// Try to read a VIFcode instruction fron the input stream.
			// If this fails, give up and stop interpreting data.
			VifCodeInstruction inst {};
			if(auto n = getBytesFromInput(&inst, sizeof(inst)); n != sizeof(inst))
				break;

			// Execute the instruction.
			(this->*gInstructionTable[inst.cmd])(inst);
		}

		this->pInput = nullptr;
		this->inputLength = 0;
		this->pOutput = nullptr;
		this->outputLength = 0;
	}

} // namespace jmmt::ps2
