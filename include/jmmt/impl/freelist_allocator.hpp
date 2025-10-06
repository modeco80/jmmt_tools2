#pragma once
#include <bitset>
#include <cstring>
#include <mco/base_types.hpp>

namespace jmmt::impl {

	/// A very simple freelist allocator. Holds a bucket of memory
	/// and allows objects of a given type to be allocated and retrieved from it.
	template <class T, u32 MaxSize>
	class FreeListAllocator {
		std::bitset<MaxSize> freeBitSet;
		T* objectMemory;

	   public:
		/// A handle to an object in the freelist allocator.
		using Handle = i32;

		/// An invalid handle.
		constexpr static Handle InvalidHandle = -1;

		FreeListAllocator() {
			objectMemory = reinterpret_cast<T*>(malloc(MaxSize * sizeof(T)));
			std::memset(reinterpret_cast<void*>(&objectMemory[0]), 0, MaxSize * sizeof(T));
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
			Handle handlesToClear[MaxSize];
			u32 nHandles = 0;

			// Find all allocated objects.
			for(usize i = 0; i < freeBitSet.size(); ++i) {
				if(freeBitSet[i]) {
					handlesToClear[nHandles++] = i;
				}
			}

			// Clear them.
			for(u32 i = 0; i < nHandles; ++i)
				freeObject(handlesToClear[i]);
		}

		template <class... Args>
		Handle allocateObject(Args&&... args) {
			// Find any free position.
			for(usize i = 0; i < freeBitSet.size(); ++i) {
				// Construct the object in the memory, and then return the handle.
				if(!freeBitSet[i]) {
					freeBitSet[i] = true;
					new(&objectMemory[i]) T(static_cast<Args&&>(args)...);
					return i;
				}
			}

			// There are no free slots, so give up and return an invalid handle.
			return InvalidHandle;
		}

		/// Dereference an handle, obtaining a concrete pointer to an object.
		T* dereferenceHandle(Handle handle) {
			if(handle == InvalidHandle && handle > MaxSize)
				return nullptr;

			// If this object is actually allocated, then return a pointer to its memory
			// (i.e: actually deref the handle). Otherwise, return a null pointer, to indicate failure.
			if(freeBitSet[handle]) {
				return &objectMemory[handle];
			}

			return nullptr;
		}

		/// Free a object pointed to by [handle].
		void freeObject(Handle handle) {
			if(handle == InvalidHandle && handle > MaxSize)
				return;

			// If the handle is actually dereferenceable to an allocated object..
			if(freeBitSet[handle]) {
				// Free the object. Another allocateObject() call
				// can provide the same handle.
				freeBitSet[handle] = false;
				objectMemory[handle].~T();

				// Zero the memory once the object is freed.
				std::memset(reinterpret_cast<void*>(&objectMemory[handle]), 0, sizeof(T));
			}
		}
	};

} // namespace jmmt::impl
