#pragma once
#include <mco/base_types.hpp>
#include <jmmt/fs/package_metadata.hpp>
#include <unordered_map>

namespace jmmt::fs {
	class GameFileSystem;

	class PakFileSystem : public std::enable_shared_from_this<PakFileSystem> {
		struct Impl;
		Unique<Impl> impl;
	public:
		using FileHandle = i32;

		/// Public metadata.
		struct Metadata {
			std::string sourceName;
			std::string sourceConvertName;
			std::string sourceCompressName;
			std::string typeName;

			u32 fileSize;
			u32 dateStamp; // It is currently unknown what this date stamp is.
		};

		enum SeekOrigin {
				SeekBegin = 0,
				SeekCurrent,
				SeekEnd
		};

		enum Error {
			// init errors
			Success = 0,
			InitReadChunkFailure = 1,
			InitReadStringTableFailure = 2,
			InitProcessChunksFailure = 3,

			// fileOpen errors
			FileNotExist = -1,
		};

		explicit PakFileSystem(Ref<GameFileSystem> fs, const PackageMetadata& metadata, const std::string& fileName);
		~PakFileSystem();

		Error initialize();

		/// Get file metadata
		const std::unordered_map<std::string, PakFileSystem::Metadata>& getMetadata();

		/// Opens a new pak file.
		FileHandle fileOpen(const std::string_view path);

		/// Reads data from a previously-opened package file.
		i32 fileRead(FileHandle file, void* pBuffer, u32 size);

		/// Sets the seek pointer of a pak file.
		i32 fileSeek(FileHandle file, i32 offset, SeekOrigin origin);

		/// Obtains the seek pointer of a pak file.
		i32 fileTell(FileHandle file);

		/// Obtains the size of a opened pak file.
		u32 fileGetSize(FileHandle file);

		/// Closes a previously-owned pak file.
		void fileClose(FileHandle file);

	};

}
