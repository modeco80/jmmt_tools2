#include <cassert>
#include <cstdio>
#include <cstring>
#include <jmmt/ps2/vif.hpp>

namespace jmmt::ps2 {

	void vifDisassemble(const u8* pVifCodeInstructions, unsigned length) {
		auto i = 0;
		while(i < length) {
			auto* pInst = reinterpret_cast<const VifCodeInstruction*>(&pVifCodeInstructions[i]);
			i += 4;

			if(pInst->interrupt) {
				std::printf("(I) ");
			}

			switch(pInst->cmd) {
				case VifCodeInstruction::nop:
					std::printf("nop\n");
					break;

				case VifCodeInstruction::stcycl:
					std::printf("stcycl %d\n", pInst->immediate);
					break;

				case VifCodeInstruction::offset:
					std::printf("offset %d\n", pInst->instOffset.newOfst);
					break;

				case VifCodeInstruction::itop:
					std::printf("itop %d\n", pInst->instItop.newItop);
					break;

				case VifCodeInstruction::stmod:
					std::printf("stmod %d\n", pInst->immediate & 0b00000011);
					break;

				case VifCodeInstruction::mskpath3:
					std::printf("(vif1) mskpath3 %d", pInst->immediate & (1 << 15));
					break;

				case VifCodeInstruction::mark:
					std::printf("mark %d\n", pInst->immediate);
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
					std::printf("mscal %d\n", pInst->immediate);
					break;

				case VifCodeInstruction::mscalf:
					std::printf("(vif1) mscalf %d\n", pInst->immediate);
					break;

				case VifCodeInstruction::mscnt:
					std::printf("mscnt\n");
					break;

				// register sets
				case VifCodeInstruction::stmask:
					std::printf("stmask %d\n", *(u32*)&pVifCodeInstructions[i]);
					i += 4;
					break;

				case VifCodeInstruction::strow: {
					u32 data[4];
					memcpy(&data[0], &pVifCodeInstructions[i], 4 * sizeof(u32));
					std::printf("strow 0x%08x,0x%08x,0x%08x,0x%08x\n",
								data[0], data[1], data[2], data[3]);
					i += 4 * sizeof(u32);
				} break;

				case VifCodeInstruction::stcol: {
					u32 data[4];
					memcpy(&data[0], &pVifCodeInstructions[i], 4 * sizeof(u32));
					std::printf("stcol %d,%d,%d,%d\n",
								data[0], data[1], data[2], data[3]);
					i += 4 * sizeof(u32);
				} break;

				case VifCodeInstruction::mpg:
					std::printf("mpg (%08x), %u\n",
								static_cast<u32>(pInst->immediate) * 8,
								static_cast<u32>(pInst->num));
					i += static_cast<u32>(pInst->num) * 8;
					break;

				case VifCodeInstruction::direct: {
					auto n = pInst->getDirectByteCount();
					std::printf("(vif1) direct %d\n", n);
					i += n;
				} break;

				case VifCodeInstruction::directhl: {
					auto n = pInst->getDirectByteCount();
					std::printf("(vif1) directhl %d\n", n);
					i += n;
				} break;

				case 0x60 ... 0x6f: // UNPACK
				case 0x70 ... 0x7f: {
					constexpr static const char* elementNameTable[] = {
						"s",
						"v2",
						"v3",
						"v4"
					};

					printf("unpack %s.%d", elementNameTable[static_cast<usize>(pInst->getUnpackElementType())], pInst->getUnpackLength());

					if(pInst->getUnpackWriteMask()) {
						std::printf(" wmask");
					}

					std::printf("\n");

					i += pInst->getUnpackByteLength();
				} break;

				default:
					std::printf("(!) unhandled VIF command %02x\n", pInst->cmd);
					return;
			}
		}
	}

} // namespace jmmt::ps2
