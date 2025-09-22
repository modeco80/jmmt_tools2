#include <base/types.hpp>

namespace jmmt {

	// First, let's declare some bookkeeping data structures.

	/// The chunk map. This data is used to store information
	/// We pre-create this for every file inside of a package file.
	///
	struct FileChunkMap {
		struct Entry {
			u32 chunkIndex;		 // Used to sort
			u32 chunkByteOffset; // The offset where this chunk should be placed in the file data
			u32 chunkDataOffset; // Offset in .pak file where this chunk starts

			// std::optional<structs::lzss::Header> lzssHeader;
			// if provided this is the LZSS header to use when decompressing..
		};

		u32 nHash;
		u32 nHashAlt;
		u32 nHashCompAlt;

		u32 nChunks;
		Entry* pChunkEntries;

		FileChunkMap(u32 nChunks) : nChunks(nChunks) {
			pChunkEntries = new Entry[nChunks];
		}

		// Can't be relocated. Files will only retain const& non-owning references
		// to a particular chunk map instance which matches the file they have open.
		FileChunkMap(const FileChunkMap&) = delete;
		FileChunkMap(FileChunkMap&&) = delete;

		~FileChunkMap() {
			delete[] pChunkEntries;
		}

		Entry& operator[](usize index) {
			return pChunkEntries[index];
		}
		const Entry& operator[](usize index) const {
			return pChunkEntries[index];
		}
	};

	// A file is read chunk-by-chunk (providing some read-ahead).
	// When reading, once we cross a chunk boundary,
	// we decompress the data and continue, until there are no more chunks left.

} // namespace jmmt
