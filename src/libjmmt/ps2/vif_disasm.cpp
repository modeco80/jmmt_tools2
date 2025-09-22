#include <cassert>
#include <cstdio>
#include <cstring>
#include <jmmt/ps2/vif.hpp>

#include "memory_buffer_stream.hpp"

namespace jmmt::ps2 {

	void vifDisassemble(const u8* pVifCodeInstructions, unsigned length) {
		auto buffer = impl::MemoryBufferStream(pVifCodeInstructions, length);
		while(true) {
			// Read instruction
			VifCodeInstruction inst {};
			if(buffer.readSome(&inst, sizeof(inst)) != sizeof(inst))
				break;

			std::printf("%08x: ", (u32)buffer.tell());

			if(inst.interrupt) {
				std::printf("(I) ");
			}

			switch(inst.cmd) {
				case VifCodeInstruction::nop:
					std::printf("nop\n");
					break;

				case VifCodeInstruction::stcycl:
					std::printf("stcycl %d\n", inst.immediate);
					break;

				case VifCodeInstruction::offset:
					std::printf("offset %d\n", inst.instOffset.newOfst);
					break;

				case VifCodeInstruction::itop:
					std::printf("itop %d\n", inst.instItop.newItop);
					break;

				case VifCodeInstruction::stmod:
					std::printf("stmod %d\n", inst.immediate & 0b00000011);
					break;

				case VifCodeInstruction::mskpath3:
					std::printf("(vif1) mskpath3 %d", inst.immediate & (1 << 15));
					break;

				case VifCodeInstruction::mark:
					std::printf("mark %d\n", inst.immediate);
					break;

				// stalls
				case VifCodeInstruction::flushe:
					std::printf("flushe\n");
					break;

				case VifCodeInstruction::flush:
					std::printf("(vif1) flush\n");
					break;

				case VifCodeInstruction::flusha:
					std::printf("(vif1) flusha\n");
					break;

				// execute vu micro
				case VifCodeInstruction::mscal:
					std::printf("mscal %d\n", inst.immediate);
					break;

				case VifCodeInstruction::mscalf:
					std::printf("(vif1) mscalf %d\n", inst.immediate);
					break;

				case VifCodeInstruction::mscnt:
					std::printf("mscnt\n");
					break;

				// register sets
				case VifCodeInstruction::stmask: {
					u32 mask {};
					if(buffer.readSome(&mask, sizeof(mask)) != sizeof(mask))
						break;
					std::printf("stmask %d\n", mask);
				} break;

				case VifCodeInstruction::strow: {
					u32 data[4];
					if(buffer.readSome(&data, sizeof(data)) != sizeof(data))
						break;

					std::printf("strow 0x%08x,0x%08x,0x%08x,0x%08x\n",
								data[0], data[1], data[2], data[3]);
				} break;

				case VifCodeInstruction::stcol: {
					u32 data[4];
					if(buffer.readSome(&data, sizeof(data)) != sizeof(data))
						break;
					std::printf("stcol %d,%d,%d,%d\n",
								data[0], data[1], data[2], data[3]);
				} break;

				case VifCodeInstruction::mpg:
					std::printf("mpg (%08x), %u\n",
								static_cast<u32>(inst.immediate) * 8,
								static_cast<u32>(inst.num));
					buffer.advanceInput(static_cast<u32>(inst.num) * 8);
					break;

				case VifCodeInstruction::direct: {
					auto n = inst.getDirectByteCount();
					std::printf("(vif1) direct %d\n", n);
					buffer.advanceInput(n);
				} break;

				case VifCodeInstruction::directhl: {
					auto n = inst.getDirectByteCount();
					std::printf("(vif1) directhl %d\n", n);
					buffer.advanceInput(n);
				} break;

				case 0x60 ... 0x6f: // UNPACK
				case 0x70 ... 0x7f: {
					constexpr static const char* elementNameTable[] = {
						"s",
						"v2",
						"v3",
						"v4"
					};

					printf("unpack %s.%d", elementNameTable[static_cast<usize>(inst.getUnpackElementType())], inst.getUnpackLength());

					if(inst.getUnpackWriteMask()) {
						std::printf(" wmask");
					}

					std::printf("\n");

					buffer.advanceInput(inst.getUnpackByteLength());
				} break;

				default:
					std::printf("(!) unhandled VIF command %02x\n", inst.cmd);
					return;
			}
		}
	}

} // namespace jmmt::ps2
