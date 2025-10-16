#pragma once
#include <bitset>
#include <cstring>
#include <mco/base_types.hpp>

namespace jmmt::impl {

	/// A very simple freelist allocator. Holds bits of memory
	/// and allows objects of a given type to be allocated and retrieved from it.
	/// Reuses memory so heap shouldn't get too fragmented either.
	template <class T, u32 MaxSize>
	class FreeListAllocator {
		// TODO: It might be possible to make this part not template code.
		// I'm not sure of the utility of that, but it might help compile times?
		// I'll look into it.
		struct BucketInfo {
			std::bitset<MaxSize> allocatedSet;
			T* objectPointers[MaxSize];

			// NOTE: You must placement new in the memory here,
			// this function doesn't start lifetime.
			T* mallocObject(u32 index) {
				if(!objectPointers[index]) {
					objectPointers[index] = reinterpret_cast<T*>(malloc(sizeof(T)));
					std::memset(reinterpret_cast<void*>(objectPointers[index]), 0, sizeof(T));
				}
				return objectPointers[index];
			}

			// NOTE: This function does not call destructors, it's expected that
			// the calling code can appropiately clean up the bucket data beforehand.
			void freeObject(u32 index) {
				if(objectPointers[index]) {
					free(objectPointers[index]);
					objectPointers[index] = nullptr;
				}
			}

			~BucketInfo() {
				for(auto i = 0; i < MaxSize; ++i) {
					freeObject(i);
				}
			}
		};

		BucketInfo* pBucketInfo = nullptr;

	   public:
		/// A handle to an object in the freelist allocator.
		using Handle = i32;

		/// An invalid handle.
		constexpr static Handle InvalidHandle = -1;

		FreeListAllocator() = default;

		// Freelist allocators cannot be relocated. However, the only consumer of this code
		// won't ever do that anyways.
		FreeListAllocator(const FreeListAllocator&) = delete;
		FreeListAllocator(FreeListAllocator&&) = delete;

		~FreeListAllocator() {
			clear();
		}

		void clear() {
			Handle handlesToClear[MaxSize];
			u32 nHandles = 0;

			// Find all allocated objects.
			for(usize i = 0; i < pBucketInfo->allocatedSet.size(); ++i) {
				if(pBucketInfo->allocatedSet[i]) {
					handlesToClear[nHandles++] = i;
				}
			}

			// Clear them.
			for(u32 i = 0; i < nHandles; ++i)
				freeObject(handlesToClear[i]);

			delete pBucketInfo;
			pBucketInfo = nullptr;
		}

		template <class... Args>
		Handle allocateObject(Args&&... args) {
			// If bucket information hasn't been allocated beforehand,
			// do so.
			if(!pBucketInfo) {
				pBucketInfo = new BucketInfo();
			}

			// Find any free position.
			for(usize i = 0; i < pBucketInfo->allocatedSet.size(); ++i) {
				// Construct the object in the memory, and then return the handle.
				if(!pBucketInfo->allocatedSet[i]) {
					pBucketInfo->allocatedSet[i] = true;
					new(pBucketInfo->mallocObject(i)) T(static_cast<Args&&>(args)...);
					return i;
				}
			}

			// There are no free slots, so give up and return an invalid handle.
			return InvalidHandle;
		}

		/// Dereference an handle, obtaining a concrete pointer to an object.
		T* dereferenceHandle(Handle handle) {
			if(handle == InvalidHandle)
				return nullptr;

			if(handle > MaxSize)
				return nullptr;

			if(!pBucketInfo)
				return nullptr;

			// If this object is actually allocated, then return a pointer to its memory
			// (i.e: actually deref the handle). Otherwise, return a null pointer, to indicate failure.
			if(pBucketInfo->allocatedSet[handle]) {
				return pBucketInfo->objectPointers[handle];
			}

			return nullptr;
		}

		/// Free a object pointed to by [handle].
		void freeObject(Handle handle) {
			if(handle == InvalidHandle)
				return;

			if(handle > MaxSize)
				return;

			if(!pBucketInfo)
				return;

			// If the handle is actually dereferenceable to an allocated object..
			if(pBucketInfo->allocatedSet[handle]) {
				// Free the object. Another allocateObject() call
				// can provide the same handle.
				pBucketInfo->allocatedSet[handle] = false;
				pBucketInfo->objectPointers[handle]->~T();

				// Zero the memory once the object is freed.
				std::memset(reinterpret_cast<void*>(pBucketInfo->objectPointers[handle]), 0, sizeof(T));
			}
		}
	};

} // namespace jmmt::impl
