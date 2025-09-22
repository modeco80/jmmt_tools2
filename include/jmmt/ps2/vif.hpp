// This is a very simple VIF interpreter.
#pragma once
#include <cassert>
#include <mco/base_types.hpp>

namespace jmmt::ps2 {

	/// A VIFcode instruction. This strucutre is public so that consumers of this
	/// code can also create their own VIFcode instruction.
	struct [[gnu::packed]] VifCodeInstruction {
		enum CmdType : u8 {
			// general instructions
			nop = 0,
			stcycl = 1,
			offset = 2,
			base = 3,
			itop = 4,
			stmod = 5,
			mskpath3 = 6,
			mark = 7,

			// pipeline flushes
			flushe = 0x10,
			flush = 0x11,
			flusha = 0x13,

			// call vu microprogram
			mscal = 0x14,
			mscalf = 0x15,
			mscnt = 0x17,

			// mask/row/column registers
			stmask = 0x20,
			strow = 0x30,
			stcol = 0x31,

			// transfer memory into vu micromem
			mpg = 0x4a,

			// transfer direct to GIF via p2
			direct = 0x50,
			directhl = 0x51,

			// unpack (welcome to hell, hope you like it here)
			unpack = 0x60,
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
				u8 unused : 6;
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
	};

	mcoAssertSize(VifCodeInstruction, 4);

	/// Emulates the PS2's VIF, allowing for unpack of VIFtag data.
	class Vif {
		// registers

		u32 row[4] {};
		u32 col[4] {};

	   public:
		/// Reset the VIF state.
		void reset();

		/// Executes the VIF tags. This will unpack the data.
		void execute(const u8* pTags, u32 tagBufferLength, u8* pUnpackData, u32 unpackLength);
	};

	/// Disassembles VIF tag data into a more human-friendly text stream. Does not actually unpack the data.
	void vifDisassemble(const u8* pTagData, u32 tagBufferLength);

} // namespace jmmt::ps2
