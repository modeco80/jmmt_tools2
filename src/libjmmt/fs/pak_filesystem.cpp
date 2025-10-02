#include <jmmt/crc.hpp>
#include <jmmt/fourcc.hpp>
#include <jmmt/fs/game_filesystem.hpp>
#include <jmmt/fs/pak_filesystem.hpp>
#include <jmmt/impl/freelist_allocator.hpp>
#include <jmmt/lzss/decompress.hpp>
#include <jmmt/structs/package/file.hpp>
#include <jmmt/structs/package/group.hpp>
#include <mco/base_types.hpp>
#include <mco/io/file_stream.hpp>
#include <mco/io/memory_stream.hpp>
#include <unordered_map>

namespace jmmt::fs {

	/// This data is used to store the chunk information.
	/// We pre-create this for every file inside of a package file
	/// when initializing the package filesystem.
	struct FileMetadata {
		struct ChunkMetadata {
			u32 chunkByteOffset;	   // The offset where this chunk is placed
			u32 chunkDataOffset;	   // Offset in .pak file where this chunk starts
			u32 chunkDataSize;		   // The size of the chunk data inside of the pak
			u32 chunkUncompressedSize; // The uncompressed size of the chunk.
			bool compressed;		   // True if this chunk is compressed
		};

		u32 nChunks;
		ChunkMetadata* pChunkMetaEntries;

		std::string sourceName;
		std::string sourceConvertName;
		std::string sourceCompressName;
		std::string typeName;

		u32 fileSize;

		FileMetadata(u32 nChunks) : nChunks(nChunks) {
			pChunkMetaEntries = new ChunkMetadata[nChunks];
		}

		// Can't be relocated. Files will only retain const& non-owning references
		// to a particular chunk map instance which matches the file they have open.
		FileMetadata(const FileMetadata&) = delete;
		FileMetadata(FileMetadata&& move) = delete;

		~FileMetadata() {
			delete[] pChunkMetaEntries;
		}

		ChunkMetadata& operator[](usize index) {
			return pChunkMetaEntries[index];
		}

		const ChunkMetadata& operator[](usize index) const {
			return pChunkMetaEntries[index];
		}
	};

	/// A package file.
	class PakFile {
		/// The metadata for this file.
		const FileMetadata& metadata;

		/// A 64k buffer which we decompress or copy chunk data into
		Unique<u8[]> chunkBuffer;

		/// A 64k buffer which we read the raw chunk data buffer from the file into.
		Unique<u8[]> chunkReadBuffer;

		/// A file stream with the .pak file.
		mco::FileStream packageFileStream;

		// The current active chunk.
		u16 currentChunk;

		// byte offset in the chunk
		u32 currentChunkByteOffset;
		u32 currentChunkByteSize;

		// current byte offset in the file.
		u32 currentByteOffset;

		u32 findChunkIndex(u32 offset, u32 numChunks) {
			u32 cumulativeOffset = 0;

			for(u32 i = 0; i < metadata.nChunks; ++i) {
				cumulativeOffset += metadata[i].chunkUncompressedSize;
				if(offset < cumulativeOffset) {
					return i;
				}
			}

			return -1;
		}

		u32 getChunkTotalSize(u32 chunkIndex) {
			u32 cumulativeOffset = 0;
			for(u32 i = 0; i < chunkIndex; ++i) {
				cumulativeOffset += metadata[i].chunkUncompressedSize;
			}
			return cumulativeOffset;
		}

		void newChunk(u32 chunkIndex) {
			// dont switch chunks
			if(chunkIndex > metadata.nChunks)
				return;
			currentChunk = chunkIndex;
			currentChunkByteOffset = 0;
			updateChunk();
		}

		void updateChunk() {
			// Cache the uncompressed size of the chunk.
			currentChunkByteSize = metadata[currentChunk].chunkUncompressedSize;

			// Read the chunk data from the package file into memory.
			packageFileStream.seek(metadata[currentChunk].chunkDataOffset, mco::Stream::Begin);
			packageFileStream.read(&chunkReadBuffer[0], metadata[currentChunk].chunkDataSize);

			// Depending on if the chunk is compressed or not, copy the data into the chunk buffer.
			if(metadata[currentChunk].compressed) {
				lzss::decompress(nullptr, &chunkReadBuffer[0], metadata[currentChunk].chunkDataSize, &chunkBuffer[0]);
			} else {
				memcpy(&chunkBuffer[0], &chunkReadBuffer[0], metadata[currentChunk].chunkUncompressedSize);
			}
		}

		/// Helper to seek to a byte offset. seek() builds upon this
		/// to implement the fully-featured seek function.
		void seekOffset(u32 offset) {
			if(offset >= metadata.fileSize)
				return;

			// Don't switch the current chunk unless we have to.
			if(auto chunkIndex = findChunkIndex(offset, metadata.nChunks); chunkIndex != currentChunk)
				newChunk(chunkIndex);

			// Set state once we've done that
			currentByteOffset = offset;
			currentChunkByteOffset = offset - getChunkTotalSize(currentChunk);
		}

	   public:
		explicit PakFile(const FileMetadata& metadata, mco::FileStream&& fileStream)
			: metadata(metadata), packageFileStream(std::move(fileStream)) {
			// Allocate temporary buffers.
			chunkBuffer = std::make_unique<u8[]>(65536);
			chunkReadBuffer = std::make_unique<u8[]>(65536);

			// Reset state and initalize the first chunk.
			currentByteOffset = 0;
			newChunk(0);
		}

		i32 read(void* buffer, u32 count) {
			if(currentByteOffset > metadata.fileSize)
				return 0;
			u32 bytesRemaining = count;
			auto outputBuffer = reinterpret_cast<u8*>(buffer);

			while(bytesRemaining > 0) {
				u32 currentChunkSize = metadata[currentChunk].chunkUncompressedSize;
				u32 bytesToRead = std::min(bytesRemaining, currentChunkSize - currentChunkByteOffset);
				std::memcpy(outputBuffer + (count - bytesRemaining), chunkBuffer.get() + currentChunkByteOffset, bytesToRead);
				bytesRemaining -= bytesToRead;

				// I don't like this logic, but it works, so /shrug
				// it should be possible to use seek() but that doesn't work?
				currentChunkByteOffset += bytesToRead;
				currentByteOffset += bytesToRead;
				if(currentChunkByteOffset >= currentChunkSize) {
					if(currentChunk + 1 >= metadata.nChunks)
						break;
					newChunk(currentChunk + 1);
				}
			}

			return count - bytesRemaining;
		}

		i32 seek(i32 offset, PakFileSystem::SeekOrigin whence) {
			u32 computedOffset;
			switch(whence) {
				case PakFileSystem::SeekBegin:
					computedOffset = offset;
					break;
				case PakFileSystem::SeekCurrent:
					computedOffset = currentByteOffset + offset;
					break;
				case PakFileSystem::SeekEnd:
					computedOffset = metadata.fileSize + offset;
					break;
			}

			if(computedOffset < 0 || computedOffset > metadata.fileSize)
				return -1;

			seekOffset(computedOffset);
			return computedOffset;
		}

		u32 tell() const {
			return currentByteOffset;
		}
	};

	/// The max amount of files that can be open in the package filesystem. This is an implementation detail,
	/// and thus is not exposed externally.
	constexpr static usize MaxFileCount = 32;

	/// freelist allocator type for files.
	using FileFreeList = impl::FreeListAllocator<PakFile, MaxFileCount>;

	/// Read except not really.
	i64 temporaryRead(mco::Stream& stream, void* buffer, i64 len) {
		// TODO: precondition is stream.isRandomAccess() == true
		auto previouslyAt = stream.tell();
		auto ret = stream.read(buffer, len);
		stream.seek(previouslyAt, mco::Stream::Begin);
		return ret;
	}

	std::string findInStringTable(u32 nHash, const std::vector<std::string>& stringTable, const std::vector<u32>& stringTableHashes) {
		for(auto i = 0; i < stringTable.size(); ++i) {
			if(nHash == stringTableHashes[i]) {
				return stringTable[i];
				break;
			}
		}
		return "Not found in string table???";
	}

	class PakFileSystem::Impl {
		Ref<GameFileSystem> gameFs;
		PackageMetadata metadata;
		std::string pakFilename;

		structs::PackageGroupHeader packageGroup;

		std::unordered_map<std::string, Unique<FileMetadata>> fileMetadata;

		/// Open files.
		FileFreeList openFiles;

		// string table
		std::vector<std::string> stringTable;
		std::vector<u32> stringTableHashes;

	   public:
		Impl(Ref<GameFileSystem> fs, const PackageMetadata& metadata, const std::string& fileName)
			: gameFs(fs), metadata(metadata), pakFilename(fileName) {
		}

		Error processPackageChunks(u8* pChunkData, usize chunkSize, const std::vector<std::string>& stringTable, const std::vector<u32>& stringTableHashes) {
			auto chunkStream = mco::MemoryStream(pChunkData, chunkSize);
			std::string currentFileName;

			while(!chunkStream.hasEnded()) {
				// Read the chunk id and seek back so we can figure out what chunk it is.
				FourCC chunkId {};
				if(auto n = temporaryRead(chunkStream, &chunkId, sizeof(chunkId)); n != sizeof(chunkId))
					return PakFileSystem::InitProcessChunksFailure;

				switch(chunkId) {
					case structs::PackageGroupHeader::MAGIC: {
						// might be nice to provide this?
						structs::PackageGroupHeader groupHeader {};
						if(auto n = chunkStream.read(&groupHeader, sizeof(groupHeader)); n != sizeof(groupHeader))
							return PakFileSystem::InitProcessChunksFailure;
						std::memcpy(&this->packageGroup, &groupHeader, sizeof(groupHeader));
					} break;

					case structs::PackageFileHeader::MAGIC: {
						structs::PackageFileHeader pfil {};
						if(auto n = chunkStream.read(&pfil, sizeof(pfil)); n != sizeof(pfil))
							return PakFileSystem::InitProcessChunksFailure;

						if(pfil.chunkNumber == 0) {
							// Find the filename in the string table hashes.
							currentFileName = findInStringTable(pfil.indexName, stringTable, stringTableHashes);

							// Create metadata for this file.
							fileMetadata.emplace(currentFileName, std::make_unique<FileMetadata>(pfil.chunkCount));
							fileMetadata[currentFileName]->sourceName = findInStringTable(pfil.indexSourceName, stringTable, stringTableHashes);
							fileMetadata[currentFileName]->sourceConvertName = findInStringTable(pfil.indexSourceConvertName, stringTable, stringTableHashes);
							fileMetadata[currentFileName]->sourceCompressName = findInStringTable(pfil.indexSourceCompressName, stringTable, stringTableHashes);
							fileMetadata[currentFileName]->typeName = findInStringTable(pfil.indexType, stringTable, stringTableHashes);
							fileMetadata[currentFileName]->fileSize = pfil.totalFileSize;

							(*fileMetadata[currentFileName])[pfil.chunkNumber] = {
								.chunkByteOffset = pfil.chunkOffset,
								.chunkDataOffset = pfil.dataOffset,
								.chunkDataSize = pfil.dataSize,
								.chunkUncompressedSize = pfil.chunkSize,
								.compressed = pfil.chunkSize != pfil.dataSize
							};
						} else {
							(*fileMetadata[currentFileName])[pfil.chunkNumber] = {
								.chunkByteOffset = pfil.chunkOffset,
								.chunkDataOffset = pfil.dataOffset,
								.chunkDataSize = pfil.dataSize,
								.chunkUncompressedSize = pfil.chunkSize,
								.compressed = pfil.chunkSize != pfil.dataSize
							};
						}
					} break;

					default:
						// invalid type.
						return PakFileSystem::InitProcessChunksFailure;
				}
			}

			return PakFileSystem::Success;
		}

		Error initializeImpl() {
			auto file = gameFs->openFile(pakFilename, GameFileSystem::FileData);
			Unique<u8[]> mHeaderBuffer = std::make_unique<u8[]>(metadata.chunkDataSize);

			// Read the package chunk data into a data buffer. We'll process these in a second.
			file.seek(metadata.chunkStartOffset, mco::Stream::Begin);
			if(auto n = file.read(&mHeaderBuffer[0], metadata.chunkDataSize); n != metadata.chunkDataSize) {
				// This should never short read. If it does this indicates a problem with the file data
				// which would imply the game could not use this file either.
				return PakFileSystem::InitReadChunkFailure;
			}

			u32 nStringTableEntries {};

			// Next, read the string table. This will be used for filename resolution.
			if(auto n = file.read(&nStringTableEntries, sizeof(u32)); n != sizeof(u32))
				return PakFileSystem::InitReadStringTableFailure;

			stringTable.reserve(nStringTableEntries);
			stringTableHashes.resize(nStringTableEntries);
			for(auto i = 0; i < nStringTableEntries; ++i) {
				stringTable.push_back(file.readString());
				stringTableHashes[i] = jmmt::hashString(stringTable[i]);
			}

			// propagate the error
			if(auto procesRet = processPackageChunks(mHeaderBuffer.get(), metadata.chunkDataSize, stringTable, stringTableHashes); procesRet != PakFileSystem::Success)
				return procesRet;

			return PakFileSystem::Success;
		}

		FileHandle openFileImpl(std::string_view path) {
			if(auto it = fileMetadata.find(std::string(path)); it != fileMetadata.end()) {
				auto file = gameFs->openFile(pakFilename, GameFileSystem::FileData);
				return openFiles.allocateObject(*it->second, std::move(file));
			}
			return -1;
		}

		i32 readSomeImpl(FileHandle file, void* pBuffer, u32 size) {
			if(auto filePtr = openFiles.dereferenceHandle(file); filePtr) {
				return filePtr->read(pBuffer, size);
			}
			return -1;
		}

		i32 seekFileImpl(FileHandle file, i32 offset, SeekOrigin origin) {
			if(auto filePtr = openFiles.dereferenceHandle(file); filePtr) {
				return filePtr->seek(offset, origin);
			}
			return -1;
		}

		i32 tellFileImpl(FileHandle file) {
			if(auto filePtr = openFiles.dereferenceHandle(file); filePtr) {
				return filePtr->tell();
			}
			return -1;
		}

		void closeFileImpl(FileHandle file) {
			openFiles.freeObject(file);
		}
	};

	PakFileSystem::PakFileSystem(Ref<GameFileSystem> fs, const PackageMetadata& metadata, const std::string& fileName)
		: impl(std::make_unique<Impl>(fs, metadata, fileName)) {
	}

	PakFileSystem::~PakFileSystem() = default;

	PakFileSystem::Error PakFileSystem::initialize() {
		return impl->initializeImpl();
	}

	PakFileSystem::FileHandle PakFileSystem::openFile(const std::string_view path) {
		return impl->openFileImpl(path);
	}

	i32 PakFileSystem::readSome(FileHandle file, void* pBuffer, u32 size) {
		return impl->readSomeImpl(file, pBuffer, size);
	}

	i32 PakFileSystem::seekFile(FileHandle file, i32 offset, SeekOrigin origin) {
		return impl->seekFileImpl(file, offset, origin);
	}

	i32 PakFileSystem::tellFile(FileHandle file) {
		return impl->tellFileImpl(file);
	}

	void PakFileSystem::closeFile(FileHandle file) {
		return impl->closeFileImpl(file);
	}

} // namespace jmmt::fs
