// This is a very simple VIF interpreter.
#pragma once
#include <array>
#include <cassert>
#include <mco/base_types.hpp>

namespace jmmt::ps2 {

#define VIF_INSTRUCTIONS() \
	X(nop, 0)              \
	X(stcycl, 1)           \
	X(offset, 2)           \
	X(base, 3)             \
	X(itop, 4)             \
	X(stmod, 5)            \
	X(mskpath3, 6)         \
	X(mark, 7)             \
	X(flushe, 0x10)        \
	X(flush, 0x11)         \
	X(flusha, 0x13)        \
	X(mscal, 0x14)         \
	X(mscalf, 0x15)        \
	X(mscnt, 0x17)         \
	X(stmask, 0x20)        \
	X(strow, 0x30)         \
	X(stcol, 0x31)         \
	X(mpg, 0x4a)           \
	X(direct, 0x50)        \
	X(directhl, 0x51)      \
	X(unpack, 0x60)

	/// A VIFcode instruction. This strucutre is public so that consumers of this
	/// code can also create their own VIFcode instruction.
	struct [[gnu::packed]] VifCodeInstruction {
		enum CmdType : u8 {
#define X(inst, code) inst = code,
			VIF_INSTRUCTIONS()
#undef X
		};

		enum class UnpackElementType {
			S,
			V2,
			V3,
			V4
		};

		constexpr static u32 LENGTH_LOOKUP_TABLE[4] = {
			32, 16, 8, 5
		};

		union {
			// this is the general structure. some instructions
			// may interpret them differently, they will have structs
			// if they do so

			struct [[gnu::packed]] {
				u16 immediate;
				u8 num;
			};

			struct [[gnu::packed]] {
				u16 newOfst : 9;
				u8 num;
			} instOffset;

			struct [[gnu::packed]] {
				u16 newItop : 9;
				u8 unused : 6;

				u8 num;
			} instItop;

			// used for any unpack.<> instruction
			struct [[gnu::packed]] {
				/* immediate means this */
				u16 addressDiv16 : 9;
				u16 unkn : 4;
				u8 zeroExtend : 1;
				u8 addTOPS : 1;

				u8 num;
			} instUnpack;
		};

		u8 cmd : 7;

		// we don't actually emulate this, but if this bit is set
		// when the tag is processed the vif will fire an interrupt
		u8 interrupt : 1;

		// helper functions

		u32 getDirectByteCount() const {
			assert(cmd == direct || cmd == directhl);
			u32 n = immediate;
			// ps2tek: If IMMEDIATE is 0, 65,536 quadwords are transferred.
			if(n == 0)
				n = 65536;

			return static_cast<u32>(n) * 16;
		}

		// UNPACK helpers

		UnpackElementType getUnpackElementType() const {
			assert((cmd >= 0x60 && cmd <= 0x6f) ||
				   (cmd >= 0x70 && cmd <= 0x7f));

			auto elementSizeRaw = (cmd >> 2) & 3;
			return static_cast<UnpackElementType>(elementSizeRaw);
		}

		u32 getUnpackLength() const {
			assert((cmd >= 0x60 && cmd <= 0x6f) ||
				   (cmd >= 0x70 && cmd <= 0x7f));

			return LENGTH_LOOKUP_TABLE[(cmd >> 0) & 3];
		}

		bool getUnpackWriteMask() const {
			assert((cmd >= 0x60 && cmd <= 0x6f) ||
				   (cmd >= 0x70 && cmd <= 0x7f));

			return cmd & (1 << 4);
		}

		u32 getUnpackByteLength() const {
			switch(getUnpackElementType()) {
				case UnpackElementType::S:
					return num * (getUnpackLength() / 8);
				case UnpackElementType::V2:
					return 2 * (num * (getUnpackLength() / 8));
				case VifCodeInstruction::UnpackElementType::V3:
					return 3 * (num * (getUnpackLength() / 8));
				case VifCodeInstruction::UnpackElementType::V4:
					if(getUnpackLength() == 5)
						return num * 2;
					else
						return 4 * (num * (getUnpackLength() / 8));
				default:
					break;
			}
			// should never happen
			assert(false);
			return -1;
		}
	};

	mcoAssertSize(VifCodeInstruction, 4);

#define VIF_INSTRUCTION(inst) void Vif::vifInst_##inst(VifCodeInstruction instr)

	/// Emulates the PS2's VIF, allowing for unpack of VIFtag data.
	class Vif {
		// registers
		u8 mode {};
		struct CycleRegister {
			u8 cl : 8;
			u16 wl : 9;
		};

		CycleRegister cycle;
		u32 ofst;
		u32 mask;
		u32 row[4] {};
		u32 col[4] {};

		// Interperter state
		bool exit = false;

		u8 const* pInput;
		u32 inputLength;
		u32 inputConsumed = 0;

		u8* pOutput;
		u32 outputLength;
		u32 outputConsumed = 0;

		u32 getBytesFromInput(void* pOut, u32 len);
		u32 advanceInput(u32 len);
		u32 writeBytesToOutput(const void* pWrite, u32 len);

		// Instructions
		void vifInst_Invalid(VifCodeInstruction);
#define X(inst, _code) void vifInst_##inst(VifCodeInstruction);
		VIF_INSTRUCTIONS()
#undef X

	   public:
		Vif() {
			reset();
		}

		/// See [vif_instructions.cpp] and [vif_instructions_unpack.cpp] for instruction implmentations.
		constexpr static auto makeInstructionTable() {
			std::array<void (Vif::*)(VifCodeInstruction), 0x7f> table {};
			// initalize all elements of the table with invalid instructions

			for(auto i = 0; i < table.size(); ++i)
				table[i] = &Vif::vifInst_Invalid;
#define X(inst, code) table[code] = &Vif::vifInst_##inst;
			VIF_INSTRUCTIONS()
#undef X

			// Unpack modes are handled by the instruction based on the instruction's
			// cmd value; therefore we have to fill all variations with the value
			for(auto i = 0x60; i < 0x6f; ++i) {
				table[i] = &Vif::vifInst_unpack;
				table[i + 0x10] = &Vif::vifInst_unpack; // w. writemask bit set
			}
			return table;
		}

		/// Reset the VIF state.
		void reset();

		/// Executes the VIFcode. This will unpack the data into [pUnpackData].
		void execute(const u8* pTags, u32 tagBufferLength, u8* pUnpackData, u32 unpackLength);
	};

	/// Disassembles VIF tag data into a more human-friendly text stream. Does not actually unpack the data.
	void vifDisassemble(const u8* pTagData, u32 tagBufferLength);

} // namespace jmmt::ps2
