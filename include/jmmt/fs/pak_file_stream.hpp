#pragma once

#include <jmmt/fs/pak_filesystem.hpp>
#include <mco/base_types.hpp>
#include <mco/io/stream.hpp>

namespace jmmt::fs {

	/// An adapter stream analagous to [mco::FileStream], but for package files.
	/// Allows mcolib stream algorithms to be used on them rather well.
	class PakFileStream : public mco::Stream {
		Ref<PakFileSystem> pakFs;
		PakFileSystem::FileHandle pakFd;
		u32 size;

		PakFileStream(Ref<PakFileSystem> fs, PakFileSystem::FileHandle pakFd);

	   public:
		/// Opens a file and creates a stream on it.
		static PakFileStream open(Ref<PakFileSystem> fs, const std::string_view fileName);

		/// Creates a [PakFileStream] from a pak file system and file handle.
		static PakFileStream fromPakHandle(Ref<PakFileSystem> fs, PakFileSystem::FileHandle handle);

		PakFileStream(const PakFileStream&) = delete;
		PakFileStream(PakFileStream&&);
		~PakFileStream();

		/// Read some data from the stream
		/// Returns the amount of bytes taken from the stream.
		u64 read(void* buffer, u64 length) override;

		/// Returns true if this stream is random access
		/// (i.e: it can be rewound repeatedly).
		/// If false, seek/tell() will always return -1.
		bool isRandomAccess() const noexcept override;

		/// Obtain the internal seek pointer.
		/// If [isRandomAccess()] returns false, this function will always return -1.
		i64 tell() override;

		/// Advance the stream in a random-access fashion.
		/// If [isRandomAccess()] returns false, this function will always return -1.
		i64 seek(i64 offset, Whence whence) override;

		/// If this stream is random-access, obtains the size of it.
		u64 getSize() override;

		/// If this stream is random-access, returns if the stream has ended.
		bool hasEnded() override;
	};

} // namespace jmmt::fs
