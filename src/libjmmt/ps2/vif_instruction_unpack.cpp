#include <cstring>
#include <jmmt/ps2/vif.hpp>

namespace jmmt::ps2 {

	VIF_INSTRUCTION(unpack) {
		const auto elementType = instr.getUnpackElementType();
		const auto elementRawBitLength = static_cast<u8>(instr.getUnpackRawLength());
		const int numComponents = static_cast<int>(elementType) + 1;

		int bitsPerComponent = 32;
		switch(elementRawBitLength) {
			case 0:
				bitsPerComponent = 32;
				break;
			case 1:
				bitsPerComponent = 16;
				break;
			case 2:
				bitsPerComponent = 8;
				break;
			case 3:
				bitsPerComponent = (elementType == VifCodeInstruction::UnpackElementType::V4) ? 4 : 16;
				break;
			default:
				break;
		}

		const bool isV45 = elementRawBitLength == 3 && elementType == VifCodeInstruction::UnpackElementType::V4;
		const auto bitsPerVector = isV45 ? 16 : (numComponents * bitsPerComponent);
		const auto bytesPerVector = static_cast<u32>((bitsPerVector + 7) / 8);
		const auto writeVectorCount = (instr.instUnpack.num == 0u) ? 256 : static_cast<u32>(instr.instUnpack.num);

		u32 cl = this->cycle.cl;
		u32 wl = this->cycle.wl & 0xff;

		if(cl == 0u)
			cl = 1u;
		if(wl == 0u)
			wl = 1u;

		const bool filling = cl < wl;

		u32 sourceVectorCount = writeVectorCount;

		if(filling) {
			const u32 fullBlocks = writeVectorCount / wl;
			u32 remainder = writeVectorCount % wl;
			if(remainder > cl)
				remainder = cl;
			sourceVectorCount = fullBlocks * cl + remainder;
		}

		const auto totalBytes = ((sourceVectorCount * bytesPerVector) + 3u) & ~3u;
		const auto vuMemoryAddress = static_cast<u32>(instr.instUnpack.addressDiv16 & 0x3FFu);


		auto decompressScalar = [&](const u8* pSource, u32 component) -> u32 {
			switch(bitsPerComponent) {
				case 32: {
					u32 value;
					std::memcpy(&value, pSource + component * sizeof(u32), sizeof(value));
					return value;
				};

				case 16: {
					u16 value;
					std::memcpy(&value, pSource + component * sizeof(u16), sizeof(value));
					if(instr.instUnpack.zeroExtend)
						return static_cast<u32>(value);

					return static_cast<u32>(static_cast<i32>(static_cast<i16>(value)));
				};

				case 8: {
					const u8 value = pSource[component];
					if(instr.instUnpack.zeroExtend)
						return static_cast<u32>(value);

					return static_cast<u32>(static_cast<i32>(static_cast<i8>(value)));
				};

				default:
					assert(false && "unhandled decompress");
					return -1;
			}
		};

		auto applyVifMode = [&](u32 value, u32 component) -> u32 {
			switch(mode & 3u) {
				case 0:
					return value;

				case 1:
					return value + row[component];

				case 2:
					row[component] += value;
					return row[component];

				case 3:
					row[component] = value;
					return row[component];

				default:
					return value;
			}
		};

		auto decodeVector = [&](const u8* pSource, std::array<u32, 4>& out) {
			if(isV45) {
				u16 packed;
				std::memcpy(&packed, pSource, sizeof(packed));

				// Expand the packed V4.5 data.
				// Note that V4.5 doesn't get mode applied to it, so row registers
				// are not updated or used here.
				out[0] = (static_cast<u32>(packed) & 0x001Fu) << 3;
				out[1] = (static_cast<u32>(packed) & 0x03E0u) >> 2;
				out[2] = (static_cast<u32>(packed) & 0x7C00u) >> 7;
				out[3] = (static_cast<u32>(packed) & 0x8000u) >> 8;
				return;
			}

			switch(elementType) {
				case VifCodeInstruction::UnpackElementType::S: {
					// The scalar is broadcasted across XYZW lanes.
					const u32 value = decompressScalar(pSource, 0);
					for(u32 component = 0; component < 4; ++component)
						out[component] = applyVifMode(value, component);
				} break;

				case VifCodeInstruction::UnpackElementType::V2: {
					const u32 x = decompressScalar(pSource, 0);
					const u32 y = decompressScalar(pSource, 1);

					// XYZW lanes become XYXY with V2.<n>.
					out[0] = applyVifMode(x, 0);
					out[1] = applyVifMode(y, 1);
					out[2] = applyVifMode(x, 2);
					out[3] = applyVifMode(y, 3);
				} break;

				case VifCodeInstruction::UnpackElementType::V3: {
					const u32 x = decompressScalar(pSource, 0);
					const u32 y = decompressScalar(pSource, 1);
					const u32 z = decompressScalar(pSource, 2);

					out[0] = applyVifMode(x, 0);
					out[1] = applyVifMode(y, 1);
					out[2] = applyVifMode(z, 2);

					// V3 has no fourth source component. The W destination
					// component is left unchanged.
					out[3] = 0;
				} break;

				case VifCodeInstruction::UnpackElementType::V4: {
					for(u32 component = 0; component < 4; ++component)
						out[component] = applyVifMode(decompressScalar(pSource, component), component);
				} break;

				default:
					break;
			}
		};

		const u8* pVectorInputBase = &pInput[inputConsumed];
		u32 inputIndex = 0;

		for(u32 i = 0; i < writeVectorCount; ++i) {
			const u32 cyclePos = i % wl;
			const bool writeThisVector = filling ? (cyclePos < cl) : true;
			const bool consumeInput = filling ? (cyclePos < cl) : true;
			const u32 cycleWidth = filling ? cl : wl;
			const u32 destVec = (vuMemoryAddress + ((i / wl) * cycleWidth) + cyclePos) & 0x3FF;
			const u32 destQwordOffset = destVec * 16;

			// If this slot is skipped/filler, don't consume source.
			if(!writeThisVector)
				continue;

			// No more data.
			if(inputIndex >= sourceVectorCount)
				break;

			const u8* pVectorSource = pVectorInputBase + inputIndex * bytesPerVector;
			++inputIndex;

			std::array<u32, 4> vectorLanes{};
			decodeVector(pVectorSource, vectorLanes);

			// Write the decompressed vector into the simulated micromem.
			if(destQwordOffset + 16 <= VU0_MEMORY_SIZE)
				std::memcpy(&pOutput[destQwordOffset], &vectorLanes[0], sizeof(vectorLanes));
		}

		advanceInput((pVectorInputBase + inputIndex * bytesPerVector) - pVectorInputBase);
	}

} // namespace jmmt::ps2
