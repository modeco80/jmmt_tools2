#pragma once
#include <cstring>
#include <mco/base_types.hpp>

namespace jmmt::ps2::impl {

	class MemoryBufferStream {
	   public:
		MemoryBufferStream(u8 const* pBuffer, usize length) : pBuffer(pBuffer), length(length), consumed(0) {
		}

		void reset() {
			consumed = 0;
		}

		usize advanceInput(usize amount) {
			if((consumed + amount) > length) {
				amount = length - consumed;
			}

			consumed += amount;
			return amount;
		}

		usize readSome(void* pPtr, usize size) {
			if((consumed + size) > length) {
				size = length - consumed;
			}

			// don't copy anything
			if(size == 0)
				return 0;

			memcpy(pPtr, &pBuffer[consumed], size);
			advanceInput(size);
			return size;
		}

		usize tell() const {
			return consumed;
		}

	   private:
		u8 const* pBuffer;
		usize length;
		usize consumed = 0;
	};
} // namespace jmmt::ps2::impl
