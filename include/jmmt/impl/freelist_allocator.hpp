#pragma once
#include <bitset>
#include <cstring>
#include <mco/base_types.hpp>

namespace jmmt::impl {

	/// Base bucket pool class.
	class FreeListBucketPool {
	   protected:
		u8** ppObjectPointers;

	   public:
		/// Constructor. Allocates a [maxObjects] * [objectSize] pool of memory
		/// which can be used for the freelist. Inheritors have access to the [ppObjectPointers]
		/// pointer array, which is initalized by this constructor to point to each object slot.
		FreeListBucketPool(usize maxObjects, usize objectSize);

		FreeListBucketPool(const FreeListBucketPool&) = delete;
		FreeListBucketPool(FreeListBucketPool&&) = delete; // No relocation (?)

		~FreeListBucketPool();
	};

	/// Shim over [FreeListBucketPool] to make it typed.
	template <class T>
	class FreeListTypedBucketPool : FreeListBucketPool {
	   public:
		explicit FreeListTypedBucketPool(usize maxObjects)
			: FreeListBucketPool(maxObjects, sizeof(T)) {
		}

	   protected:
		inline T* getPointer(usize index) const {
			return reinterpret_cast<T*>(ppObjectPointers[index]);
		}

		template <class... Args>
		inline void constructObject(usize index, Args&&... args) {
			auto* pObject = getPointer(index);
			std::memset(reinterpret_cast<void*>(pObject), 0, sizeof(T));
			new(pObject) T(static_cast<Args&&>(args)...);
		}

		inline void destructObject(usize index) {
			auto* pObject = getPointer(index);
			pObject->~T();
			std::memset(reinterpret_cast<void*>(pObject), 0, sizeof(T));
		}
	};

	/// A handle to an object in the freelist allocator.
	using FreeListObjectHandle = i32;

	/// An invalid handle.
	constexpr static FreeListObjectHandle InvalidHandle = -1;

	/// The actual freelist bucket.
	template <class T, usize MaxSize>
	class FreeListBucket : public FreeListTypedBucketPool<T> {
		std::bitset<MaxSize> allocatedSet;

	   public:
		T* getObjectPointer(u32 index) {
			if(!allocatedSet[index])
				return nullptr;

			return FreeListTypedBucketPool<T>::getPointer(index);
		}

		template <class... Args>
		i32 allocateObject(Args&&... args) {
			// Find any free position.
			for(usize i = 0; i < MaxSize; ++i) {
				// Construct the object in the memory, and then return the handle.
				if(!allocatedSet[i]) {
					FreeListTypedBucketPool<T>::constructObject(i, static_cast<Args&&>(args)...);
					allocatedSet[i] = true;
					return i;
				}
			}

			return InvalidHandle;
		}

		void freeObject(FreeListObjectHandle handle) {
			// If the handle is actually dereferenceable to an allocated object..
			if(allocatedSet[handle]) {
				// Free the object. Another allocateObject() call
				// can provide the same handle.
				FreeListTypedBucketPool<T>::destructObject(handle);
				allocatedSet[handle] = false;
			}
		}

		void clear() {
			FreeListObjectHandle handlesToClear[MaxSize];
			u32 nHandles = 0;

			// Find all allocated objects.
			for(usize i = 0; i < MaxSize; ++i) {
				if(allocatedSet[i]) {
					handlesToClear[nHandles++] = i;
				}
			}

			// Clear them.
			for(u32 i = 0; i < nHandles; ++i)
				freeObject(handlesToClear[i]);
		}

		FreeListBucket()
			: FreeListTypedBucketPool<T>(MaxSize) {
		}

		~FreeListBucket() {
			clear();
		}
	};

	/// A very simple freelist allocator. Holds bits of memory
	/// and allows objects of a given type to be allocated and retrieved from it.
	template <class T, u32 MaxSize>
	class FreeListAllocator {
		FreeListBucket<T, MaxSize>* pBucket = nullptr;

	   public:
		FreeListAllocator() = default;

		// Freelist allocators cannot be relocated. However, the only consumer of this code
		// won't ever do that anyways.
		FreeListAllocator(const FreeListAllocator&) = delete;
		FreeListAllocator(FreeListAllocator&&) = delete;

		~FreeListAllocator() {
			clear();
		}

		void clear() {
			// Nothing to clear.
			if(!pBucket)
				return;

			delete pBucket;
			pBucket = nullptr;
		}

		template <class... Args>
		FreeListObjectHandle allocateObject(Args&&... args) {
			// Allocate the bucket if it hasn't been allocated before.
			if(!pBucket) {
				pBucket = new FreeListBucket<T, MaxSize>();
			}

			return pBucket->allocateObject(static_cast<Args&&>(args)...);
		}

		/// Dereference an handle, obtaining a concrete pointer to an object.
		T* dereferenceHandle(FreeListObjectHandle handle) {
			if(handle == InvalidHandle)
				return nullptr;

			if(handle > MaxSize)
				return nullptr;

			if(!pBucket)
				return nullptr;
			return pBucket->getObjectPointer(handle);
		}

		/// Free a object pointed to by [handle].
		void freeObject(FreeListObjectHandle handle) {
			if(handle == InvalidHandle)
				return;
			if(handle > MaxSize)
				return;
			if(!pBucket)
				return;
			pBucket->freeObject(handle);
		}
	};

} // namespace jmmt::impl
