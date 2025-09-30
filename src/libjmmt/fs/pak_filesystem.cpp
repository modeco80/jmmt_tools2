#include <jmmt/fourcc.hpp>
#include <jmmt/fs/game_filesystem.hpp>
#include <jmmt/fs/pak_filesystem.hpp>
#include <jmmt/structs/package/file.hpp>
#include <jmmt/structs/package/group.hpp>
#include <mco/base_types.hpp>
#include <mco/io/file_stream.hpp>
#include <mco/io/memory_stream.hpp>
#include <unordered_map>
#include <jmmt/crc.hpp>

namespace jmmt::fs {

	/// This data is used to store the chunk information.
	/// We pre-create this for every file inside of a package file
	/// when initializing the package filesystem.
	struct FileMetadata {
		struct Entry {
			u32 chunkByteOffset; // The offset where this chunk is placed
			u32 chunkDataOffset; // Offset in .pak file where this chunk starts
			u32 chunkDataSize; // The size of the uncompressed chunk data
			bool compressed; // True if this chunk is compressed
		};

		u32 nChunks;
		Entry* pChunkEntries;

		std::string sourceName;
		std::string sourceConvertName;
		std::string sourceCompressName;
		std::string typeName;

		FileMetadata(u32 nChunks) : nChunks(nChunks) {
			pChunkEntries = new Entry[nChunks];
		}

		// Can't be relocated. Files will only retain const& non-owning references
		// to a particular chunk map instance which matches the file they have open.
		FileMetadata(const FileMetadata&) = delete;
		FileMetadata(FileMetadata&& move) = delete;

		~FileMetadata() {
			delete[] pChunkEntries;
		}

		Entry& operator[](usize index) {
			return pChunkEntries[index];
		}

		const Entry& operator[](usize index) const {
			return pChunkEntries[index];
		}
	};

	/// Read except not really.
	i64 temporaryRead(mco::Stream& stream, void* buffer, i64 len) {
		// TODO: precondition is stream.isRandomAccess() == true
		auto previouslyAt = stream.tell();
		auto ret = stream.read(buffer, len);
		stream.seek(previouslyAt, mco::Stream::Begin);
		return ret;
	}

	std::string findInStringTable(u32 nHash, const std::vector<std::string>& stringTable, const std::vector<u32>& stringTableHashes) {
		for(auto i = 0; i <stringTable.size(); ++i) {
			if(nHash == stringTableHashes[i]) {
				return stringTable[i];
				break;
			}
		}
		return "Not found in string table???";
	}

	// A file is read chunk-by-chunk (providing some read-ahead).
	// When reading, once we cross a chunk boundary,
	// we decompress the data and continue, until there are no more chunks left.

	class PakFile {
		const FileMetadata& chunkList;
		// a 64k buffer which we decompress or copy chunks into
		Unique<u8[]> mChunkDecompressBuffer;
	};

	class PakFileSystem::Impl {
		Ref<GameFileSystem> gameFs;
		PackageMetadata metadata;
		std::string pakFilename;

		std::unordered_map<std::string, Unique<FileMetadata>> fileMetadata;
		std::unordered_map<FileHandle, PakFile> openFiles;
		FileHandle lastHandle { 0 };

	   public:
		Impl(Ref<GameFileSystem> fs, const PackageMetadata& metadata, const std::string& fileName)
			: gameFs(fs), metadata(metadata), pakFilename(fileName) {
		}

		bool processPackageChunks(u8* pChunkData, usize chunkSize, const std::vector<std::string>& stringTable, const std::vector<u32>& stringTableHashes) {
			auto chunkStream = mco::MemoryStream(pChunkData, chunkSize);
			std::string currentFileName;

			while(!chunkStream.hasEnded()) {
				// Read the chunk id and seek back so we can figure out what chunk it is.
				FourCC chunkId {};
				if(auto n = temporaryRead(chunkStream, &chunkId, sizeof(chunkId)); n != sizeof(chunkId))
					return false;

				switch(chunkId) {
					case structs::PackageGroupHeader::MAGIC: {
						// might be nice to provide this?
						structs::PackageGroupHeader pgrp {};
						if(auto n = chunkStream.read(&pgrp, sizeof(pgrp)); n != sizeof(pgrp))
							return false;
					} break;

					case structs::PackageFileHeader::MAGIC: {
						structs::PackageFileHeader pfil {};
						if(auto n = chunkStream.read(&pfil, sizeof(pfil)); n != sizeof(pfil))
							return false;

						if(pfil.chunkNumber == 0) {
							// Find the filename in the string table hashes.
							currentFileName = findInStringTable(pfil.indexName, stringTable, stringTableHashes);


							std::printf("new file name %s\n", currentFileName.c_str());
							fileMetadata.emplace(currentFileName, std::make_unique<FileMetadata>(pfil.chunkCount));

							fileMetadata[currentFileName]->sourceName = findInStringTable(pfil.indexSourceName, stringTable, stringTableHashes);
							fileMetadata[currentFileName]->sourceConvertName = findInStringTable(pfil.indexSourceConvertName, stringTable, stringTableHashes);
							fileMetadata[currentFileName]->sourceCompressName = findInStringTable(pfil.indexSourceCompressName, stringTable, stringTableHashes);
							fileMetadata[currentFileName]->typeName = findInStringTable(pfil.indexType, stringTable, stringTableHashes);

							std::printf("%s\n%s\n%s\n%s\n",
									fileMetadata[currentFileName]->sourceName.c_str(),
									fileMetadata[currentFileName]->sourceConvertName.c_str(),
									fileMetadata[currentFileName]->sourceCompressName.c_str(),
									fileMetadata[currentFileName]->typeName.c_str()
							);

							fileMetadata[currentFileName]->pChunkEntries[pfil.chunkNumber] = {
								.chunkByteOffset = pfil.chunkOffset,
								.chunkDataOffset = pfil.dataOffset,
								.chunkDataSize = pfil.chunkSize,
								.compressed = pfil.chunkSize != pfil.dataSize
							};
						} else {
							fileMetadata[currentFileName]->pChunkEntries[pfil.chunkNumber] = {
								.chunkByteOffset = pfil.chunkOffset,
								.chunkDataOffset = pfil.dataOffset,
								.chunkDataSize = pfil.chunkSize,
								.compressed = pfil.chunkSize != pfil.dataSize
							};
						}
					} break;

					default:
						// invalid type.
						std::printf("invalid chunk type %08x\n", chunkId);
						return false;
				}
			}

			return true;
		}

		bool initializeImpl() {
			auto file = gameFs->openFile(pakFilename, GameFileSystem::FileData);
			Unique<u8[]> mHeaderBuffer = std::make_unique<u8[]>(metadata.chunkDataSize);

			// Read the package chunk data into a data buffer. We'll process these in a second.
			file.seek(metadata.chunkStartOffset, mco::Stream::Begin);
			if(auto n = file.read(&mHeaderBuffer[0], metadata.chunkDataSize); n != metadata.chunkDataSize) {
				// This should never short read. If it does this indicates a problem with the file data
				// which would imply the game could not use this file either.
				return false;
			}

			// Next, read the string table. This will be used for filename resolution.
			std::vector<std::string> stringTable;
			std::vector<u32> stringTableHashes;

			u32 nStringTableEntries {};
			if(auto n = file.read(&nStringTableEntries, sizeof(u32)); n != sizeof(u32))
				return false;

			stringTable.reserve(nStringTableEntries);
			stringTableHashes.resize(nStringTableEntries);
			for(auto i = 0; i < nStringTableEntries; ++i) {
				stringTable.push_back(file.readString());
				stringTableHashes[i] = jmmt::hashString(stringTable[i]);
			}

#if 0
			for(auto i = 0; i < nStringTableEntries; ++i) {
				std::printf("String Table Entry %u: %s Hash(%08x)\n", i, stringTable[i].c_str(), stringTableHashes[i]);
			}
#endif

			if(!processPackageChunks(mHeaderBuffer.get(), metadata.chunkDataSize, stringTable, stringTableHashes))
				return false;

			return true;
		}
	};

	PakFileSystem::PakFileSystem(Ref<GameFileSystem> fs, const PackageMetadata& metadata, const std::string& fileName)
		: impl(std::make_unique<Impl>(fs, metadata, fileName)) {
	}

	PakFileSystem::~PakFileSystem() = default;

	bool PakFileSystem::initialize() {
		return impl->initializeImpl();
	}

	PakFileSystem::FileHandle PakFileSystem::openFile(const std::string_view path) {
		return -1;
	}

	i64 PakFileSystem::readSome(FileHandle file, void* pBuffer, u64 size) {
		return -1;
	}

	i64 PakFileSystem::seekFile(FileHandle file, i64 offset, SeekOrigin origin) {
		return -1;
	}

	void PakFileSystem::closeFile(FileHandle file) {
		return;
	}

} // namespace jmmt::fs
