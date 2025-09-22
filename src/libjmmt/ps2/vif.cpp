#include <array>
#include <cstring>
#include <jmmt/ps2/vif.hpp>

namespace jmmt::ps2 {

	// this is techincally write-only-C++ but i write-only-don't give a fuck
	// we have constexpr for a reason, so we don't need to write mindless codegens

	/// See [vif_instructions.cpp] and [vif_instructions_unpack.cpp] for instruction implmentations.
	constexpr static auto instructionTable = []() {
		std::array<void (Vif::*)(VifCodeInstruction), 0x7f> table {};
		// initalize all elements of the table with invalid instructions

		for(auto i = 0; i < table.size(); ++i)
			table[i] = &Vif::vifInstInvalid;
#define X(inst, code) table[code] = &Vif::vifInst##inst;
		VIF_INSTRUCTIONS()
#undef X

		// Unpack modes are handled by the instruction based on the instruction's
		// cmd value; therefore we have to fill all variations with the value
		for(auto i = 0x60; i < 0x6f; ++i) {
			table[i] = &Vif::vifInstunpack;
			table[i + 0x10] = &Vif::vifInstunpack; // w. writemask bit set
		}
		return table;
	}();

	u32 Vif::advanceInput(u32 len) {
		if((inputConsumed + len) > inputLength) {
			len = inputLength - inputConsumed;
		}

		inputConsumed += len;
		return len;
	}

	u32 Vif::getBytesFromInput(void* pOut, u32 len) {
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

	u32 Vif::writeBytesToOutput(const void* pWrite, u32 len) {
		if((outputConsumed + len) > outputLength) {
			len = outputLength - outputConsumed;
		}

		if(len == 0)
			return 0;

		memcpy(&pOutput[outputConsumed], pWrite, len);
		return len;
	}

	void Vif::reset() {
		memset(this, 0, sizeof(*this));
	}

	void Vif::execute(const u8* pTags, u32 tagBufferLength, u8* pUnpackData, u32 unpackLength) {
		this->pInput = pTags;
		this->inputLength = tagBufferLength;
		this->pOutput = pUnpackData;
		this->outputLength = unpackLength;

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

			(this->*instructionTable[inst.cmd])(inst);
		}

		this->pInput = nullptr;
		this->inputLength = 0;
		this->pOutput = nullptr;
		this->outputLength = 0;
	}

} // namespace jmmt::ps2
