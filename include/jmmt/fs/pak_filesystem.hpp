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

		explicit PakFileSystem(Ref<GameFileSystem> fs, const PackageMetadata& metadata, const std::string& fileName);
		~PakFileSystem();

		bool initialize();

		/// Opens a new pak file.
		FileHandle openFile(const std::string_view path);
		i64 readSome(FileHandle file, void* pBuffer, u64 size);
		i64 seekFile(FileHandle file, i64 offset, SeekOrigin origin);
		void closeFile(FileHandle file);
	};

}
