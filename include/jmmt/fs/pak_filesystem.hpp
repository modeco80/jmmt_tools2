#pragma once
#include <mco/base_types.hpp>
#include <jmmt/fs/package_metadata.hpp>

namespace jmmt::fs {
	class GameFileSystem;

	class PakFileSystem {
		struct Impl;
		Unique<Impl> impl;
	public:
		using FileHandle = i32;

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

			FileNotExist = -1,
		};

		explicit PakFileSystem(Ref<GameFileSystem> fs, const PackageMetadata& metadata, const std::string& fileName);
		~PakFileSystem();

		Error initialize();

		/// Opens a new pak file.
		FileHandle openFile(const std::string_view path);
		i32 readSome(FileHandle file, void* pBuffer, u32 size);
		i32 seekFile(FileHandle file, i32 offset, SeekOrigin origin);
		i32 tellFile(FileHandle file);
		void closeFile(FileHandle file);
	};

}
