#include <jmmt/ps2/vif.hpp>

namespace jmmt::ps2 {

	enum offset {
		OffsetX = 0,
		OffsetY = 1,
		OffsetZ = 2,
		OffsetW = 3,
	};

	void doWrite(Vif& vif, bool mask, u32 offset, u32& dest, u32 data) {
		u32 n = 0;

		if(mask) {
			switch(vif.cycle.cl) {
				case 0:
					n = (vif.mask >> (offset * 2)) & 3;
					break;
				case 1:
					n = (vif.mask >> (8 + (offset * 2))) & 3;
					break;
				case 2:
					n = (vif.mask >> (16 + (offset * 2))) & 3;
					break;
				default:
					n = (vif.mask >> (24 + (offset * 2))) & 3;
					break;
			}
		}

		switch(n) {
			case 0:
				switch(vif.mode) {
					case 1:
						dest = data + vif.row[offset];
						break;
					case 2:
						vif.row[offset] = vif.row[offset] + data;
						dest = data;
					case 3:
						vif.row[offset] = data;
						dest = data;
						break;
					default:
						dest = data;
						break;
				}
				break;
			case 1:
				dest = vif.row[offset];
			case 2:
				dest = vif.col[vif.cycle.cl & 3];
				break;
			case 3:
				break;
		}
	}

	template <class T>
	void unpackS(Vif& vif, bool mask, u32* dest, const T* src) {
		u32 data = *src;
		doWrite(vif, mask, OffsetX, *(dest + 0), data);
		doWrite(vif, mask, OffsetY, *(dest + 1), data);
		doWrite(vif, mask, OffsetZ, *(dest + 2), data);
		doWrite(vif, mask, OffsetW, *(dest + 3), data);
	}

	template <class T>
	void unpackV2(Vif& vif, bool mask, u32* dest, const T* src) {
		doWrite(vif, mask, OffsetX, *(dest + 0), *(src + 0));
		doWrite(vif, mask, OffsetY, *(dest + 1), *(src + 1));
		doWrite(vif, mask, OffsetZ, *(dest + 2), *(src + 0));
		doWrite(vif, mask, OffsetW, *(dest + 3), *(src + 1));
	}

	template <class T>
	void unpackV4(Vif& vif, bool mask, u32* dest, const T* src) {
		doWrite(vif, mask, OffsetX, *(dest + 0), *(src + 0));
		doWrite(vif, mask, OffsetY, *(dest + 1), *(src + 1));
		doWrite(vif, mask, OffsetZ, *(dest + 2), *(src + 2));
		doWrite(vif, mask, OffsetW, *(dest + 3), *(src + 3));
	}

	template <class T>
	void unpackV45(Vif& vif, bool mask, u32* dest, const T* src) {
		u32 data = *src;
		doWrite(vif, mask, OffsetX, *(dest + 0), ((data & 0x001f) << 3));
		doWrite(vif, mask, OffsetY, *(dest + 1), ((data & 0x03e0) >> 2));
		doWrite(vif, mask, OffsetZ, *(dest + 2), ((data & 0x7c00) >> 7));
		doWrite(vif, mask, OffsetW, *(dest + 3), ((data & 0x8000) >> 8));
	}

	VIF_INSTRUCTION(unpack) {
		constexpr static const char* elementNameTable[] = {
			"s",
			"v2",
			"v3",
			"v4"
		};

		std::printf("reached an unpack\n");

		// Allocate memory in the output buffer which can hold our unpacked data.
		auto* pPacket = allocVifPacket(instr.getUnpackByteLength());
		void* pOutput = pPacket->data(); // allocOutputData(instr.getUnpackByteLength());

		pPacket->kind = VifPacket::Unpack;

		for(u8 i = 0; i < instr.num; ++i) {
			switch(instr.getUnpackBitLength()) {
				case 5:
					// We should only get here for unpack.v4 instructions.
					assert(instr.getUnpackElementType() == VifCodeInstruction::UnpackElementType::V4);
					pPacket->lanekind = VifPacket::V4;
					unpackV45(*this, instr.getUnpackWriteMask(), (u32*)pOutput + i, (const u32*)pInput);
					break;
#define DOSIZE(N)                                                                                    \
	case N:                                                                                          \
		switch(instr.getUnpackElementType()) {                                                       \
			case VifCodeInstruction::UnpackElementType::S:                                           \
				pPacket->lanekind = VifPacket::Single;                                               \
				unpackS(*this, instr.getUnpackWriteMask(), (u32*)pOutput + i, (const u##N*)pInput);  \
				break;                                                                               \
			case VifCodeInstruction::UnpackElementType::V2:                                          \
				pPacket->lanekind = VifPacket::V2;                                                   \
				unpackV2(*this, instr.getUnpackWriteMask(), (u32*)pOutput + i, (const u##N*)pInput); \
				break;                                                                               \
			case VifCodeInstruction::UnpackElementType::V3:                                          \
			case VifCodeInstruction::UnpackElementType::V4:                                          \
				pPacket->lanekind = VifPacket::V4;                                                   \
				unpackV4(*this, instr.getUnpackWriteMask(), (u32*)pOutput + i, (const u##N*)pInput); \
				break;                                                                               \
		}                                                                                            \
		break;
					DOSIZE(8)
					DOSIZE(16)
					DOSIZE(32)
			}

			advanceInput(instr.getUnpackElementByteLength());
		}

		// advanceInput(instr.getUnpackByteLength());
	}

} // namespace jmmt::ps2
