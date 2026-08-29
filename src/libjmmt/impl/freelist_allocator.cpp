#include <jmmt/impl/freelist_allocator.hpp>

namespace jmmt::impl {

	FreeListBucketPool::FreeListBucketPool(usize maxObjects, usize objectSize) {
		ppObjectPointers = reinterpret_cast<u8**>(malloc(maxObjects * sizeof(void*)));

		// Allocate a pool of memory that the objects will be allocated inside.
		u8* pBase = reinterpret_cast<u8*>(malloc(maxObjects * objectSize));

		// Set up the object pointers so they point in the correct place for the pool.
		for(usize i = 0; i < maxObjects; ++i) {
			ppObjectPointers[i] = reinterpret_cast<u8*>(&pBase[i * objectSize]);
		}
	}

	FreeListBucketPool::~FreeListBucketPool() {
		// Free both the pool and the pointer array.
		free(ppObjectPointers[0]);
		free(ppObjectPointers);
	}

}
