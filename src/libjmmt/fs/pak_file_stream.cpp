#include <jmmt/fs/pak_file_stream.hpp>

namespace jmmt::fs {

	PakFileStream PakFileStream::open(Ref<PakFileSystem> fs, const std::string_view fileName) {
		if(auto fd = fs->fileOpen(fileName); fd != PakFileSystem::FileNotExist) {
			return PakFileStream(fs, fd);
		}
		throw std::runtime_error("Attempt to open package file which doesn't exist");
	}

	PakFileStream::PakFileStream(Ref<PakFileSystem> fs, PakFileSystem::FileHandle pakFd)
		: pakFs(fs), pakFd(pakFd) {
		size = pakFs->fileGetSize(pakFd);
	}

	PakFileStream::PakFileStream(PakFileStream&& mv) {
		pakFs = mv.pakFs;
		pakFd = mv.pakFd;
		mv.pakFs.reset();
		mv.pakFd = -1;
	}

	PakFileStream::~PakFileStream() {
		pakFs->fileClose(pakFd);
	}

	u64 PakFileStream::read(void* buffer, u64 length) {
		if(!buffer || length == 0)
			return 0;

		return pakFs->fileRead(pakFd, buffer, length);
	}

	bool PakFileStream::isRandomAccess() const noexcept {
			return true;
	}

	i64 PakFileStream::tell() {
		return pakFs->fileTell(pakFd);
	}

	i64 PakFileStream::seek(i64 offset, Whence whence) {
		PakFileSystem::SeekOrigin pakWhence;
		switch(whence) {
			case Stream::Begin: pakWhence = PakFileSystem::SeekBegin; break;
			case Stream::Current: pakWhence = PakFileSystem::SeekCurrent; break;
			case Stream::End: pakWhence = PakFileSystem::SeekEnd; break;
		}
		return pakFs->fileSeek(pakFd, offset, pakWhence);
	}

	u64 PakFileStream::getSize() {
		return size;
	}

	bool PakFileStream::hasEnded()  {
		return tell() == size;
	}
}
