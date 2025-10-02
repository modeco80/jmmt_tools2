#pragma once
#include <mco/base_types.hpp>
#include <bitset>
#include <cstring>

namespace jmmt::impl {

	/// A very simple freelist allocator.
	template<class T, usize MaxSize>
	class FreeListAllocator {
		std::bitset<MaxSize> freeBitSet;
		T* objectMemory;
	public:
		/// A handle to an object in the freelist allocator.
		using Handle = i32;

		/// A invalid handle.
		constexpr static Handle InvalidHandle = -1;

		FreeListAllocator() {
			objectMemory = reinterpret_cast<T*>(malloc(MaxSize * sizeof(T)));
		}

		// Freelist allocators cannot be relocated. However, the only consumer of this code
		// won't ever do that anyways.
		FreeListAllocator(const FreeListAllocator&) = delete;
		FreeListAllocator(FreeListAllocator&&) = delete;

		~FreeListAllocator() {
			clear();
			free(objectMemory);
		}

		void clear() {
			// For all bits in freeSet which are set (indicating the object slot is in use):
			// - Call T::~T().
			for (usize i = 0; i < freeBitSet.size(); ++i) {
				if (freeBitSet[i]) {
					freeBitSet[i] = false;
					objectMemory[i].~T();
				}
			}
		}

		template<class ...Args>
		Handle allocateObject(Args&&... args) {
			// Find any free position.
			for (usize i = 0; i < freeBitSet.size(); ++i) {
				if (!freeBitSet[i]) {
					freeBitSet[i] = true;
					// Construct the object in the memory, and then return the handle.
					std::memset(&objectMemory[i], 0, sizeof(T));
					new (&objectMemory[i]) T(static_cast<Args&&>(args)...);
					return i;
				}
			}

			// No free slots.
			return InvalidHandle;
		}

		/// Dereference an handle, obtaining a pointer to an object
		T* dereferenceHandle(Handle handle) {
			if(handle == InvalidHandle)
				return nullptr;

			// If this object is actually allocated, then return a pointer to its memory
			// (i.e: actually deref the handle). Otherwise, return a null pointer, to indicate failure.
			if(freeBitSet[handle]) {
				return &objectMemory[handle];
			}

			return nullptr;
		}

		/// Free a object pointed to by handle.
		void freeObject(Handle handle) {
			if(handle == InvalidHandle)
				return;

			// If the handle is actually dereferenceable to an allocated object..
			if(freeBitSet[handle]) {
				// Free the object. Another allocateObject() call
				// can provide the same handle.
				freeBitSet[handle] = false;
				objectMemory[handle].~T();
			}
		}
	};


}
