#include <cstring>
#include <jmmt/lzss/decompress.hpp>

#define LZSS_DEFAULT_RINGSIZE 512
#define LZSS_DEFAULT_MATCHSIZE 66
#define LZSS_EOF -1
#define LZSS_CHUNKSIZE 256
#define LZSS_RINGBITS 9
#define LZSS_THRESHOLD 2
#define LZSS_STALLBIT (30)

#define LZSS_GETBYTE(in, inb, out)                                          \
	do {                                                                    \
		if((inb) <= (nInputBufferIndex))                                    \
			(out) = -1;                                                     \
		else {                                                              \
			/* std::printf("getting '%c'\n", *((in)+nInputBufferIndex)); */ \
			(out) = *((uint8_t*)((in) + nInputBufferIndex));                \
			nInputBufferIndex++;                                            \
		}                                                                   \
	} while(0)

// this version logs what it's going to put and where.
// #define LZSS_PUTBYTE(outp,outb) std::printf("LZSS_PUTBYTE(%llu, %x)\n", outp - oldptr, outb); \
*(outp)++ = (uint8_t)(outb)

#define LZSS_PUTBYTE(outp, outb) *(outp)++ = (uint8_t)(outb)

#define FileIO_ZeroMemory(dst, size) memset(dst, 0, size)

namespace jmmt::lzss {

	int lzssDecompress(structs::LzssHeader* header, const u8* compressedInput, i32 compressedLength, u8* destBuffer) {
		int32_t nRingIndex, nInSize, nInputBufferIndex, nRingBits, nRingSize;
		uint8_t aRingBuffer[LZSS_DEFAULT_RINGSIZE], *pRingBuffer;
		uint32_t nBitFlags = 0;

		std::int32_t nInByte;

		// auto* oldptr = destBuffer; // uncomment for logging version of LZSS_PUTBYTE

		// TODO: this is where we might want to place header usage. You know, if we need to.

		nBitFlags = 0;
		nInputBufferIndex = 0;
		nInSize = compressedLength;

		nRingSize = LZSS_DEFAULT_RINGSIZE;
		nRingBits = LZSS_RINGBITS;
		nRingIndex = LZSS_DEFAULT_RINGSIZE - LZSS_DEFAULT_MATCHSIZE;

		// Use stack allocated default ring buffer
		pRingBuffer = &aRingBuffer[0];
		FileIO_ZeroMemory(pRingBuffer, nRingSize);

		for(;;) {
			// get next 8 opcodes
			if(((nBitFlags >>= 1) & 256) == 0) {
				LZSS_GETBYTE(compressedInput, nInSize, nInByte);
				if(nInByte == -1)
					break;

				// std::printf("LZSS new opcodes\n");

				// store 255 in upper word, when zero get next 8 opcodes
				nBitFlags = nInByte | 0xff00;
			}

			// single char
			if(nBitFlags & 1) {
				LZSS_GETBYTE(compressedInput, nInSize, nInByte);
				if(nInByte == -1)
					break;

				// std::printf("LZSS single char '%c'\n", nInByte);

				LZSS_PUTBYTE(destBuffer, nInByte);
				pRingBuffer[nRingIndex++] = (uint8_t)nInByte;
				nRingIndex &= (nRingSize - 1);
			}

			// string
			else { // get position & length pair (note: 1 bit of position is stored in length word)
				int32_t i, j;
				LZSS_GETBYTE(compressedInput, nInSize, i);
				if(i == -1)
					break;
				LZSS_GETBYTE(compressedInput, nInSize, j);

				i |= ((j >> (16 - nRingBits)) << 8);
				j = (j & (0x00FF >> (nRingBits - 8))) + LZSS_THRESHOLD;

				// std::printf("LZSS string pos %d len %d\n", i , j);

				// LZSS_VALIDATE(j <= LZSS_DEFAULT_MATCHSIZE, "Invalid match size for decompression");

				for(int32_t k = 0; k <= j; ++k) {
					nInByte = pRingBuffer[(i + k) & (nRingSize - 1)];
					// std::printf("LZSS string byte '%c'\n", nInByte);
					LZSS_PUTBYTE(destBuffer, nInByte);
					pRingBuffer[nRingIndex++] = (uint8_t)nInByte;
					nRingIndex &= (nRingSize - 1);
				}
			}
		}

		return 0;
	}

} // namespace jmmt::lzss
